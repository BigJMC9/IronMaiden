#include "H_Editor.hpp"

namespace Madam {
	EditorSurface::EditorSurface() : Surface("Editor") 
	{

	}

	void EditorSurface::OnAttach()
	{
		MADAM_INFO("Editor Server Attached");

		Rendering::CameraData cameraData;
		cameraData.projectionType = Rendering::CameraData::ProjectionType::Perspective;
		cameraData.perspective = Rendering::CameraData::Perspective(glm::radians(50.0f), Application::Get().getAspectRatio(), 0.1f, 1000.0f);
		Entity& obj = Application::Get().getScene().CreateEntity();
		obj.GetComponent<Object>().name = "Editor Camera";
		obj.GetComponent<Transform>().translation.z = -2.5f;
		obj.AddComponent<Camera>(cameraData);
		viewerObject = std::make_shared<Entity>(obj);
		viewerObject->GetComponent<Camera>().cameraHandle->setViewDirection(glm::vec3(0.f, 2.0f, 0.f), glm::vec3(0.f, 0.f, 0.f));
		viewerObject->GetComponent<Camera>().cameraHandle->setMain();
		if (viewerObject->GetComponent<Camera>().cameraHandle->isMain()) {
			MADAM_INFO("Camera is main camera");
		}
		else {
			MADAM_INFO("Camera is not main camera");
		}
		entt::entity& entityID = obj.GetHandleAsRef();
		MADAM_CORE_INFO("Entity ID: {0} at: {1}", (uint32_t)entityID, obj.GetHandleMemoryLocation());
	}

	void EditorSurface::OnUpdate()
	{
		if (isFirst) {

			std::string debugStr;
			Application::Get().getScene().Reg().view<entt::entity>().each([&](auto entityID) {
				std::stringstream ss;
				ss << "Entity ID: " << std::to_string((uint32_t)entityID) << " at: " << static_cast<void*>(&entityID) << "\n";
				debugStr += ss.str();
				Entity entity = { entityID, &Application::Get().getScene() };
				if (!entity) return;
				});
			MADAM_INFO(debugStr);
			isFirst = false;
			entt::entity& entityID = viewerObject->GetHandleAsRef();
			MADAM_INFO("Entity ID; {0} at: {1}", (uint32_t)entityID, viewerObject->GetHandleMemoryLocation());

			//Madam::Rendering::CameraHandle& cameraHandle = viewerObject->GetComponent<Camera>().cameraHandle->get();

			if (viewerObject->GetComponent<Camera>().cameraHandle->isMain()) {
				MADAM_INFO("Camera is main camera");
			}
			else {
				MADAM_INFO("Camera is not main camera");
			}

			if (Madam::Rendering::CameraHandle::getMain().isMain()) {
				MADAM_INFO("Is main function is working");
			}
		}
		
		Entity& obj = *viewerObject.get();
		cameraController.moveInPlaneXZ(Application::Get().getWindow().getGLFWwindow(), Application::Get().getTime().GetDeltaTime(), obj);
		Camera& camera = viewerObject->GetComponent<Camera>();
		camera.cameraHandle->setViewYXZ(viewerObject->GetComponent<Transform>().translation, viewerObject->GetComponent<Transform>().rotation);
		float aspect = Application::Get().getAspectRatio();
		camera.cameraHandle->setProjection();
		//MADAM_INFO("EditorSurface Updated");
	}

	//Tag component could fix camera issue?
	//Should be handled as an event
	void EditorSurface::OnSceneLoad() {
		MADAM_INFO("Scene Loaded");
		Application::Get().getScene().Reg().view<entt::entity>().each([&](auto entityID) {
			Entity entity = { entityID, &Application::Get().getScene() };
			if (!entity) return;
			if (entity.HasComponent<Camera>()) {
				if (entity.GetComponent<Camera>().cameraHandle->isMain())
				{
					MADAM_INFO("Camera is main camera");
					viewerObject = std::make_shared<Entity>(entity);
				}
				else {
					MADAM_INFO("Camera is not main camera");
				}
			}
		});

	}
}