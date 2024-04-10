#include "H_Compiler.hpp"
#include <algorithm>

namespace Madam
{
	namespace Scripting {

		NativeCompiler::NativeCompiler()
		{
			//Constructor
		}

		NativeCompiler::~NativeCompiler()
		{
			//Destructor
		}
		
		void NativeCompiler::Construct()
		{
			HardScanScripts();
			
		}

		void NativeCompiler::Compile()
		{
			std::string path = "./" + Application::Get().getConfig().assets;
			std::string command = "cl /LD /Fe:./" + Application::Get().getConfig().internals + "runtime.dll > buildoutput.log 2>&1";
			std::string insert = "";
			for (const auto& pair : scripts) {
				insert += pair.second + " ";
				MADAM_CORE_INFO("Compiling: {0}", insert);
			}
			command.insert(7, insert);
			MADAM_CORE_INFO("Executing Command: {0}", command);
			system(command.c_str());
			//Compile all scripts
		}

		void NativeCompiler::Import()
		{
			//Import Runtime DLL
		}

		//Creates a new script
		void NativeCompiler::OnCreateScript(std::string filePath)
		{
			std::ifstream samplefile(Application::Get().getConfig().internals + "sample.ims", std::ios::binary);
			if (std::ifstream(Application::Get().getConfig().assets + filePath)) {
				MADAM_CORE_ERROR("File already Exists");
				return;
			}
			std::ofstream newfile(Application::Get().getConfig().assets + filePath, std::ios::binary);

			if (!samplefile) {
				MADAM_CORE_ERROR("Cannot Open Sample File for Native Scripts");
				return;
			}
			if (!newfile) {
				MADAM_CORE_ERROR("Cannot Create new File");
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
			MADAM_CORE_INFO("Sample File:");
			for (std::string debugLine : lines)
			{
				MADAM_CORE_INFO(debugLine);
			}
			samplefile.close();
			
			MADAM_CORE_INFO("Replacing line: {0}", lines[0]);
			MADAM_CORE_INFO("with {0}", fileName);
			MADAM_CORE_INFO("as //Class: {0}", fileName);
			std::string replace = "//Class: " + fileName;
			lines[0] = replace;
			std::string search = "SampleClass";
			size_t pos = lines[4].find("SampleClass");
			if (pos != std::string::npos) {
				lines[4].replace(pos, search.length(), fileName);
			}
			MADAM_CORE_INFO("New File:");
			for (const auto& newline : lines) {
				MADAM_CORE_INFO(newline);
				newfile << newline << "\n";
			}
			
			newfile.close();

			scripts[fileName] = filePath;
			SoftScanScripts();
		}

		void NativeCompiler::OnDeleteScript()
		{
			//Delete a script
		}

		void NativeCompiler::SoftScanScripts()
		{
			for (const auto& pair : scripts) {
				MADAM_CORE_INFO("Script: {0} at: {1}", pair.first, pair.second);
			}
			//Scan all scripts
		}

		void NativeCompiler::HardScanScripts() 
		{
			std::string path = "./" + Application::Get().getConfig().assets;
			for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
				if (std::filesystem::is_regular_file(entry.path()) && entry.path().extension() == ".cpp") {
					MADAM_CORE_INFO(entry.path().string());
					std::string newPath = entry.path().string();
					std::replace(newPath.begin(), newPath.end(), '\\', '/');
					MADAM_CORE_INFO("Formatted Path: {0} ", newPath);
					std::string className = ReadClassName(newPath);
					if (className != "") {
						scripts[className] = newPath;
					}
				}
			}
		}

		std::string NativeCompiler::ReadClassName(std::string path)
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
				MADAM_CORE_ERROR("Unable to open file: {0}", path);
			}
			size_t pos = lines[4].find("class");
			if (pos != std::string::npos) {
				std::string className = lines[4].substr(pos + 6);
				pos = className.find(" ");
				if (pos != std::string::npos) {
					className = className.substr(0, pos);
				}
				else {
					MADAM_CORE_ERROR("Unable to format class name: \"{0}\" Correctly", className);
					return "";
				}
				MADAM_CORE_INFO("Class Name: {0}", className);
				return className;
			}
			else {
				MADAM_CORE_ERROR("Unable to find class name in file: {0}", path);
				return "";
			}
			return "";
		}
	}
}