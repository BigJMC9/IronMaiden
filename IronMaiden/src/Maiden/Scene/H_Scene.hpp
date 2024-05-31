#pragma once

#include "maidenpch.hpp"
#include "../Rendering/Vulkan/H_VulkanDevice.hpp"
#include "H_Model.hpp"
#include "../Rendering/FrameInfo.hpp"
#include "../Rendering/H_Camera.hpp"
#include "../Events/H_EventSystem.h"

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
		void DestroyEntity(Entity entity);
		Entity LoadGameObject(Ref<Model> model);
		Entity LoadGameObject(Ref<Model> model, Material mat);

		void Start();
		void RunTimeStart();
		void Update();
		void Render();


		Ref<Rendering::CameraHandle> GetCurrentCamera() {
			return cameras[mainCameraIndex];
		}
		Ref<Scene> Copy();

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

		template<typename... Components, typename... Args>
		auto GetAllEntitiesWith(Args&&... args)
		{
			return registry.view<Components...>(std::forward<Args>(args)...);
		}

	private:
		std::vector<Ref<Rendering::CameraHandle>> cameras;
		int mainCameraIndex = 0;

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		entt::registry registry;

		friend class Entity;
		friend class SceneSerializer;
	};
}