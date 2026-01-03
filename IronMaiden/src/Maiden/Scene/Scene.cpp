#include "maidenpch.hpp"
#include "../Core/H_Application.hpp"
#include "../Rendering/H_Mesh.h"
#include "H_Scene.hpp"
#include "../Events/H_Input.hpp"
#include "Components.hpp"
#include "../Rendering/H_Renderer.hpp"
#include "../Rendering/H_RenderScene.hpp"

namespace Madam
{

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& entt_map)
	{
		([&]()
		{
			auto view = src.view<Component>();
			for (auto src_entity : view)
			{
				entt::entity dst_entity = entt_map.at(src.get<CUniqueIdentifier>(src_entity).uuid);

				auto& src_component = src.get<Component>(src_entity);
				dst.emplace_or_replace<Component>(dst_entity, src_component);
			}
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{
		m_registry.clear();
		m_entity_map.clear();
	}

	Ref<Scene> Scene::Copy()
	{
		Ref<Scene> new_scene = CreateRef<Scene>();
		std::unordered_map<UUID, entt::entity> entt_map;

		auto id_view = m_registry.view<CUniqueIdentifier>();
		for (entt::entity entity : id_view)
		{
			UUID& uuid = m_registry.get<CUniqueIdentifier>(entity).uuid;
			const auto& name = m_registry.get<CMetadata>(entity).name;
			Entity new_entity = new_scene->CreateEntity(uuid, name);
			entt_map[uuid] = (entt::entity)new_entity;
		}

		CopyComponent(AllComponents{}, new_scene->GetRegistry(), m_registry, entt_map);

		return new_scene;
	}

	void Scene::BuildRenderScene(Rendering::RenderScene& out)
	{
		out.Clear();
		out.registry = &m_registry;

		auto meshView = m_registry.view<CTransform, CMeshRenderer>();
		out.meshEntities.reserve(meshView.size_hint());
		for (auto entity : meshView)
		{
			out.meshEntities.push_back(entity);
		}

		auto pointLightView = m_registry.view<CTransform, CPointLight>();
		out.pointLightEntities.reserve(pointLightView.size_hint());
		for (auto entity : pointLightView)
		{
			out.pointLightEntities.push_back(entity);
		}
	}

	Entity Scene::CreateErrorEntity()
	{
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>();
		entity.AddComponent<CMetadata>();
		CMetadata& entityMetadata = entity.GetComponent<CMetadata>();
		entityMetadata.name = "ERROR";
		entityMetadata.is_error_entity = true;
		m_entity_map[entity.GetComponent<CUniqueIdentifier>().uuid] = entity.GetHandle();
		return entity;
	}

	Entity Scene::CreateEntity()
	{
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>();
		entity.AddComponent<CMetadata>();
		entity.AddComponent<CTransform>();
		entity.AddComponent<CRelationship>();
		m_entity_map[entity.GetComponent<CUniqueIdentifier>().uuid] = entity.GetHandle();
		return entity;
	}

	Entity Scene::CreateEntity(const std::string& name, bool is_hidden)
	{
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>();
		entity.AddComponent<CMetadata>(name, is_hidden);
		entity.AddComponent<CTransform>();
		entity.AddComponent<CRelationship>();
		m_entity_map[entity.GetComponent<CUniqueIdentifier>().uuid] = entity.GetHandle();
		return entity;
	}

	Entity Scene::CreateEntity(entt::entity entt_entity)
	{
		Entity entity = { m_registry.create(entt_entity), this };
		entity.AddComponent<CUniqueIdentifier>();
		entity.AddComponent<CMetadata>();
		entity.AddComponent<CTransform>();
		entity.AddComponent<CRelationship>();
		m_entity_map[entity.GetComponent<CUniqueIdentifier>().uuid] = entity.GetHandle();
		return entity;
	}

	Entity Scene::CreateEntity(UUID uuid)
	{
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>(uuid);
		entity.AddComponent<CMetadata>();
		entity.AddComponent<CTransform>();
		entity.AddComponent<CRelationship>();
		m_entity_map[entity.GetComponent<CUniqueIdentifier>().uuid] = entity.GetHandle();
		return entity;
	}

	Entity Scene::CreateEntity(UUID uuid, const std::string& name)
	{
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>(uuid);
		entity.AddComponent<CMetadata>(name);
		entity.AddComponent<CTransform>();
		entity.AddComponent<CRelationship>();
		m_entity_map[entity.GetComponent<CUniqueIdentifier>().uuid] = entity.GetHandle();
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		for each (UUID child in entity.GetComponent<CRelationship>().children)
		{
			Entity childEntity = GetEntity(child);
			if (childEntity != null)
			{
				DestroyEntity(childEntity);
			}
		}
		UUID parent = entity.GetComponent<CRelationship>().parent;
		UUID uuid = entity.GetComponent<CUniqueIdentifier>().uuid;
		if (parent != null)
		{
			Entity parentEntity = GetEntity(parent);

			std::vector<UUID> children = parentEntity.GetComponent<CRelationship>().children;

			for (size_t i = 0; i < children.size(); i++)
			{
				if (children[i] == uuid)
				{
					children.erase(children.begin() + i);
					break;
				}
			}

			parentEntity.GetComponent<CRelationship>().children = children;
		}
		m_entity_map.erase(uuid);
		m_registry.destroy(entity);
	}

	glm::mat4 Scene::GetWorldTransform(UUID entityUUID)
	{
		Entity entity = GetEntity(entityUUID);
		if (entity == null)
		{
			MADAM_CORE_ERROR("Attempting to get an Entity that does not exist");
			return glm::mat4(glm::vec4(0), glm::vec4(0), glm::vec4(0), glm::vec4(0));
		}

		glm::mat4 transform = entity.GetComponent<CTransform>().TransformMatrix();
		UUID parent = entity.GetComponent<CRelationship>().parent;
		if (parent != null)
		{
			Entity parentEntity = GetEntity(parent);
			if (parentEntity != null)
			{
				transform = GetWorldTransform(parentEntity) * transform;
			}
		}
		return transform;
	}

	glm::mat4 Scene::GetWorldTransform(Entity entity)
	{
		if (entity == null)
		{
			MADAM_CORE_ERROR("Attempting to get an Entity that does not exist");
			return glm::mat4(glm::vec4(0), glm::vec4(0), glm::vec4(0), glm::vec4(0));
		}

		glm::mat4 transform = entity.GetComponent<CTransform>().TransformMatrix();
		UUID parent = entity.GetComponent<CRelationship>().parent;
		if (parent != null)
		{
			Entity parentEntity = GetEntity(parent);
			if (parentEntity != null)
			{
				transform = GetWorldTransform(parentEntity) * transform;
			}
		}
		return transform;
	}

	void Scene::Start()
	{

	}

	void Scene::RunTimeStart()
	{

	}

	void Scene::Update()
	{
		{
			m_registry.view <CNativeScript>().each([=](auto entity, auto& nsc) {
				//Move to on scene play
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.instantiate_script();
					nsc.Instance->entity = Entity{ entity, this };
					nsc.onCreate(nsc.Instance);
					nsc.onStart(nsc.Instance);
					//nsc.instance->entity = Entity{ entity, this };
					//nsc.instance->onCreate(nsc.instance);
					//nsc.instance->onStart(nsc.instance);
				}
				nsc.onUpdate(nsc.Instance);
				nsc.onLateUpdate(nsc.Instance);
				});

			//SetView Proj
		}
	}

	void Scene::Render()
	{
		m_registry.view <CNativeScript>().each([=](auto entity, auto& nsc) 
		{
			nsc.onRender(nsc.Instance);
		});
	}

	void Scene::AddEntityRelationship(Entity parent, Entity child)
	{
		if (parent.GetComponent<CUniqueIdentifier>().uuid == child.GetComponent<CUniqueIdentifier>().uuid)
		{
			MADAM_ERROR("Cannot add a entity relationship in which the parent and child are the same entity");
			return;
		}

		RemoveParentEntityRelationship(child);
		parent.GetComponent<CRelationship>().children.push_back(child.GetComponent<CUniqueIdentifier>().uuid);
		child.GetComponent<CRelationship>().parent = parent.GetComponent<CUniqueIdentifier>().uuid;
	}

	void Scene::RemoveParentEntityRelationship(Entity child)
	{
		if (child.GetComponent<CRelationship>().parent != null)
		{
			UUID oldParentUUID = child.GetComponent<CRelationship>().parent;
			Entity oldParent = GetEntity(oldParentUUID);
			if (oldParent != null)
			{
				std::vector<UUID> children = oldParent.GetComponent<CRelationship>().children;
				for (size_t i = 0; i < children.size(); i++)
				{
					if (children[i] == child.GetComponent<CUniqueIdentifier>().uuid)
					{
						children.erase(children.begin() + i);
						break;
					}
				}
				oldParent.GetComponent<CRelationship>().children = children;
			}
			else
			{
				MADAM_CORE_INFO("Old Parrent no longer exists");
			}

			child.GetComponent<CRelationship>().parent = UUID("");
		}
	}

	void Scene::RepopulateEntityMap()
	{
		m_entity_map.clear();
		auto entities = m_registry.view<CUniqueIdentifier>();
		for (auto handle : entities)
		{
			m_entity_map[entities.get<CUniqueIdentifier>(handle).uuid] = handle;
		}
	}

	Entity Scene::GetEntity(UUID uuid)
	{
		auto it = m_entity_map.find(uuid);
		if (it == m_entity_map.end() || it->second == entt::null)
		{
			return Entity();
		}
		return Entity(it->second, this);
	}

	Entity Scene::GetMainCameraEntity()
	{
		for (size_t i = 0; i <= 1; i++)
		{
			auto entities = GetAllEntitiesWith<CCamera>();
			for (auto entity : entities)
			{
				if (entities.get<CCamera>(entity).cameraHandle->IsMain())
				{
					return Entity(entity, this);
				}
			}
			Rendering::CameraHandle::GetMain();
		}
		return Entity();
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template<>
	void MADAM_API Scene::OnComponentAdded<CUniqueIdentifier>(Entity entity, CUniqueIdentifier& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<CMetadata>(Entity entity, CMetadata& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<CTransform>(Entity entity, CTransform& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<CMeshRenderer>(Entity entity, CMeshRenderer& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<CPointLight>(Entity entity, CPointLight& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<CMaterial>(Entity entity, CMaterial& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<CNativeScript>(Entity entity, CNativeScript& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<CCamera>(Entity entity, CCamera& component)
	{

	}

	//DAMN YOU!!! YOU STUPID CAMERA SYSTEM!!!!
	template<typename T>
	void Scene::OnComponentRemoved(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}
}
