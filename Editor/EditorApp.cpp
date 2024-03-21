#include <Maiden.hpp>
#include "src/H_Editor.hpp"
// Entry Point
#include "Maiden/Core/Main/EntryPoint.hpp"


namespace Madam{
	class Editor : public Madam::Application {
	public:
		Editor() : Madam::Application()
		{
			EditorSurface editorSurface = {};
			addSurface(editorSurface);
			//peditorSur = new Madam::EditorSurface();

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
