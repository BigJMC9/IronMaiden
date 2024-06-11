#include "H_ScriptEngine.hpp"
#include <algorithm>
#include <windows.h>

namespace Madam
{
	namespace Scripting {

		ScriptEngine::ScriptEngine()
		{

		}

		ScriptEngine::~ScriptEngine()
		{
			if (isImported) {
				ReleaseLibrary();
			}
		}
		
		bool ScriptEngine::Runtime(Entity entity)
		{
			if (isImported) {
				ReleaseLibrary();
			}
			return Import(entity);
		}

		void ScriptEngine::Update() {
			RescanScripts();
			UpdateBehaviour();
		}

		bool ScriptEngine::Import(Entity entity)
		{
			std::string dllDir = Project::Get().getProjectDirectory().string() + "Library\\grt.dll";
			MADAM_INFO("Importing DLL: {0}", dllDir);
			hGetProcIDDLL = LoadLibrary(stringToWSTR(dllDir).c_str());
			if (!hGetProcIDDLL) {
				MADAM_ERROR("Unable to load DLL {0}", GetWindowsError().ToString());
				FreeLibrary(hGetProcIDDLL);
				return false;
			}

			SetAppInstance setAppInstance = (SetAppInstance)GetProcAddress(hGetProcIDDLL, "SetAppInstance");
			if (!setAppInstance) {
				MADAM_ERROR("Unable to Load Function SetAppInstance. Error code: {0}", GetWindowsError().ToString());
				FreeLibrary(hGetProcIDDLL);
				return false;
			}
			setAppInstance(Application::GetPtr());

			BindType getScript = (BindType)GetProcAddress(hGetProcIDDLL, "BindType");
			if (!getScript) {
				MADAM_ERROR("Unable to Load Function BindType. Error code: {0}", GetWindowsError().ToString());
				FreeLibrary(hGetProcIDDLL);
				return false;
			}

			entity.AddComponent<NativeScriptComponent>();
			entity.GetComponent<NativeScriptComponent>() = getScript("gamemanager");

			isImported = true;
			return true;
		}

		void ScriptEngine::ReleaseLibrary()
		{
			FreeLibrary(hGetProcIDDLL);
			isImported = false;
		};

		//Creates a new script
		void ScriptEngine::OnCreateScript(std::string filePath)
		{
			std::ifstream samplefile(Project::Get().getResourcesDirectory().string() + "sample.ims", std::ios::binary);
			if (std::ifstream(Project::Get().getProjectDirectory().string() + "\\Assets\\" + filePath)) {
				MADAM_ERROR("File already Exists");
				return;
			}
			std::ofstream newfile(Project::Get().getProjectDirectory().string() + "\\Assets\\" + filePath, std::ios::binary);

			if (!samplefile) {
				MADAM_ERROR("Cannot Open Sample File for Native Scripts");
				return;
			}
			if (!newfile) {
				MADAM_ERROR("Cannot Create new File");
				return;
			}

			std::istringstream filepathStream(filePath);
			std::vector<std::string> fileWords;

			std::string file;
			while (std::getline(filepathStream, file, '/')) {
				fileWords.push_back(file);
			}
			std::string fileName = file.substr(0, file.length() - 4);
			std::vector<std::string> lines;
			std::string line;
			while (std::getline(samplefile, line)) {
				lines.push_back(line);
			}
			MADAM_INFO("Sample File:");
			for (std::string debugLine : lines)
			{
				MADAM_INFO(debugLine);
			}
			samplefile.close();
			
			MADAM_INFO("Replacing line: {0}", lines[0]);
			MADAM_INFO("with {0}", fileName);
			MADAM_INFO("as //Class: {0}", fileName);
			std::string replace = "//Class: " + fileName;
			lines[0] = replace;
			std::string search = "SampleClass";
			size_t pos = lines[4].find("SampleClass");
			if (pos != std::string::npos) {
				lines[4].replace(pos, search.length(), fileName);
			}
			MADAM_INFO("New File:");
			for (const auto& newline : lines) {
				MADAM_INFO(newline);
				newfile << newline << "\n";
			}
			
			newfile.close();

			scripts[fileName] = filePath;
			char cwd[MAX_PATH];
			GetCurrentDirectoryA(MAX_PATH, cwd);
			std::string batFile = "\"" + std::string(cwd) + "CompileProject.bat\"";
			MADAM_CORE_INFO("Bat File: {0}", batFile);
			std::replace(batFile.begin(), batFile.end(), '/', '\\');
			CreateCMDProcess(batFile);
			RescanScripts();
			UpdateBehaviour();
		}

