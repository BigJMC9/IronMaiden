#pragma once

#include "maidenpch.hpp"
#include "../Rendering/Vulkan/H_VulkanDevice.hpp"
#include "../Rendering/FrameInfo.hpp"
#include "../Rendering/H_Camera.hpp"
#include "../Events/H_EventSystem.h"
#include "../Asset/Asset.h"

#include <deque>

//When we add animation, we will have to compress the data when loaded into the GPU or it will be very memory intensive. see (pg 63)

//Look into frustum culling and Spacial subdivision culling (pg 47)

//Rank Each Camera by priority and render in that order. Put the editor camera at the top of the list.
namespace Madam {
	struct MaterialComponent;

	class Entity;
	struct UUID;

	//Needs special Asset Serialization and Deserialization. (Don't want to load scene when loading asset details)
	class MADAM_API Scene : public Asset
	{
	public:
		Scene();
		~Scene() override;


		Entity CreateEntity();
		Entity CreateEntity(const std::string& name);
		Entity CreateEntity(entt::entity _entity);
		Entity CreateEntity(UUID uuid);
		Entity CreateEntity(UUID uuid, const std::string& name);

		void DestroyEntity(Entity entity);

		void Start();
		void RunTimeStart();
		void Update();
		void Render();

		Ref<Scene> Copy();

		entt::registry& Reg() { return registry; }

		Scene& scene() { return *this; }

		Scene(Scene&& other) noexcept : registry(std::move(other.registry)) {
			RepopulateEntityMap();
		}

		Scene& operator=(Scene&& other) noexcept {
			if (this != &other) {
				registry = std::move(other.registry);
				RepopulateEntityMap();
			}
			return *this;
		}

		static AssetType GetStaticType() { return AssetType::SCENE; }
		AssetType GetAssetType() const override { return GetStaticType(); }

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

		glm::mat4 Scene::GetWorldTransform(UUID entityUUID);
		glm::mat4 GetWorldTransform(Entity entity);

		Entity GetEntity(UUID uuid);

		void AddEntityRelationship(Entity Parent, Entity Child);

		Entity GetMainCameraEntity();

	private:

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		template<typename T>
		void OnComponentRemoved(Entity entity, T& component);

		void RepopulateEntityMap();

		entt::registry registry;

		std::unordered_map<UUID, Entity> entityMap;

		friend class Entity;
		friend class SceneSerializer;
	};
}