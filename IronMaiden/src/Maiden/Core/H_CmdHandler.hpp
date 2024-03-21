#pragma once
#include "maidenpch.hpp"
#include "Main/Core.hpp"
#include "H_ProcessHandler.hpp"
#include "../Scene/H_SceneSerializer.hpp"

namespace Madam {
	namespace App {
		class MADAM_API CommandHandler {
		public:
			static void HandleCommand(std::string command, PipeHandler& handler, SceneSerializer* scene);
		private:
			static void HandleObject(std::vector<std::string> commandWords, PipeHandler& handler);
			static void HandleScene(std::vector<std::string> commandWords, PipeHandler& handler, SceneSerializer* scene);
			static void HandleRendering(std::vector<std::string> commandWords, PipeHandler& handler, SceneSerializer* scene);
			static void HandleRuntime(std::vector<std::string> commandWords, PipeHandler& handler);
		};
	}
}