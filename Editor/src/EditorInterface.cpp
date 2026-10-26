#include "H_EditorInterface.hpp"

namespace Madam {

	class CameraController : public ScriptableEntity {
	public:
		float moveSpeed = 5.0f;
		float mouseSensitivity = 0.003f;
		float panSensitivity = 0.008f;
		float scrollSensitivity = 2.0f;
		glm::vec2 mouseDelta{ 0.0f };
		glm::vec2 mouseScroll{ 0.0f };
		glm::vec2 lastMousePos{ 0.0f };
		glm::vec2 smoothedMouseDelta{ 0.0f };
		glm::vec3 velocity{ 0.0f };
		bool hasLastMousePos = false;

		void Create() {
			Events::EventSystem::Get().AddListener(this, &CameraController::OnMouseMoveRawEvent);
			Events::EventSystem::Get().AddListener(this, &CameraController::OnMouseScrollEvent);
		}
		void Destroy() {
			Events::EventSystem::Get().RemoveListener(this, &CameraController::OnMouseMoveRawEvent);
			Events::EventSystem::Get().RemoveListener(this, &CameraController::OnMouseScrollEvent);
		}
		void Start() {}
		void Update() {
			if (!GetComponent<CCamera>().cameraHandle->IsMain()) return;
			GLFWwindow* window = Application::Get().GetWindow().getGLFWwindow();
			const float dt = glm::clamp(Application::Get().GetTime().GetDeltaTime(), 0.0f, 0.05f);
			const bool captured = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
			const bool looking = captured && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
			const bool panning = captured && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
			const float blend = 1.0f - std::exp(-18.0f * dt);
			const glm::vec2 inputDelta = mouseDelta;
			mouseDelta = { 0.0f, 0.0f };
			smoothedMouseDelta = glm::mix(smoothedMouseDelta,
				(looking || panning) ? inputDelta : glm::vec2(0.0f), blend);

			auto& transform = GetComponent<CTransform>();
			if (looking) {
				const glm::quat yaw = glm::angleAxis(smoothedMouseDelta.x * mouseSensitivity, glm::vec3(0, -1, 0));
				const glm::quat yawed = glm::normalize(yaw * transform.rotation);
				const glm::quat pitch = glm::angleAxis(smoothedMouseDelta.y * mouseSensitivity, glm::vec3(1, 0, 0));
				const glm::quat candidate = glm::normalize(yawed * pitch);
				const float pitchAngle = std::asin(glm::clamp((candidate * glm::vec3(0, 0, 1)).y, -1.0f, 1.0f));
				transform.rotation = std::abs(pitchAngle) < glm::radians(89.0f) ? candidate : yawed;
			}

			const glm::vec3 forward = transform.rotation * glm::vec3(0, 0, 1);
			const glm::vec3 right = transform.rotation * glm::vec3(1, 0, 0);
			const glm::vec3 up = transform.rotation * glm::vec3(0, -1, 0);
			if (panning)
				transform.translation += (right * smoothedMouseDelta.x - up * smoothedMouseDelta.y) * panSensitivity;

			glm::vec3 direction(0.0f);
			if (looking) {
				if (Input::Get().IsKeyPress(KeyCode::W)) direction += forward;
				if (Input::Get().IsKeyPress(KeyCode::S)) direction -= forward;
				if (Input::Get().IsKeyPress(KeyCode::A)) direction -= right;
				if (Input::Get().IsKeyPress(KeyCode::D)) direction += right;
				if (Input::Get().IsKeyPress(KeyCode::E)) direction += up;
				if (Input::Get().IsKeyPress(KeyCode::Q)) direction -= up;
				moveSpeed = glm::clamp(moveSpeed + mouseScroll.y, 0.25f, 50.0f);
			} else if (!panning) {
				transform.translation += forward * mouseScroll.y * scrollSensitivity;
			}
			mouseScroll = { 0.0f, 0.0f };
			const glm::vec3 desiredVelocity = glm::length(direction) > 0.0f
				? glm::normalize(direction) * moveSpeed : glm::vec3(0.0f);
			velocity = glm::mix(velocity, desiredVelocity, 1.0f - std::exp(-12.0f * dt));
			transform.translation += velocity * dt;
		}
		void LateUpdate() {}
		void Render() {}

