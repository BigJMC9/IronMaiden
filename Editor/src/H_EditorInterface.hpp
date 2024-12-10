#pragma once

#include <Maiden.hpp>
#include "Scripting/H_ScriptEngine.hpp"

namespace Madam {
	class EditorLayer : public EngineInterface {
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		void OnUpdate() override;

		void SetUpEvents();
		void OnSceneChangeEvent(SceneChangeEvent* e);
	private:
		//Entity viewerObject;
		bool isFirst = true;
		std::shared_ptr<Scripting::ScriptEngine> scriptEngine = nullptr;
		std::shared_ptr<Scene> scene = nullptr;
	};
}