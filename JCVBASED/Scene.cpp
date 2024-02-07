#include "H_scene.hpp"
#include "H_keyboard_movement_controller.hpp"
#include "H_Game_Object.hpp"
#include "H_components.hpp"
#include "H_JCVB_renderer.hpp"

//std
#include <iostream>

namespace Digestion {

	Scene::~Scene()
	{

	}

	Entity Scene::CreateGameObject() {
		Entity gameObject = { registry.create(), this};
		gameObject.AddComponent<Transform>();
		gameObject.transform = gameObject.GetComponent<Transform>();
		return gameObject;
	}

	Entity Scene::LoadGameObject(std::shared_ptr<Model> model) {
		Entity gameObject = { registry.create(), this};
		gameObject.AddComponent<MeshRenderer>();
		gameObject.AddComponent<MeshFilter>();
		gameObject.GetComponent<MeshFilter>().model = model;
		gameObject.GetComponent<MeshRenderer>().mesh = gameObject.GetComponent<MeshFilter>();
		gameObject.AddComponent<Transform>();
		gameObject.transform = gameObject.GetComponent<Transform>();
		return gameObject;
	}

	Entity Scene::LoadGameObject(std::shared_ptr<Model> model, std::shared_ptr<Material> mat) {
		Entity gameObject = { registry.create(), this};
		gameObject.AddComponent<MeshRenderer>();
		gameObject.AddComponent<MeshFilter>();
		gameObject.GetComponent<MeshFilter>().model = model;
		gameObject.GetComponent<MeshRenderer>().mesh = gameObject.GetComponent<MeshFilter>();
		gameObject.GetComponent<MeshRenderer>().material = mat;
		gameObject.AddComponent<Transform>();
		gameObject.transform = gameObject.GetComponent<Transform>();
		return gameObject;
	}

	void Scene::Start() {

	}

	void Scene::Update() {

	}

	void Scene::Render() {

	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentAdded<Transform>(Entity entity, Transform& component)
	{

	}

	/*template<>
	void Scene::OnComponentAdded<Camera>(GameObject entity, Camera& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}*/

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
}