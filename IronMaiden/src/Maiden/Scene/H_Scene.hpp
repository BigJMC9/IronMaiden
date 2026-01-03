#pragma once

#include "maidenpch.hpp"
#include "../Rendering/Vulkan/H_VulkanDevice.hpp"
#include "../Rendering/FrameInfo.hpp"
#include "../Rendering/H_Camera.hpp"
#include "../Events/H_EventSystem.h"
#include "../Asset/Asset.h"

#include <deque>

// When we add animation, we will have to compress the data when loaded into the GPU or it will be very memory intensive. see (pg 63)
// Look into frustum culling and Spacial subdivision culling (pg 47)
// Rank Each Camera by priority and render in that order. Put the editor camera at the top of the list.

namespace Madam
{

	class Entity;
	struct UUID;
	namespace Rendering
	{
		struct RenderScene;
	}

	// Needs special Asset Serialization and Deserialization. (Don't want to load scene when loading asset details)
	class MADAM_API Scene : public Asset
	{
	public:
		Scene();
		~Scene() override;

		Entity CreateErrorEntity();
		Entity CreateEntity();
		Entity CreateEntity(const std::string& name, bool is_hidden = false);
		Entity CreateEntity(entt::entity entity);
		Entity CreateEntity(UUID uuid);
		Entity CreateEntity(UUID uuid, const std::string& name);

		void DestroyEntity(Entity entity);

		void Start();
		void RunTimeStart();
		void Update();
		void Render();

		Scene& Get() { return *this; }
		Ref<Scene> Copy();

		static AssetType GetStaticType() { return AssetType::SCENE; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		entt::registry& GetRegistry() { return m_registry; }

		void BuildRenderScene(Rendering::RenderScene& out);

		glm::mat4 Scene::GetWorldTransform(UUID entity_uuid);
		glm::mat4 GetWorldTransform(Entity entity);

		Entity GetEntity(UUID uuid);

		void AddEntityRelationship(Entity parent, Entity child);
		void RemoveParentEntityRelationship(Entity child);

		Entity GetMainCameraEntity();

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_registry.view<Components...>();
		}

		template<typename... Components, typename... Args>
		auto GetAllEntitiesWith(Args&&... args)
		{
			return m_registry.view<Components...>(std::forward<Args>(args)...);
		}

		Scene(Scene&& other) noexcept : m_registry(std::move(other.m_registry))
		{
			RepopulateEntityMap();
		}

		Scene& operator=(Scene&& other) noexcept
		{
			if (this != &other)
			{
				m_registry = std::move(other.m_registry);
				RepopulateEntityMap();
			}
			return *this;
		}
	private:

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		template<typename T>
		void OnComponentRemoved(Entity entity, T& component);

		void RepopulateEntityMap();

		entt::registry m_registry;
		std::unordered_map<UUID, entt::entity> m_entity_map;

		friend class Entity;
		friend class SceneSerializer;
	};
}
