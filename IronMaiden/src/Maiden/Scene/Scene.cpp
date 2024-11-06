#include "maidenpch.hpp"
#include "../Core/H_Application.hpp"
#include "../Rendering/H_Mesh.h"
#include "H_Scene.hpp"
#include "../Events/H_Input.hpp"
#include "Components.hpp"
#include "../Rendering/H_Renderer.hpp"

//namespace fs = std::filesystem;

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
	}

	Ref<Scene> Scene::Copy()
	{
		Ref<Scene> newScene = CreateRef<Scene>();
		std::unordered_map<UUID, entt::entity> enttMap;
		//Copy all entities
		auto IdView = registry.view<CUniqueIdentifier>();
		for (entt::entity entity : IdView) {
			UUID& uuid = registry.get<CUniqueIdentifier>(entity).uuid;
			const auto& name = registry.get<CMetadata>(entity).name;
			Entity newEntity = newScene->CreateEntity(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}



		//Copy all components
		CopyComponent(AllComponents{}, newScene->Reg(), registry, enttMap);

		return newScene;
	}

	Entity Scene::CreateEntity() {
		Entity entity = { registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>();
		entity.AddComponent<CMetadata>();
		entity.AddComponent<CTransform>();
		return entity;
	}

	Entity Scene::CreateEntity(const std::string& name) {
		Entity entity = { registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>();
		entity.AddComponent<CMetadata>(name);
		entity.AddComponent<CTransform>();
		return entity;
	}

	//Dangerous?!? Maybe remove?
	Entity Scene::CreateEntity(entt::entity _entity) {
		Entity entity = { registry.create(_entity), this };
		entity.AddComponent<CUniqueIdentifier>();
		entity.AddComponent<CMetadata>();
		entity.AddComponent<CTransform>();
		return entity;
	}

	Entity Scene::CreateEntity(UUID uuid) {
		Entity entity = { registry.create(), this};
		entity.AddComponent<CUniqueIdentifier>(uuid);
		entity.AddComponent<CMetadata>();
		entity.AddComponent<CTransform>();
		return entity;
	}

	Entity Scene::CreateEntity(UUID uuid, const std::string& name) {
		Entity entity = { registry.create(), this };
		entity.AddComponent<CUniqueIdentifier>(uuid);
		entity.AddComponent<CMetadata>(name);
		entity.AddComponent<CTransform>();
		return entity;
	}

	void Scene::DestroyEntity(Entity entity) {
		registry.destroy(entity);
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