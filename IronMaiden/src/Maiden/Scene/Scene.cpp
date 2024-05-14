#include "maidenpch.hpp"
#include "../Core/H_Application.hpp"
#include "H_Scene.hpp"
#include "../Events/H_Input.hpp"
#include "H_Entity.hpp"
#include "Components.hpp"
#include "../Rendering/H_Renderer.hpp"

//namespace fs = std::filesystem;

namespace Madam {

	Scene::Scene() {
	}

	Scene::~Scene()
	{
		registry.clear();
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<UniqueIdentifier>(srcEntity).uuid);	

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

	Ref<Scene> Scene::Copy()
	{
		Ref<Scene> newScene = std::make_shared<Scene>();
		std::unordered_map<UUID, entt::entity> enttMap;
		//Copy all entities
		auto IdView = registry.view<UniqueIdentifier>();
		for (entt::entity entity : IdView) {
			UUID& uuid = registry.get<UniqueIdentifier>(entity).uuid;
			const auto& name = registry.get<GameObject>(entity).name;
			Entity newEntity = newScene->CreateEntity(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}



		//Copy all components
		CopyComponent(AllComponents{}, newScene->Reg(), registry, enttMap);

		return newScene;
	}

	Entity Scene::CreateEntity() {
		Entity entity = { registry.create(), this };
		entity.AddComponent<UniqueIdentifier>();
		entity.AddComponent<GameObject>();
		entity.AddComponent<Transform>();
		return entity;
	}

	//Dangerous?!? Maybe remove?
	Entity Scene::CreateEntity(entt::entity _entity) {
		Entity entity = { registry.create(_entity), this };
		entity.AddComponent<UniqueIdentifier>();
		entity.AddComponent<GameObject>();
		entity.AddComponent<Transform>();
		return entity;
	}

	Entity Scene::CreateEntity(UUID uuid) {
		Entity entity = { registry.create(), this};
		entity.AddComponent<UniqueIdentifier>(uuid);
		entity.AddComponent<GameObject>();
		entity.AddComponent<Transform>();
		return entity;
	}

	Entity Scene::CreateEntity(UUID uuid, const std::string& name) {
		Entity entity = { registry.create(), this };
		entity.AddComponent<UniqueIdentifier>(uuid);
		entity.AddComponent<GameObject>(name);
		entity.AddComponent<Transform>();
		return entity;
	}

	void Scene::DestroyEntity(Entity entity) {
		registry.destroy(entity);
	}

	Entity Scene::LoadGameObject(Ref<Model> model) {
		Entity entity = { registry.create(), this};
		entity.AddComponent<UniqueIdentifier>();
		entity.AddComponent<GameObject>();
		entity.AddComponent<MeshRenderer>();
		entity.AddComponent<MeshFilter>();
		entity.GetComponent<MeshFilter>().model = model;
		entity.GetComponent<MeshRenderer>().mesh = entity.GetComponent<MeshFilter>();
		entity.AddComponent<Transform>();
		//gameObject.transform = gameObject.GetComponent<Transform>();
		return entity;
	}

	Entity Scene::LoadGameObject(Ref<Model> model, Material mat) {
		Entity entity = { registry.create(), this};
		entity.AddComponent<UniqueIdentifier>();
		entity.AddComponent<GameObject>();
		entity.AddComponent<MeshRenderer>();
		entity.AddComponent<MeshFilter>();
		entity.GetComponent<MeshFilter>().model = model;
		entity.GetComponent<MeshRenderer>().mesh = entity.GetComponent<MeshFilter>();
		entity.GetComponent<MeshRenderer>().material = std::make_shared<Material>(mat);
		entity.AddComponent<Material>();
		entity.GetComponent<Material>().shader = mat.shader;
		entity.GetComponent<Material>().diffuseMap = mat.diffuseMap;
		entity.GetComponent<Material>().normalMap = mat.normalMap;
		entity.GetComponent<Material>().ambientOcclusionMap = mat.ambientOcclusionMap;
		entity.GetComponent<Material>().glossMap = mat.glossMap;
		entity.AddComponent<Transform>();
		return entity;
	}

	void Scene::Start() {

	}

	void Scene::RunTimeStart() {

	}

	void Scene::Update() {
		{
			registry.view <NativeScriptComponent>().each([=](auto entity, auto& nsc) {
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
		}
	}

	void Scene::Render() {
		registry.view <NativeScriptComponent>().each([=](auto entity, auto& nsc) {
			nsc.onRender(nsc.Instance);
		});
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template<>
	void MADAM_API Scene::OnComponentAdded<UniqueIdentifier>(Entity entity, UniqueIdentifier& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<GameObject>(Entity entity, GameObject& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<Transform>(Entity entity, Transform& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<MeshFilter>(Entity entity, MeshFilter& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<MeshRenderer>(Entity entity, MeshRenderer& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<PointLight>(Entity entity, PointLight& component) 
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<Material>(Entity entity, Material& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{

	}

	template<>
	void MADAM_API Scene::OnComponentAdded<Camera>(Entity entity, Camera& component)
	{
		cameras.push_back(component.cameraHandle);
		MADAM_CORE_INFO("Camera added to entity");
	}
}