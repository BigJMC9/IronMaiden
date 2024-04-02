#pragma once

#include "maidenpch.hpp"
#include "../Core/H_Device.hpp"
#include "H_Model.hpp"
#include "../Rendering/FrameInfo.hpp"
#include "../Rendering/H_Camera.hpp"

//When we add animation, we will have to compress the data when loaded into the GPU or it will be very memory intensive. see (pg 63)

//Look into frustum culling and Spacial subdivision culling (pg 47)
namespace Madam {
	struct Material;

	class Entity;
	struct UUID;
	class MADAM_API Scene
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
	private:
		std::vector<std::shared_ptr<Rendering::CameraHandle>> cameras;

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		entt::registry registry;

		friend class Entity;
		friend class SceneSerializer;
	};
}