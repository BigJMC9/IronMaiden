#pragma once
#include <Maiden.hpp>
#include <fstream>
#include <filesystem>

namespace Madam {
	namespace Scripting {

		class ScriptEngine {
		public:
			ScriptEngine();
			~ScriptEngine();

			//void Compile();
			bool Runtime(Entity entity);
			void Update();
			bool Import(Entity entity);

			void OnCreateScript(std::string file);
			void OnDeleteScript();
			//void SoftScanScripts();
			void RescanScripts();

			void ShowScripts() {
				for (auto& script : scripts) {
					MADAM_INFO("{0} at: {1}", script.first, script.second);
				}
			}

			bool CheckImportStatus() {
				return isImported;
			}
		private:
			

			bool isImported = false;
			HINSTANCE hGetProcIDDLL = NULL;
			std::wstring stringToWSTR(const std::string& s) {
				int size_needed = MultiByteToWideChar(CP_UTF8, 0, &s[0], (int)s.size(), NULL, 0);
				std::wstring wstrTo(size_needed, 0);
				MultiByteToWideChar(CP_UTF8, 0, &s[0], (int)s.size(), &wstrTo[0], size_needed);
				return wstrTo;
			}
			void ReleaseLibrary();
			

			std::string ReadClassName(std::string path);
			void UpdateBehaviour();
			bool CopyScript(std::string src, std::string dest);
			std::map<std::string, std::string> scripts;

			std::vector<std::string> formatIF(std::string _class) {
				return std::vector<std::string> { 
					"\t\tif (type == \"" + _class + "\") {",
					"\t\t\tNativeScriptComponent nsc;",
					"\t\t\tnsc.Bind<" + _class + ">();",
					"\t\t\treturn nsc;",
					"\t\t}"
				};
			}

			std::vector<std::string> formatELIF(std::string _class) {
				return std::vector<std::string> {
					"\t\telse if (type == \"" + _class + "\") {",
					"\t\t\tNativeScriptComponent nsc;",
					"\t\t\tnsc.Bind<" + _class + ">();",
					"\t\t\treturn nsc;",
					"\t\t}"
				};
			}

			std::vector<std::string> formatELSE{
					"\t\telse {",
					"\t\t\tstd::cout << \"No script found\" << std::endl;",
					"\t\t\treturn NativeScriptComponent();",
					"\t\t}"
			};

			/*std::vector<std::string> includes{
				"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.39.33519/include",
				"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.39.33519/atlmfc/include",
				"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/VS/include",
				"C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt",
				"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/VS/UnitTest/include",
				"C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/um",
				"C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/shared",
				"C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/winrt",
				"C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/cppwinrt",
				"C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8.1/Include/um",
				"C:/Program Files/VulkanSDK/1.3.268.0/include",
				"../IronMaiden/vendors/glfw-3.3.8.bin.WIN64/include",
				"../IronMaiden/vendors/glm",
				"../IronMaiden/vendors/spdlog/include",
				"../IronMaiden/vendors/entt/single_include",
				"../IronMaiden/src",
				"../IronMaiden"
			};

			std::vector<std::string> preprocessors{
				"MADAM_PLATFORM_WINDOWS",
				"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",
				"_WINDLL",
				"_UNICODE",
				"UNICODE"
			};

			std::vector<std::string> libPath{
				"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.39.33519/lib/x64",
				"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.39.33519/atlmfc/lib/x64",
				"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/VS/lib/x64",
				"C:/Program Files (x86)/Windows Kits/10/lib/10.0.22621.0/ucrt/x64",
				"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/VS/UnitTest/lib",
				"C:/Program Files (x86)/Windows Kits/10/lib/10.0.22621.0/um/x64",
				"C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8.1/lib/um/x64"
			};

			std::vector<std::string> libraries{

			};
			//std::map<std::string, T> scriptClass;
			*/
		};

	}
}