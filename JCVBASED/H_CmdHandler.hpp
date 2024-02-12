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
			static void HandleObject(std::string command);
			static void HandleScene(std::string command);
		};
	}
}