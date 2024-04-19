#pragma once

#include "maidenpch.hpp"
#include "../Core/H_Device.hpp"
#include "H_Model.hpp"
#include "../Rendering/FrameInfo.hpp"
#include "../Rendering/H_Camera.hpp"

//When we add animation, we will have to compress the data when loaded into the GPU or it will be very memory intensive. see (pg 63)

//Look into frustum culling and Spacial subdivision culling (pg 47)

//Rank Each Camera by priority and render in that order. Put the editor camera at the top of the list.
namespace Madam {
	struct Material;

	class Entity;
	class UUID;
	class MADAM_API Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity();
		Entity CreateEntity(entt::entity _entity);
		Entity CreateEntity(UUID uuid);
		Entity CreateEntity(UUID uuid, const std::string& name);
		Entity LoadGameObject(std::shared_ptr<Model> model);
		Entity LoadGameObject(std::shared_ptr<Model> model, Material mat);

		void Start();
		void RunTimeStart();
		void Update();
		void Render();


		std::shared_ptr<Rendering::CameraHandle> GetCurrentCamera() {
			return cameras[mainCameraIndex];
		}
		std::shared_ptr<Scene> Copy();

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

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return registry.view<Components...>();
		}

	private:
		std::vector<std::shared_ptr<Rendering::CameraHandle>> cameras;
		int mainCameraIndex = 0;

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		entt::registry registry;

		friend class Entity;
		friend class SceneSerializer;
	};
}