		void ScriptEngine::UpdateBehaviour()
		{
			if (scripts.size() == 0) {
				MADAM_ERROR("No scripts found");
				return;
			}

			std::string path = Project::Get().getResourcesDirectory().string() + "NativeBehaviour.cpp";
			std::ifstream behaviour(path);
			std::vector<std::string> lines;
			if (behaviour) {
				std::string line;
				while (std::getline(behaviour, line)) {
					lines.push_back(line);
				}
				behaviour.close();
			}
			else {
				MADAM_ERROR("Unable to open file: {0}", path);
				return;
			}

			std::vector<std::string> includes;
			for (auto script : scripts) {
				includes.push_back("#include \"../" + script.second + "\"");
			}

			MADAM_INFO("Includes:");
			for (auto include : includes) {
				MADAM_INFO(include);
			}

			std::vector<std::string> BindLines;
			BindLines = formatIF(scripts.begin()->first);
			for (std::map<std::string, std::string>::iterator i = std::next(scripts.begin()); i != scripts.end(); i++)
			{
				std::vector Temp = formatELIF(i->first);
				BindLines.insert(BindLines.end(), Temp.begin(), Temp.end());
			}
			BindLines.insert(BindLines.end(), formatELSE.begin(), formatELSE.end());

			size_t iterator = 0;

			for (iterator; iterator < lines.size(); iterator++) {
				size_t pos = lines[iterator].find("/*[Include]*/");
				if (pos != std::string::npos) {
					lines.insert(lines.begin() + iterator, includes.begin(), includes.end());
					break;
				}
			}

			for (iterator; iterator < lines.size(); iterator++) {
				size_t pos = lines[iterator].find("/*[Insert]*/");
				if (pos != std::string::npos) {
					lines.insert(lines.begin() + iterator, BindLines.begin(), BindLines.end());
					break;
				}
			}

			std::ofstream newBehaviour(Project::Get().getProjectDirectory().string() + "Library\\NativeBehaviour.cpp");
			if (newBehaviour.is_open()) {
				for (const auto& newline : lines) {
					newBehaviour << newline << "\n";
				}
			}
			else {
				std::string libLoc = Project::Get().getProjectDirectory().string() + "Library/NativeBehaviour.cpp";
				MADAM_ERROR("Unable to open file: {0}", libLoc);
			}
			newBehaviour.close();

			std::string srcpath = Project::Get().getResourcesDirectory().string() + "H_NativeBehaviour.hpp";
			std::string dstpath = Project::Get().getProjectDirectory().string() + "Library/H_NativeBehaviour.hpp";
			if (!CopyScript(srcpath, dstpath)) {
				return;
			}
		}

		bool ScriptEngine::CopyScript(std::string src, std::string dst)
		{
			std::ifstream srcFile(src, std::ios::binary);
			std::ofstream newFile(dst, std::ios::binary);
			if (srcFile && newFile) {
				newFile << srcFile.rdbuf();
				srcFile.close();
				newFile.close();
				return true;
			}
			else if (!srcFile) {
				MADAM_ERROR("Unable to open file: {0}", src);
			}
			else if (!newFile) {
				MADAM_ERROR("Unable to open file: {0}", dst);
			}
			else {
				MADAM_ERROR("Unable to copy file: {0} to {1}", src, dst);
			}
			return false;
		}

		void ScriptEngine::OnDeleteScript()
		{

			//Delete a script
		}

		void ScriptEngine::CreateCMDProcess(std::string path)
		{
			STARTUPINFO si;
			PROCESS_INFORMATION pi;

			std::wstring wstr(path.begin(), path.end());
			const wchar_t* wCharArray = wstr.c_str();

			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));

			if (!CreateProcess(NULL,   // No module name (use command line)
				(LPWSTR)wCharArray,  // Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				FALSE,          // Set handle inheritance to FALSE
				0,              // No creation flags
				NULL,           // Use parent's environment block
				NULL,           // Use parent's starting directory 
				&si,            // Pointer to STARTUPINFO structure
				&pi)           // Pointer to PROCESS_INFORMATION structure
				)
			{
				DWORD errorCode = GetLastError();
				LPVOID lpMsgBuf;
				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					errorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&lpMsgBuf,
					0,
					NULL
				);
				std::wstring wideStr((wchar_t*)lpMsgBuf);
				std::string narrowStr(wideStr.begin(), wideStr.end());
				MADAM_ERROR("CreateProcess failed {0} : {1}", errorCode, narrowStr);
				LocalFree(lpMsgBuf);
				return;
			}

			// Close process and thread handles. 
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}

		void ScriptEngine::RescanScripts() 
		{
			scripts.clear();
			std::string projectFolder = Project::Get().getProjectDirectory().string();
			std::string path = projectFolder + "\\Assets\\";
			MADAM_INFO("Scanning for scripts in: {0}", path);

			for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
				if (std::filesystem::is_regular_file(entry.path()) && entry.path().extension() == ".cpp") {
					MADAM_INFO(entry.path().string());
					std::string newPath = entry.path().string();
					newPath = newPath.substr(projectFolder.length());
					std::replace(newPath.begin(), newPath.end(), '\\', '/');
					MADAM_INFO("Formatted Path: {0} ", newPath);
					std::string className = ReadClassName(newPath);
					if (className != "") {
						scripts[className] = newPath;
					}
				}
			}
		}

		std::string ScriptEngine::ReadClassName(std::string _path)
		{
			std::string path = Project::Get().getProjectDirectory().string() + _path;
			std::ifstream script(path);
			std::vector<std::string> lines;
			if (script.is_open()) {
				std::string line;
				while (std::getline(script, line)) {
					lines.push_back(line);
				}
				script.close();
			}
			else if (script.fail()) {
				MADAM_ERROR("Unable to open file: {0}", path);
				perror("Error: ");
				return "";
			}
			else {
				MADAM_ERROR("Unable to open file: {0}", path);
				return "";
			}
			size_t pos = lines[4].find("class");
			if (pos != std::string::npos) {
				std::string className = lines[4].substr(pos + 6);
				pos = className.find(" ");
				if (pos != std::string::npos) {
					className = className.substr(0, pos);
				}
				else {
					MADAM_ERROR("Unable to format class name: \"{0}\" Correctly", className);
					return "";
				}
				MADAM_INFO("Class Name: {0}", className);
				return className;
			}
			else {
				MADAM_ERROR("Unable to find class name in file: {0}", path);
				return "";
			}
			return "";
		}
	}
}