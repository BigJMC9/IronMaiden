#include "H_ScriptEngine.hpp"
#include <algorithm>

namespace Madam
{
	namespace Scripting {

		ScriptEngine::ScriptEngine()
		{
			//Constructor
		}

		ScriptEngine::~ScriptEngine()
		{
			if (isImported) {
				ReleaseLibrary();
			}
			//Destructor
		}
		
		bool ScriptEngine::Runtime(Entity entity)
		{
			if (isImported) {
				ReleaseLibrary();
			}
			//Update();
			//Application::Get().RuntimeStart();
			return Import(entity);
		}

		void ScriptEngine::Update() {
			RescanScripts();
			UpdateBehaviour();
		}

		//Include premake into editor build
		//Use premake to generate new solution for the project
		//Then use cl to compile the project
		//Don't worry about this. This is a future feature
		//Import DLL for now and focus on cleaning code
		/*void ScriptEngine::Compile()
		{
			std::string path = "./" + Application::Get().getConfig().assets;
			std::string pathint = "./" + Application::Get().getConfig().internals + "bin-int";
			std::string commandPrefix = "cl /JMC /ZI /std:c++17 /GS /Gm- /Od /fp:precise /Zc:inline /external:W3 /MDd /Gd /EHsc /errorReport:prompt /WX- /RTC1 /FC /ifcOutput \""
				+ pathint + "/\" /Fa\"" + pathint + "/\" /Fo\"" + pathint + "/\" /Fp\"" + pathint + "/runtime.pch\" ";
			std::string includesDir = "";
			for (const auto& include : includes) {
				includesDir += "/I\"" + include + "\" ";
			}
			includesDir += "/I\"./Library\" ";
			std::string macros = "";
			for (const auto& macro : preprocessors) {
				macros += "/D \"" + macro + "\" ";
			}
			std::string scriptDir = "/LD ";
			for (const auto& script : scripts) {
				scriptDir += script.second + " ";
			}
			std::string libsDir = "";
			for (const auto& lib : libPath) {
				libsDir += "/link /LIBPATH:\"" + lib + "\" ";
			}
			std::string libs = "";
			for (const auto& lib : libraries) {
				libs += "\"" + lib + ".lib\" ";
			}
			scriptDir += "./Library/NativeBehaviour.cpp ";
			std::string outputDir = "/out:\"" + Application::Get().getConfig().internals + "runtime.dll\" /Fe:\"./" + Application::Get().getConfig().internals + "runtime.dll\"";
			std::string customArgs = " > buildoutput.log 2>&1";

			std::string command = commandPrefix + includesDir + macros + scriptDir + libsDir + libs + outputDir + customArgs;
			MADAM_INFO("Executing Command: {0}", command);
			system(command.c_str());
			//Compile all scripts
		}*/

		bool ScriptEngine::Import(Entity entity)
		{
			std::string dllDir = Application::Get().getConfig().projectFolder + "Library/grt.dll";
			MADAM_INFO("Importing DLL: {0}", dllDir);
			hGetProcIDDLL = LoadLibrary(stringToWSTR(dllDir).c_str());
			if (!hGetProcIDDLL) {
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
				char* errormessage = (char*)lpMsgBuf;
				size_t len = strlen(errormessage);
				if (len > 0 && errormessage[len - 1] == '\n') {
					errormessage[len - 1] = '\0';
				}
				MADAM_ERROR("Unable to load DLL Function. Error code: {0}: {1}", errorCode, errormessage);
				FreeLibrary(hGetProcIDDLL);
				return false;
			}

			typedef NativeScriptComponent (*BindType)(std::string);

			BindType getScript = (BindType)GetProcAddress(hGetProcIDDLL, "BindType");
			if (!getScript) {
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
				char* errormessage = (char*)lpMsgBuf;
				size_t len = strlen(errormessage);
				if (len > 0 && errormessage[len - 1] == '\n') {
					errormessage[len - 1] = '\0';
				}
				MADAM_ERROR("Unable to Load Function. Error code: {0}: {1}", errorCode, errormessage);
				FreeLibrary(hGetProcIDDLL);
				return false;
			}

			entity.AddComponent<NativeScriptComponent>();
			entity.GetComponent<NativeScriptComponent>() = getScript("controller");

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
			std::ifstream samplefile(Application::Get().getConfig().internals + "sample.ims", std::ios::binary);
			if (std::ifstream(Application::Get().getConfig().assets + filePath)) {
				MADAM_ERROR("File already Exists");
				return;
			}
			std::ofstream newfile(Application::Get().getConfig().assets + filePath, std::ios::binary);

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
			RescanScripts();
			UpdateBehaviour();
		}

		void ScriptEngine::UpdateBehaviour()
		{
			if (scripts.size() == 0) {
				MADAM_ERROR("No scripts found");
				return;
			}

			std::string path = Application::Get().getConfig().projectFolder + Application::Get().getConfig().internals + "NativeBehaviour.cpp";
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

			std::ofstream newBehaviour(Application::Get().getConfig().projectFolder + "Library/NativeBehaviour.cpp");
			if (newBehaviour.is_open()) {
				for (const auto& newline : lines) {
					newBehaviour << newline << "\n";
				}
			}
			else {
				std::string libLoc = Application::Get().getConfig().projectFolder + "Library/NativeBehaviour.cpp";
				MADAM_ERROR("Unable to open file: {0}", libLoc);
			}
			newBehaviour.close();

			std::string srcpath = Application::Get().getConfig().projectFolder + Application::Get().getConfig().internals + "H_NativeBehaviour.hpp";
			std::string dstpath = Application::Get().getConfig().projectFolder + "Library/H_NativeBehaviour.hpp";
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

		/*void ScriptEngine::SoftScanScripts()
		{
			for (const auto& pair : scripts) {
				MADAM_INFO("Script: {0} at: {1}", pair.first, pair.second);
			}
			//Scan all scripts
		}*/

		void ScriptEngine::RescanScripts() 
		{
			scripts.clear();
			std::string projectFolder = Application::Get().getConfig().projectFolder;
			std::string path = projectFolder + Application::Get().getConfig().assets;
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

		std::string ScriptEngine::ReadClassName(std::string path)
		{
			std::ifstream script(path);
			std::vector<std::string> lines;
			if (script) {
				std::string line;
				while (std::getline(script, line)) {
					lines.push_back(line);
				}
				script.close();
			}
			else {
				MADAM_ERROR("Unable to open file: {0}", path);
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