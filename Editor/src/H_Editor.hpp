#pragma once

#include <Maiden.hpp>
#include "Scripting/H_ScriptEngine.hpp"

namespace Madam {
	class EditorSurface : public Surface {
	public:
		EditorSurface();
		virtual ~EditorSurface() = default;

		virtual void OnAttach() override;
		//virtual void OnDetach() override;

		void OnUpdate() override;

		void OnSceneLoad() override;

	private:
		Entity viewerObject;
		bool isFirst = true;
		std::shared_ptr<Scripting::ScriptEngine> scriptEngine = nullptr;
		std::shared_ptr<Scene> scene = nullptr;
	};
}