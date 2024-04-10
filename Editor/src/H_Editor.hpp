#pragma once

#include <Maiden.hpp>
#include "Scripting/H_Compiler.hpp"

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
		std::shared_ptr<Entity> viewerObject = nullptr;
		bool isFirst = true;
		std::shared_ptr<Scripting::NativeCompiler> compiler = nullptr;
	};
}