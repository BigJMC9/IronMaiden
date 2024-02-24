#include "maidenpch.hpp"
#include "H_CmdHandler.hpp"
#include "H_RenderSystems.hpp"
#include "H_Application.hpp"


// std
#include <cctype> 

namespace Madam {
	namespace App {

		static std::string toLowerCase(const std::string& str) {
			std::string result;
			for (char c : str) {
				result += std::tolower(c);
			}
			return result;
		}

		//Handle on seperate thread and then use switch with int to handle command when meeting with main thread
		//All essential options should now be restructured to be available to be accessed by this command handler
		void CommandHandler::HandleCommand(std::string command, PipeHandler& handler, SceneSerializer* scene) {

			if (command[0] == '?') {
				std::string message;
				message = "Commands: \ninstantiate\nload [Scene Path]\nsave [Scene Path]\n";
				message += "[!n!]";
				std::cout << message << std::endl;
				handler.Write(message);
				handler.Write("[!n!]");
				return;
			}

			std::stringstream commandStream(command);
			std::vector<std::string> commandWords;

			std::string word;
			while (commandStream >> word) {
				commandWords.push_back(toLowerCase(word));
			}

			if (commandWords[0] == "instantiate") {
				HandleObject(commandWords);
			}
			else if (commandWords[0] == "scene") {
				HandleScene(commandWords, handler, scene);
			}
			else if (commandWords[0] == "render") {
				HandleRendering(commandWords, handler, scene);
			}
			else if (commandWords[0] == "snap") {
				Application::Get().debug = true;
			}

			if (!handler.Write("[!n!]")) {
				std::cout << "Error: unable to send end of write to pipe!" << std::endl;
			}
		}

		void CommandHandler::HandleRendering(std::vector<std::string> commandWords, PipeHandler& handler, SceneSerializer* scene) {
			if (commandWords.size() < 2) {
				handler.Write("Commands for Rendering: \nrender switch [Render System number] [Render System number] \nrender order ");
			}
			else if (commandWords[1] == "order") {
				auto renderSystems = Application::Get().getRenderSystems();
				std::string systemNames = "";
				int i = 0;
				for (auto& system : renderSystems) {
					systemNames += std::to_string(i) + " - " + system->name + "\n";
					i += 1;
				}
				handler.Write(systemNames);
				std::cout << systemNames << std::endl;
			}
			else if (commandWords[1] == "switch") {
				if (commandWords.size() > 3) {
					int first = std::stoi(commandWords[2]);
					int second = std::stoi(commandWords[3]);
					Application::Get().getMasterRenderSystem().switchRenderSystems(first, second);
				}
				else {
					handler.Write("Commands for Rendering: \nrender switch [Render System number] [Render System number]");
				}
			}
		}

		void CommandHandler::HandleObject(std::vector<std::string> commandWords) {

		}

		void CommandHandler::HandleScene(std::vector<std::string> commandWords, PipeHandler& handler, SceneSerializer* scene) {
			if (commandWords.size() < 2) {
				//handler.Write("Commands for Scene: \nscene load [Scene Path]\nscene save [Scene Path]");
			}
			else if (commandWords[1] == "load") {
				if (commandWords.size() > 2) {
					scene->Deserialize(commandWords[2]);
				}
				else {
					scene->Deserialize("scenes/temp.scene");
				}
			}
			else if (commandWords[1] == "save") {
				if (commandWords.size() > 2) {
					scene->Serialize(commandWords[2]);
				}
				else {
					scene->Serialize("scenes/temp.scene");
				}
			}
		}

	}
}