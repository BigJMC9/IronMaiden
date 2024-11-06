#pragma once
#include <Maiden.hpp>
#include <fstream>
#include <filesystem>

namespace Madam {
	namespace Scripting {

		struct WindowsError {
			DWORD errorCode;
			std::string errorMsg;
			LPVOID lpMsgBuf;

			std::string ToString() {
				return std::to_string(errorCode) + ": " + errorMsg;
			}
		};

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

			typedef void (*SetAppInstance)(Application* app);
			typedef CNativeScript(*BindType)(std::string);

			bool isImported = false;
			HINSTANCE hGetProcIDDLL = NULL;
			std::wstring stringToWSTR(const std::string& s) {
				int size_needed = MultiByteToWideChar(CP_UTF8, 0, &s[0], (int)s.size(), NULL, 0);
				std::wstring wstrTo(size_needed, 0);
				MultiByteToWideChar(CP_UTF8, 0, &s[0], (int)s.size(), &wstrTo[0], size_needed);
				return wstrTo;
			}

			WindowsError GetWindowsError() {
				WindowsError windowsError = { GetLastError(), "", nullptr };

				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					windowsError.errorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&windowsError.lpMsgBuf,
					0,
					NULL
				);
				std::wstring wideStr((wchar_t*)windowsError.lpMsgBuf);
				std::string narrowStr(wideStr.begin(), wideStr.end());
				windowsError.errorMsg = narrowStr;
				return windowsError;
			}

			void ReleaseLibrary();
			void CreateCMDProcess(std::string path);

			std::string ReadClassName(std::string path);
			void UpdateBehaviour();
			bool CopyScript(std::string src, std::string dest);
			std::map<std::string, std::string> scripts;

			std::vector<std::string> formatIF(std::string _class) {
				return std::vector<std::string> { 
					"\t\tif (type == \"" + _class + "\") {",
					"\t\t\tCNativeScript nsc;",
					"\t\t\tnsc.Bind<" + _class + ">();",
					"\t\t\treturn nsc;",
					"\t\t}"
				};
			}

			std::vector<std::string> formatELIF(std::string _class) {
				return std::vector<std::string> {
					"\t\telse if (type == \"" + _class + "\") {",
					"\t\t\tCNativeScript nsc;",
					"\t\t\tnsc.Bind<" + _class + ">();",
					"\t\t\treturn nsc;",
					"\t\t}"
				};
			}

			std::vector<std::string> formatELSE{
					"\t\telse {",
					"\t\t\tstd::cout << \"No script found\" << std::endl;",
					"\t\t\treturn CNativeScript();",
					"\t\t}"
			};
		};
	}
}