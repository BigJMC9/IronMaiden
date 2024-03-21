#include "H_Editor.hpp"

namespace Madam {
	EditorSurface::EditorSurface() : Surface("Editor") 
	{

	}

	void EditorSurface::OnAttach()
	{
		MADAM_INFO("EditorSurface Attached");

		
		camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.f, 0.f, 0.f));
		viewerObject = Application::Get().getScene().CreateEntity();
		viewerObject.SetName("Editor Camera");
		viewerObject.GetComponent<Transform>().translation.z = -2.5f;
		Application::Get().pCamera = std::make_unique<Camera>(camera);
	}

	void EditorSurface::OnUpdate()
	{
		cameraController.moveInPlaneXZ(Application::Get().getWindow().getGLFWwindow(), Application::Get().getTime().GetDeltaTime(), viewerObject);
		camera.setViewYXZ(viewerObject.GetComponent<Transform>().translation, viewerObject.GetComponent<Transform>().rotation);
		float aspect = Application::Get().getAspectRatio();
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 500.0f);
		MADAM_INFO("EditorSurface Updated");
	}
}