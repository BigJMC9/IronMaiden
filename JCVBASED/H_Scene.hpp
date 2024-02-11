#pragma once


#include "H_JCVB_device.hpp"
#include "H_JCVB_model.hpp"
//#include "H_components.hpp"
//#include "H_Game_Object.hpp"
#include "H_JCVB_frame_info.hpp"

//std
#include <memory>


namespace Digestion {
	struct Material;
	//class GameObject;
	class Entity;

	class Scene
	{
	public:
		Scene(Device& _device) : device{ _device } {}
		~Scene();

		Entity CreateGameObject();
		Entity LoadGameObject(std::shared_ptr<Model> model);
		Entity LoadGameObject(std::shared_ptr<Model> model, std::shared_ptr<Material> mat);

		void Start();
		void Update();
		void Render();
		void LoadScene();
		//void SaveScene();
		void LoadScene(std::string scenePath);

		entt::registry& Reg() { return registry; }

		Scene& scene() { return *this; }

		//std::shared_ptr<entt::registry> Reg() { return std::make_shared<entt::registry>(registry); }

		//void Initialize();
		//void Start();
		//void Update();
		//void Render();

		//GameObject editorCamera;
	private:

		//void LoadTransform(YAML::Node node, Entity gameObject);
		//void LoadPointLight(YAML::Node node, Entity gameObject);
		//void LoadMeshFilter(YAML::Node node, Entity gameObject);
		//void LoadMeshRenderer(YAML::Node node, Entity gameObjectde);
		//Shader LoadShader(YAML::Node node);
		//void LoadMaterial(YAML::Node node, Entity gameObject);
		//void LoadCamera(YAML::Node node, Entity gameObject);

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		Device& device;

		entt::registry registry;

		//KeyboardMovementController cameraController;

		friend class Entity;
		friend class SceneSerializer;
	};
}