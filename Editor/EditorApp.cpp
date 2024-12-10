#include <Maiden.hpp>
#include "src/H_EditorInterface.hpp"
// Entry Point
#include "Maiden/Core/Main/EntryPoint.hpp"
#include <filesystem>

namespace Madam{
	class Editor : public Madam::Application {
	public:
		Editor() : Madam::Application()
		{
			std::unique_ptr<EditorLayer> editorSurface = std::make_unique<EditorLayer>(EditorLayer());
			AddSurface(std::move(editorSurface));
			MADAM_CORE_INFO("Editor Created");
		}

		~Editor()
		{

		}

	};

	//Here's the issue
	Madam::Application* Madam::CreateApplication()
	{
		try {
			return new Editor();
		}
		catch (const std::filesystem::filesystem_error& e) {
			MADAM_CORE_ERROR("Error: {0}", e.what());
			std::cin.get();
		}
		catch (const std::exception& e) {
			MADAM_CORE_ERROR("Error: {0}", e.what());
			std::cin.get();
		}
		return nullptr;
	}
}
