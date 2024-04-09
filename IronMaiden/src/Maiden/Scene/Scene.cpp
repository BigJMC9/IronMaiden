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

	Entity Scene::CreateEntity() {
		Entity entity = { registry.create(), this };
		entity.AddComponent<UniqueIdentifier>();
		entity.AddComponent<Object>();
		entity.AddComponent<Transform>();
		//entity.transform = entity.GetComponent<Transform>();
		//m_EntityMap.emplace(entity.GetUUID(), entity);
		return entity;
	}

	Entity Scene::CreateEntity(entt::entity _entity) {
		Entity entity = { registry.create(_entity), this };
		entity.AddComponent<UniqueIdentifier>();
		entity.AddComponent<Object>();
		entity.AddComponent<Transform>();
		//entity.transform = entity.GetComponent<Transform>();
		//m_EntityMap.emplace(entity.GetUUID(), entity);
		return entity;
	}

	Entity Scene::CreateEntity(UUID uuid) {
		Entity entity = { registry.create(), this};
		entity.AddComponent<UniqueIdentifier>(uuid);
		entity.AddComponent<Object>();
		entity.AddComponent<Transform>();
		//entity.transform = entity.GetComponent<Transform>();
		//m_EntityMap.emplace(entity.GetUUID(), entity);
		return entity;
	}

	Entity Scene::LoadGameObject(std::shared_ptr<Model> model) {
		Entity entity = { registry.create(), this};
		entity.AddComponent<UniqueIdentifier>();
		entity.AddComponent<Object>();
		entity.AddComponent<MeshRenderer>();
		entity.AddComponent<MeshFilter>();
		entity.GetComponent<MeshFilter>().model = model;
		entity.GetComponent<MeshRenderer>().mesh = entity.GetComponent<MeshFilter>();
		entity.AddComponent<Transform>();
		//gameObject.transform = gameObject.GetComponent<Transform>();
		return entity;
	}

	Entity Scene::LoadGameObject(std::shared_ptr<Model> model, Material mat) {
		Entity entity = { registry.create(), this};
		entity.AddComponent<UniqueIdentifier>();
		entity.AddComponent<Object>();
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
		//gameObject.transform = gameObject.GetComponent<Transform>();
		return entity;
	}

	void Scene::Start() {

	}

	void Scene::Update() {
		{
			registry.view <NativeScriptComponent>().each([=](auto entity, auto& nsc) {
				if (!nsc.instance) {
					nsc.instantiate();
					nsc.instance->entity = Entity{ entity, this };
					nsc.onCreate(nsc.instance);
					nsc.onStart(nsc.instance);
					//nsc.instance->entity = Entity{ entity, this };
					//nsc.instance->onCreate(nsc.instance);
					//nsc.instance->onStart(nsc.instance);
				}
				nsc.onUpdate(nsc.instance);
				nsc.onLateUpdate(nsc.instance);
			});
		}
	}

	void Scene::Render() {
		registry.view <NativeScriptComponent>().each([=](auto entity, auto& nsc) {
			nsc.onRender(nsc.instance);
		});
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentAdded<UniqueIdentifier>(Entity entity, UniqueIdentifier& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<Object>(Entity entity, Object& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<Transform>(Entity entity, Transform& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<MeshFilter>(Entity entity, MeshFilter& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<MeshRenderer>(Entity entity, MeshRenderer& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<PointLight>(Entity entity, PointLight& component) 
	{

	}

	template<>
	void Scene::OnComponentAdded<Material>(Entity entity, Material& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{

	}

	//Error on Entity Destruction?
	template<>
	void Scene::OnComponentAdded<Camera>(Entity entity, Camera& component)
	{
		cameras.push_back(component.cameraHandle);
		MADAM_CORE_INFO("Camera added to entity");
	}
}