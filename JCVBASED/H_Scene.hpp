#pragma once


#include "H_JCVB_device.hpp"
#include "H_JCVB_model.hpp"
//#include "H_components.hpp"
//#include "H_Game_Object.hpp"
#include "H_JCVB_frame_info.hpp"
//#include "H_first_app.hpp"

//std
#include <memory>


namespace Madam {
	struct Material;

	class Entity;
	struct UUID;
	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity();
		Entity CreateEntity(entt::entity _entity);
		Entity CreateEntity(UUID uuid);
		Entity CreateGameObject();
		Entity LoadGameObject(std::shared_ptr<Model> model);
		Entity LoadGameObject(std::shared_ptr<Model> model, Material mat);

		void Start();
		void Update();
		void Render();

		entt::registry& Reg() { return registry; }

		Scene& scene() { return *this; }

		Scene(Scene&& other) noexcept : registry(std::move(other.registry)) {
			// Optionally, perform any additional move-related operations
		}

		Scene& operator=(Scene&& other) noexcept {
			if (this != &other) {
				registry = std::move(other.registry);
				// Optionally, perform any additional move-related operations
			}
			return *this;
		}

		//std::shared_ptr<entt::registry> Reg() { return std::make_shared<entt::registry>(registry); }

		//void Initialize();
		//void Start();
		//void Update();
		//void Render();

		//GameObject editorCamera;
	private:

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		//Device device;

		entt::registry registry;

		//KeyboardMovementController cameraController;
		//std::unordered_map<UUID, Entity> m_EntityMap;

		friend class Entity;
		friend class SceneSerializer;
	};
}