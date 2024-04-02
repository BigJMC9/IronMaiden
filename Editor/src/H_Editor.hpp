#pragma once

#include <Maiden.hpp>

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
		KeyboardMovementController cameraController{};
		std::shared_ptr<Entity> viewerObject = nullptr;
		bool isFirst = true;
	};
}