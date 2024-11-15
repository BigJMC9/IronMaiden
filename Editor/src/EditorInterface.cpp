#include "H_EditorInterface.hpp"

namespace Madam {

	class CameraController : public ScriptableEntity {
	public:
		float moveSpeed{ 3.f };
		float lookSpeed{ 1.5f };
		float scrollSensitivity = 30.0f;
		float mouseSensitivity = 0.5f;
		float speedStep = 2.0f;
		glm::vec2 mouseDelta{ 0.0f };
		glm::vec2 mouseScroll{ 0.0f };


		void Create() {
			Events::EventSystem::Get().AddListener(this, &CameraController::OnMouseMoveEvent);
			Events::EventSystem::Get().AddListener(this, &CameraController::OnMouseScrollEvent);
		}
		void Destroy() {
			Events::EventSystem::Get().RemoveListener(this, &CameraController::OnMouseMoveEvent);
			Events::EventSystem::Get().RemoveListener(this, &CameraController::OnMouseScrollEvent);
		}
		void Start() {
			
		}
		void Update() {
			if (GetComponent<CCamera>().cameraHandle->IsMain()) {
				GLFWwindow* window = Application::Get().getWindow().getGLFWwindow();
				glm::vec2 rotate = (mouseDelta * mouseSensitivity);
				rotate.x = -rotate.x;
				mouseDelta = { 0.0f, 0.0f };

				if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {

					glm::quat currentRotation = GetComponent<CTransform>().rotation;
					glm::quat deltaYaw = glm::angleAxis(rotate.y * lookSpeed * Application::Get().getTime().GetDeltaTime(), glm::vec3(0, -1, 0));

					currentRotation = deltaYaw * currentRotation;
					float divideSpeedBy = 1.0f;
					bool tryPitch = true;

					while (tryPitch)
					{
						glm::quat deltaPitch = glm::quat(glm::vec3(rotate.x, 0.f, 0.f) * (-lookSpeed / divideSpeedBy) * Application::Get().getTime().GetDeltaTime());

						glm::quat pitchRotation = currentRotation * deltaPitch;

						glm::vec3 pitchEuler = glm::eulerAngles(pitchRotation);

						float limit = glm::half_pi<float>();

						if (glm::abs(pitchEuler.z) > glm::radians(179.0f))
						{
							if (glm::abs(pitchEuler.x) > limit)
							{
								currentRotation = pitchRotation;
								tryPitch = false;
							}
							else if (divideSpeedBy > 1000.0f)
							{
								tryPitch = false;
							}
							else
							{
								divideSpeedBy *= 10.0f;
							}
						}
						else
						{
							if (glm::abs(pitchEuler.x) < limit)
							{
								currentRotation = pitchRotation;
								tryPitch = false;
							}
							else if (divideSpeedBy > 1000.0f)
							{
								tryPitch = false;
							}
							else
							{
								divideSpeedBy *= 10.0f;
							}
						}
					}
					GetComponent<CTransform>().rotation = glm::normalize(currentRotation);
				}

				glm::quat rotation = GetComponent<CTransform>().rotation;
				glm::vec3 forwardDir = rotation * glm::vec3(0.f, 0.f, 1.f);
				glm::vec3 rightDir = rotation * glm::vec3(1.f, 0.f, 0.f);
				glm::vec3 upDir = rotation * glm::vec3(0.f, -1.f, 0.f);

				glm::vec3 moveDir{ 0.f };
				if (Input::Get().IsMouseButtonPress(MouseCode::RIGHTMOUSEBUTTON))
				{
					if (Input::Get().IsKeyPress(KeyCode::W)) moveDir += forwardDir;
					if (Input::Get().IsKeyPress(KeyCode::S)) moveDir -= forwardDir;
					if (Input::Get().IsKeyPress(KeyCode::A)) moveDir -= rightDir;
					if (Input::Get().IsKeyPress(KeyCode::D)) moveDir += rightDir;
					if (Input::Get().IsKeyPress(KeyCode::E)) moveDir += upDir;
					if (Input::Get().IsKeyPress(KeyCode::Q)) moveDir -= upDir;

					moveSpeed += mouseScroll.y * speedStep;
					if (moveSpeed < 0)
					{
						moveSpeed = 0.1f;
					}

					if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
					{
						GetComponent<CTransform>().translation += moveSpeed * Application::Get().getTime().GetDeltaTime() * glm::normalize(moveDir);
					}
				}
				else
				{
					glm::vec3 scrollDir = mouseScroll.y * forwardDir;

					if (glm::dot(scrollDir, scrollDir) > std::numeric_limits<float>::epsilon())
					{
						GetComponent<CTransform>().translation += scrollSensitivity * Application::Get().getTime().GetDeltaTime() * glm::normalize(scrollDir);
					}
				}
				mouseScroll = glm::vec2{ 0.0f };
			}
		}
		void LateUpdate() {

		}
		void Render() {

		}

