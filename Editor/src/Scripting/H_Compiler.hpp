#pragma once
#include <Maiden.hpp>
#include <fstream>
#include <filesystem>

namespace Madam {
	namespace Scripting {

		class NativeCompiler {
		public:
			NativeCompiler();
			~NativeCompiler();

			void Compile();
			void Construct();
			void Import();

			void OnCreateScript(std::string file);
			void OnDeleteScript();
			void SoftScanScripts();
			void HardScanScripts();
		private:
			
			std::string ReadClassName(std::string path);
			std::map<std::string, std::string> scripts;
		};

	}
}