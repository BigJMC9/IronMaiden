#include "H_EditorInterface.hpp"

namespace Madam {

	class CameraController : public ScriptableEntity {
	public:
		float moveSpeed{ 3.f };
		float lookSpeed{ 1.5f };
		void Create() {

		}
		void Destroy() {

		}
		void Start() {

		}
		void Update() {
			if (GetComponent<CCamera>().cameraHandle->IsMain()) {
				GLFWwindow* window = Application::Get().getWindow().getGLFWwindow();

				glm::vec3 rotate{ 0 };
				if (Input::Get().IsKeyDown(KeyCode::UP)) rotate.x += 1.f;
				if (Input::Get().IsKeyDown(KeyCode::DOWN)) rotate.x -= 1.f;
				if (Input::Get().IsKeyDown(KeyCode::RIGHT)) rotate.y += 1.f;
				if (Input::Get().IsKeyDown(KeyCode::LEFT)) rotate.y -= 1.f;

				if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
					GetComponent<CTransform>().rotation += lookSpeed * Application::Get().getTime().GetDeltaTime() * glm::normalize(rotate);
				}

				GetComponent<CTransform>().rotation.x = glm::clamp(GetComponent<CTransform>().rotation.x, -1.5f, 1.5f);
				GetComponent<CTransform>().rotation.y = glm::mod(GetComponent<CTransform>().rotation.y, glm::two_pi<float>());

				float yaw = GetComponent<CTransform>().rotation.y;
				const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
				const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
				const glm::vec3 upDir{ 0.f, -1.f, 0.f };

				glm::vec3 moveDir{ 0.f };
				if (Input::Get().IsKeyDown(KeyCode::W)) moveDir += forwardDir;
				if (Input::Get().IsKeyDown(KeyCode::S)) moveDir -= forwardDir;
				if (Input::Get().IsKeyDown(KeyCode::A)) moveDir -= rightDir;
				if (Input::Get().IsKeyDown(KeyCode::D)) moveDir += rightDir;
				if (Input::Get().IsKeyDown(KeyCode::E)) moveDir += upDir;
				if (Input::Get().IsKeyDown(KeyCode::Q)) moveDir -= upDir;

				if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
					GetComponent<CTransform>().translation += moveSpeed * Application::Get().getTime().GetDeltaTime() * glm::normalize(moveDir);
				}
			}
		}
		void LateUpdate() {

		}
		void Render() {

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

	}
}