		void OnMouseMoveEvent(MouseMoveEvent* e)
		{
			if (Input::Get().IsMouseButtonPress(MouseCode::RIGHTMOUSEBUTTON))
			{
				mouseDelta = e->mouseDelta;
			}
		}

		void OnMouseScrollEvent(MouseScrollEvent* e)
		{
			mouseScroll = glm::vec2((float)e->x, (float)e->y);
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
		cameraData.perspective = Rendering::CameraData::Perspective(glm::radians(50.0f), Application::Get().getAspectRatio(), 0.1f, 1000.0f);
		
		Rendering::CameraHandle::SetDefaultCameraData(cameraData);

		SetUpEvents();

		Rendering::CameraHandle::GetMain();
		Application::Get().getScene().GetMainCameraEntity().AddComponent<CNativeScript>().Bind<CameraController>();
		Application::Get().getScene().GetMainCameraEntity().GetComponent<CTransform>().translation = glm::vec3(0.0f, -2.0f, -2.0f);
	}

	void EditorLayer::SetUpEvents() {
		Events::EventSystem::Get().AddListener(this, &EditorLayer::OnSceneChangeEvent);
	}

	void EditorLayer::OnUpdate()
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
		}
		
		std::string scriptPath = Application::Get().CreateScript();
		if (scriptPath != "") {
			MADAM_INFO("Script Creating");
			scriptEngine->OnCreateScript(scriptPath);
		}

		//Fix in future please, have scene load after dll loaded
		if (Application::Get().isRuntimeFlag()) {
			MADAM_INFO("Starting Game");
			std::shared_ptr<Scene> runtimeScene = Application::Get().getScene().Copy();
			Application::Get().SwitchScenes(runtimeScene);
			Entity& gameManager = Application::Get().getScene().CreateEntity();
			if (scriptEngine->Runtime(gameManager)) {
				Application::Get().RuntimeStart();
			}
			else {
				MADAM_INFO("Runtime failed");
				//viewerObject.GetComponent<Camera>().SetAsMainCamera();
				//MADAM_INFO("CameraHandle use count: {0}", viewerObject.GetComponent<Camera>().cameraHandle.use_count());
			}
		}
		if (Application::Get().isRuntimeStopFlag()) {
			MADAM_INFO("Runtime stopped");
			//viewerObject.GetComponent<Camera>().SetAsMainCamera();
			//MADAM_INFO("CameraHandle use count: {0}", viewerObject.GetComponent<Camera>().cameraHandle.use_count());
		}
		if (Application::Get().isUpdate()) {
			MADAM_INFO("Updating scripts");
			scriptEngine->Update();
		}
		if (Application::Get().getScripts()) {
			MADAM_INFO("Showing scripts");
			scriptEngine->ShowScripts();
		}

		Rendering::CameraHandle& handle = Rendering::CameraHandle::GetMain();
		handle.SetViewYXZ(Application::Get().getScene().GetMainCameraEntity().GetComponent<CTransform>().translation, Application::Get().getScene().GetMainCameraEntity().GetComponent<CTransform>().rotation);
		float aspect = Application::Get().getAspectRatio();
		handle.SetProjection();
		//MADAM_INFO("EditorSurface Updated");
	}

	//Tag component could fix camera issue?
	void EditorLayer::OnSceneChangeEvent(SceneChangeEvent* e) {
		Rendering::CameraHandle::GetMain();
		Application::Get().getScene().GetMainCameraEntity().AddComponent<CNativeScript>().Bind<CameraController>();
	}
}