#pragma once

#include <Maiden.hpp>
#include "Maiden/Scene/H_Scene.hpp"
#include "Maiden/Scene/H_Entity.hpp"
#include "Maiden/Scene/ScriptableEntity.hpp"
#include "Maiden/Events/H_Input.hpp"

namespace Madam {
	class EditorSurface : public Surface {
	public:
		EditorSurface();
		virtual ~EditorSurface() = default;

		virtual void OnAttach() override;
		//virtual void OnDetach() override;

		virtual void OnUpdate() override;

	private:
		KeyboardMovementController cameraController{};
		Entity viewerObject;
		Camera camera{};
	};
}