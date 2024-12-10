#include "maidenpch.hpp"
#include "../Core/H_Application.hpp"
#include "../Rendering/H_Mesh.h"
#include "H_Scene.hpp"
#include "../Events/H_Input.hpp"
#include "Components.hpp"
#include "../Rendering/H_Renderer.hpp"

namespace Madam {

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<CUniqueIdentifier>(srcEntity).uuid);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	Scene::Scene() {

	}

	Scene::~Scene()
	{
		registry.clear();
		entityMap.clear();
	}

	Ref<Scene> Scene::Copy()
	{
		Ref<Scene> newScene = CreateRef<Scene>();
		std::unordered_map<UUID, entt::entity> enttMap;

		auto IdView = registry.view<CUniqueIdentifier>();
		for (entt::entity entity : IdView) {
			UUID& uuid = registry.get<CUniqueIdentifier>(entity).uuid;
			const auto& name = registry.get<CMetadata>(entity).name;
			Entity newEntity = newScene->CreateEntity(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		CopyComponent(AllComponents{}, newScene->Reg(), registry, enttMap);

		return newScene;
	}

	Entity Scene::CreateErrorEntity()
	{
		Entity entity = { registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>();
		entity.AddComponent<CMetadata>();
		CMetadata& entityMetadata = entity.GetComponent<CMetadata>();
		entityMetadata.name = "ERROR";
		entityMetadata.isErrorEntity = true;
		entityMap[entity.GetComponent<CUniqueIdentifier>().uuid] = entity;
		return entity;
	}

	Entity Scene::CreateEntity() {
		Entity entity = { registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>();
		entity.AddComponent<CMetadata>();
		entity.AddComponent<CTransform>();
		entity.AddComponent<CRelationship>();
		entityMap[entity.GetComponent<CUniqueIdentifier>().uuid] = entity;
		return entity;
	}

	Entity Scene::CreateEntity(const std::string& name, bool isHidden) {
		Entity entity = { registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>();
		entity.AddComponent<CMetadata>(name, isHidden);
		entity.AddComponent<CTransform>();
		entity.AddComponent<CRelationship>();
		entityMap[entity.GetComponent<CUniqueIdentifier>().uuid] = entity;
		return entity;
	}

	Entity Scene::CreateEntity(entt::entity _entity) {
		Entity entity = { registry.create(_entity), this };
		entity.AddComponent<CUniqueIdentifier>();
		entity.AddComponent<CMetadata>();
		entity.AddComponent<CTransform>();
		entity.AddComponent<CRelationship>();
		entityMap[entity.GetComponent<CUniqueIdentifier>().uuid] = entity;
		return entity;
	}

	Entity Scene::CreateEntity(UUID uuid) {
		Entity entity = { registry.create(), this};
		entity.AddComponent<CUniqueIdentifier>(uuid);
		entity.AddComponent<CMetadata>();
		entity.AddComponent<CTransform>();
		entity.AddComponent<CRelationship>();
		entityMap[entity.GetComponent<CUniqueIdentifier>().uuid] = entity;
		return entity;
	}

	Entity Scene::CreateEntity(UUID uuid, const std::string& name) {
		Entity entity = { registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>(uuid);
		entity.AddComponent<CMetadata>(name);
		entity.AddComponent<CTransform>();
		entity.AddComponent<CRelationship>();
		entityMap[entity.GetComponent<CUniqueIdentifier>().uuid] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity) {
		for each (UUID child in entity.GetComponent<CRelationship>().children)
		{
			DestroyEntity(entityMap[child]);
		}
		UUID parent = entity.GetComponent<CRelationship>().parent;
		UUID uuid = entity.GetComponent<CUniqueIdentifier>().uuid;
		if (parent != null)
		{
			Entity parentEntity = entityMap[parent];
			
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
		entityMap.erase(uuid);
		registry.destroy(entity);
	}

	glm::mat4 Scene::GetWorldTransform(UUID entityUUID)
	{
		Entity entity = entityMap[entityUUID];
		if (entity == null)
		{
			MADAM_CORE_ERROR("Attempting to get an Entity that does not exist");
			return glm::mat4(glm::vec4(0), glm::vec4(0), glm::vec4(0), glm::vec4(0));
		}

		glm::mat4 transform = entity.GetComponent<CTransform>().transform();
		UUID parent = entity.GetComponent<CRelationship>().parent;
		if (parent != null)
		{
			transform = GetWorldTransform(entityMap[parent]) * transform;
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

		glm::mat4 transform = entity.GetComponent<CTransform>().transform();
		UUID parent = entity.GetComponent<CRelationship>().parent;
		if (parent != null)
		{
			transform = GetWorldTransform(entityMap[parent]) * transform;
		}
		return transform;
	}

	void Scene::Start() {

	}

	void Scene::RunTimeStart() {

	}

	void Scene::Update() {
		{
			registry.view <CNativeScript>().each([=](auto entity, auto& nsc) {
				//Move to on scene play
				if (!nsc.Instance) {
					nsc.Instance = nsc.InstantiateScript();
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

	void Scene::Render() {
		registry.view <CNativeScript>().each([=](auto entity, auto& nsc) {
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
			Entity oldParent = entityMap[oldParentUUID];
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
		entityMap.clear();
		auto entities = registry.view<CUniqueIdentifier>();
		for (auto handle : entities)
		{
			Entity entity = Entity(handle, this);
			entityMap[entities.get<CUniqueIdentifier>(handle).uuid] = entity;
		}
	}

	Entity Scene::GetEntity(UUID uuid)
	{
		return entityMap[uuid];
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