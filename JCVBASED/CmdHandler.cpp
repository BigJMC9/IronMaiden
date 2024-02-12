#include "H_CmdHandler.hpp"


namespace Digestion {
	namespace App {

		void CommandHandler::HandleCommand(std::string command, PipeHandler& handler, SceneSerializer* scene) {

			if (command[0] == '?') {
				std::string message;
				message = "Commands: \nInstantiate\nLoad [Scene Path]\nSave [Scene Path]\n";
				handler.Write(message);
				return;
			}

			std::stringstream commandStream(command);
			std::vector<std::string> commandWords;

			std::string word;
			while (commandStream >> word) {
				commandWords.push_back(word);
			}

			if (commandWords[0] == "Instantiate") {
				HandleObject(commandWords[0]);
			}
			else if (commandWords[0] == "Load") {
				if (commandWords.size() > 1) {
					scene->Deserialize(commandWords[1]);
				}
				else {
					scene->Deserialize("scenes/temp.scene");
				}
			}
			else if (commandWords[0] == "Save") {
				if (commandWords.size() > 1) {
					scene->Serialize(commandWords[1]);
				}
				else {
					scene->Serialize("scenes/temp.scene");
				}
			}
		}

		void CommandHandler::HandleObject(std::string command) {

		}

		void CommandHandler::HandleScene(std::string command) {

		}

	}
}