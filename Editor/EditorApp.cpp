#include <Maiden.hpp>
#include "src/H_Editor.hpp"
// Entry Point
#include "Maiden/Core/Main/EntryPoint.hpp"


namespace Madam{
	class Editor : public Madam::Application {
	public:
		Editor() : Madam::Application()
		{
			std::unique_ptr<EditorLayer> editorSurface = std::make_unique<EditorLayer>(EditorLayer());
			addSurface(std::move(editorSurface));
			MADAM_CORE_INFO("Editor Created");

		}

		~Editor()
		{

		}

	};

	Madam::Application* Madam::CreateApplication()
	{
		return new Editor();
	}
}