		void OnMouseMoveRawEvent(MouseMoveRawEvent* e) {
			GLFWwindow* window = Application::Get().GetWindow().getGLFWwindow();
			if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED ||
				(!Input::Get().IsMouseButtonPress(MouseCode::RIGHTMOUSEBUTTON) &&
				 !Input::Get().IsMouseButtonPress(MouseCode::MIDDLEMOUSEBUTTON))) {
				hasLastMousePos = false;
				mouseDelta = { 0.0f, 0.0f };
				return;
			}
			const glm::vec2 pos{ static_cast<float>(e->x), static_cast<float>(e->y) };
			if (hasLastMousePos) {
				const glm::vec2 delta = lastMousePos - pos;
				if (glm::length(delta) < 100.0f) mouseDelta += delta;
			}
			lastMousePos = pos;
			hasLastMousePos = true;
		}
		void OnMouseScrollEvent(MouseScrollEvent* e) {
			mouseScroll += glm::vec2(static_cast<float>(e->x), static_cast<float>(e->y));
		}
	};

	EditorLayer::EditorLayer() : EngineInterface("Editor") 
	{

	}

	void EditorLayer::OnAttach()
	{
		MADAM_INFO("Editor Server Attached");
		scriptEngine = CreateRef<Scripting::ScriptEngine>();
		Rendering::CameraData cameraData;
		cameraData.projectionType = Rendering::CameraData::ProjectionType::Perspective;
		cameraData.perspective = Rendering::CameraData::Perspective(glm::radians(50.0f), Application::Get().GetAspectRatio(), 0.1f, 1000.0f);
		
		Rendering::CameraHandle::SetDefaultCameraData(cameraData);

		SetUpEvents();

		auto camera = Application::Get().GetScene().GetEditorCameraEntity();
		camera.GetComponent<CCamera>().SetAsMainCamera();
		if (!camera.HasComponent<CNativeScript>())
			camera.AddComponent<CNativeScript>().Bind<CameraController>();
	}

	void EditorLayer::SetUpEvents() {
		Events::EventSystem::Get().AddListener(this, &EditorLayer::OnSceneChangeEvent);
	}

	void EditorLayer::OnUpdate()
	{
		if (isFirst) {

			std::string debugStr;
			Application::Get().GetScene().GetRegistry().view<entt::entity>().each([&](auto entityID) {
				std::stringstream ss;
				ss << "Entity ID: " << std::to_string((uint32_t)entityID) << " at: " << static_cast<void*>(&entityID) << "\n";
				debugStr += ss.str();
				Entity entity = { entityID, &Application::Get().GetScene() };
				if (!entity) return;
			});
			MADAM_INFO(debugStr);
			isFirst = false;
		}
		
		if (Application::Get().IsRuntimeFlag() && !Application::Get().IsPlay()) {
			editorScene = Application::Get().GetSceneRef();
			editorCameraId = editorScene->GetEditorCameraEntity().GetComponent<CUniqueIdentifier>().uuid;
			auto runtimeScene = editorScene->Copy();
			if (scriptEngine->Start(Project::Get().getProjectDirectory(), runtimeScene)) {
				Application::Get().SwitchScenes(runtimeScene);
				Application::Get().RuntimeStart();
				MADAM_INFO("C# runtime started");
			}
			else {
				editorScene.reset();
				editorCameraId.reset();
				MADAM_ERROR("C# runtime could not start");
			}
		}
		if (Application::Get().IsRuntimeStopFlag()) {
			scriptEngine->Stop();
			if (editorScene) Application::Get().SwitchScenes(editorScene);
			editorScene.reset();
			editorCameraId.reset();
			Application::Get().RuntimeStop();
			MADAM_INFO("Runtime stopped");
		}
		if (Application::Get().IsPlay())
			scriptEngine->Update(Time::GetDeltaTime());

		auto camera = Application::Get().GetScene().GetEditorCameraEntity();
		camera.GetComponent<CCamera>().SetAsMainCamera();
		if (!camera || !camera.HasComponent<CTransform>())
		{
			MADAM_ERROR("The active scene has no usable camera transform.");
			return;
		}
		Rendering::CameraHandle& handle = Rendering::CameraHandle::GetMain();
		const auto& transform = camera.GetComponent<CTransform>();
		handle.SetViewYXZ(transform.translation, transform.rotation);
		handle.SetProjection();
		//MADAM_INFO("EditorSurface Updated");
	}

	//Tag component could fix camera issue?
	void EditorLayer::OnSceneChangeEvent(SceneChangeEvent* e) {
		if (editorCameraId)
		{
			auto matchingCamera = Application::Get().GetScene().GetEntity(*editorCameraId);
			if (matchingCamera && matchingCamera.HasComponent<CCamera>())
				matchingCamera.GetComponent<CCamera>().SetAsMainCamera();
		}
		auto camera = Application::Get().GetScene().GetEditorCameraEntity();
		camera.GetComponent<CCamera>().SetAsMainCamera();
		if (camera && !camera.HasComponent<CNativeScript>())
			camera.AddComponent<CNativeScript>().Bind<CameraController>();
	}
}
