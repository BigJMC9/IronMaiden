#pragma once
#include "H_ProcessHandler.hpp"
#include "H_SceneSerializer.hpp"

//std
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

namespace Digestion {
	namespace App {
		class CommandHandler {
		public:
			static void HandleCommand(std::string command, PipeHandler& handler, SceneSerializer* scene);
		private:
			static void HandleObject(std::vector<std::string> commandWords);
			static void HandleScene(std::vector<std::string> commandWords, PipeHandler& handler, SceneSerializer* scene);
			static void HandleRendering(std::vector<std::string> commandWords, PipeHandler& handler, SceneSerializer* scene);
		};
	}
}