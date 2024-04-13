#include "H_Editor.hpp"

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
			if (GetComponent<Camera>().cameraHandle->isMain()) {
				GLFWwindow* window = Application::Get().getWindow().getGLFWwindow();

				glm::vec3 rotate{ 0 };
				if (Input::Get().IsKeyDown(KeyCode::UP)) rotate.x += 1.f;
				if (Input::Get().IsKeyDown(KeyCode::DOWN)) rotate.x -= 1.f;
				if (Input::Get().IsKeyDown(KeyCode::RIGHT)) rotate.y += 1.f;
				if (Input::Get().IsKeyDown(KeyCode::LEFT)) rotate.y -= 1.f;

				if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
					GetComponent<Transform>().rotation += lookSpeed * Application::Get().getTime().GetDeltaTime() * glm::normalize(rotate);
				}

				GetComponent<Transform>().rotation.x = glm::clamp(GetComponent<Transform>().rotation.x, -1.5f, 1.5f);
				GetComponent<Transform>().rotation.y = glm::mod(GetComponent<Transform>().rotation.y, glm::two_pi<float>());

				float yaw = GetComponent<Transform>().rotation.y;
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
					GetComponent<Transform>().translation += moveSpeed * Application::Get().getTime().GetDeltaTime() * glm::normalize(moveDir);
				}
			}
		}
		void LateUpdate() {

		}
		void Render() {

		}
	};

	EditorSurface::EditorSurface() : Surface("Editor") 
	{

	}

	void EditorSurface::OnAttach()
	{
		MADAM_INFO("Editor Server Attached");
		scriptEngine = std::make_shared<Scripting::ScriptEngine>();
		Rendering::CameraData cameraData;
		cameraData.projectionType = Rendering::CameraData::ProjectionType::Perspective;
		cameraData.perspective = Rendering::CameraData::Perspective(glm::radians(50.0f), Application::Get().getAspectRatio(), 0.1f, 1000.0f);
		Entity& obj = Application::Get().getScene().CreateEntity();
		obj.GetComponent<Object>().name = "Editor Camera";
		obj.GetComponent<Transform>().translation.z = -2.5f;
		obj.AddComponent<Camera>(cameraData);
		viewerObject = Entity(obj);
		viewerObject.GetComponent<Camera>().cameraHandle->setViewDirection(glm::vec3(0.f, 2.0f, 0.f), glm::vec3(0.f, 0.f, 0.f));
		viewerObject.GetComponent<Camera>().cameraHandle->setMain();
		if (viewerObject.GetComponent<Camera>().cameraHandle->isMain()) {
			MADAM_INFO("Camera is main camera");
		}
		else {
			MADAM_INFO("Camera is not main camera");
		}
		entt::entity& entityID = obj.GetHandleAsRef();
		MADAM_CORE_INFO("Entity ID: {0} at: {1}", (uint32_t)entityID, obj.GetHandleMemoryLocation());

		
		//Pain（πーπ）
		viewerObject.AddComponent<NativeScriptComponent>().Bind<CameraController>();

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
			entt::entity& entityID = viewerObject.GetHandleAsRef();

			//Madam::Rendering::CameraHandle& cameraHandle = viewerObject->GetComponent<Camera>().cameraHandle->get();

			if (viewerObject.GetComponent<Camera>().cameraHandle->isMain()) {
				MADAM_INFO("Camera is main camera");
			}
			else {
				MADAM_INFO("Camera is not main camera");
			}

			if (Madam::Rendering::CameraHandle::getMain().isMain()) {
				MADAM_INFO("Is main function is working");
			}
		}
		
		std::string scriptPath = Application::Get().CreateScript();
		if (scriptPath != "") {
			MADAM_INFO("Script Creating");
			scriptEngine->OnCreateScript(scriptPath);
		}
		/*if (Application::Get().isScan()) {
			MADAM_CORE_INFO("Scanning for scripts");
			scriptEngine->RescanScripts();
		}
		if (Application::Get().isCompile()) {
			MADAM_CORE_INFO("Compiling scripts");
			scriptEngine->Compile();
		}*/

		//Fix in future plz, have scene load after dll loaded
		if (Application::Get().isRuntimeFlag()) {
			MADAM_INFO("Starting Game");
			std::shared_ptr<Scene> runtimeScene = Application::Get().getScene().Copy();
			Application::Get().PrimeReserve(runtimeScene);
			Application::Get().SwitchScenes();
			Entity& testEnt = Application::Get().getScene().CreateEntity();
			if (scriptEngine->Runtime(testEnt)) {
				Application::Get().RuntimeStart();
				
			}
			else {
				Application::Get().SwitchScenes(true);
			}
		}
		if (Application::Get().isUpdate()) {
			MADAM_INFO("Updating scripts");
			scriptEngine->Update();
		}
		if (Application::Get().getScripts()) {
			MADAM_INFO("Showing scripts");
			scriptEngine->ShowScripts();
		}

		Entity& obj = viewerObject;
		Camera& camera = viewerObject.GetComponent<Camera>();
		camera.cameraHandle->setViewYXZ(viewerObject.GetComponent<Transform>().translation, viewerObject.GetComponent<Transform>().rotation);
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
					viewerObject = Entity(entity);
					viewerObject.AddComponent<NativeScriptComponent>().Bind<CameraController>();
				}
				else {
					MADAM_INFO("Camera is not main camera");
				}
			}
		});

	}
}