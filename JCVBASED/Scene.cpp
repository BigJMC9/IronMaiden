#include "H_first_app.hpp"
#include "H_Scene.hpp"
#include "H_keyboard_movement_controller.hpp"
#include "H_Game_Object.hpp"
#include "H_components.hpp"
#include "H_JCVB_renderer.hpp"


//std
#include <iostream>
#include <fstream>
#include <map>
#include <functional>

//namespace fs = std::filesystem;

namespace Digestion {

	Scene::Scene() {
	}

	Scene::~Scene()
	{
		registry.clear();
	}

	Entity Scene::CreateEntity() {
		Entity entity = { registry.create(), this };
		entity.AddComponent<Transform>();
		entity.transform = entity.GetComponent<Transform>();
		//m_EntityMap.emplace(entity.GetUUID(), entity);
		return entity;
	}

	Entity Scene::CreateEntity(entt::entity _entity) {
		Entity entity = { registry.create(_entity), this };
		entity.AddComponent<Transform>();
		entity.transform = entity.GetComponent<Transform>();
		//m_EntityMap.emplace(entity.GetUUID(), entity);
		return entity;
	}

	Entity Scene::CreateEntity(UUID uuid) {
		Entity entity = { registry.create(), this, uuid};
		entity.AddComponent<Transform>();
		entity.transform = entity.GetComponent<Transform>();
		//m_EntityMap.emplace(entity.GetUUID(), entity);
		return entity;
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

	Entity Scene::LoadGameObject(std::shared_ptr<Model> model, Material mat) {
		Entity gameObject = { registry.create(), this};
		gameObject.AddComponent<MeshRenderer>();
		gameObject.AddComponent<MeshFilter>();
		gameObject.GetComponent<MeshFilter>().model = model;
		gameObject.GetComponent<MeshRenderer>().mesh = gameObject.GetComponent<MeshFilter>();
		gameObject.GetComponent<MeshRenderer>().material = std::make_shared<Material>(mat);
		gameObject.AddComponent<Material>();
		gameObject.GetComponent<Material>().shader = mat.shader;
		gameObject.GetComponent<Material>().diffuseMap = mat.diffuseMap;
		gameObject.GetComponent<Material>().normalMap = mat.normalMap;
		gameObject.GetComponent<Material>().ambientOcclusionMap = mat.ambientOcclusionMap;
		gameObject.GetComponent<Material>().glossMap = mat.glossMap;
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

	/*
	void Scene::LoadScene() {

	}

	void Scene::LoadScene(std::string scenePath) {
		std::string defaultFileType = ".scene";
		std::string fileType = scenePath.substr(scenePath.size() - defaultFileType.size());
		std::cout << fileType;

		if (fileType != defaultFileType) {
			std::cout << "Error loading scene: Parsed scene is wrong file type" << std::endl;
			return;
		}

		std::ifstream file(scenePath);

		if (!file.good()) {
			std::cout << "Error loading scene: Scene does not exist" << std::endl;
			return;
		}

		std::map<std::string, std::function<void(YAML::Node, Entity)>> optionMap = {
			{"Transform", std::bind(&Scene::LoadTransform, this, std::placeholders::_1, std::placeholders::_2)},
			{"MeshFilter", std::bind(&Scene::LoadMeshFilter, this, std::placeholders::_1, std::placeholders::_2)},
			{"MeshRenderer", std::bind(&Scene::LoadMeshRenderer, this, std::placeholders::_1, std::placeholders::_2)},
			{"Material", std::bind(&Scene::LoadMaterial, this, std::placeholders::_1, std::placeholders::_2)},
			{"PointLight", std::bind(&Scene::LoadPointLight, this, std::placeholders::_1, std::placeholders::_2)},
			{"Camera", std::bind(&Scene::LoadCamera, this, std::placeholders::_1, std::placeholders::_2)}
		};

		
		YAML::Parser parser(file);
		YAML::Node doc;
		//parser.Load(file);
		

		while (parser.) {
			if (doc.Tag() == "!u!1") {
				Entity gameObject = { registry.create(), this};

				const YAML::Node& componentsNode = node["components"];
				for (const auto& componentNode : componentsNode) {
					const std::string componentName = componentNode.first.as<std::string>();
					auto it = optionMap.find(componentName);
					if (it != optionMap.end()) {
						it->second(componentNode.second, gameObject);
					}
					else {
						std::cout << "Error Loading Unknown component: " << componentName << std::endl;
					}
				}
			}
			//UUID id = { node["uuid"].as<std::string>() };	
		}
	}

	void Scene::LoadTransform(YAML::Node node, Entity gameObject) {

		gameObject.AddComponent<Transform>();
		gameObject.transform = gameObject.GetComponent<Transform>();

		//translation vec3
		const YAML::Node& translation = node["Translation"];
		gameObject.GetComponent<Transform>().translation.x = translation["x"].as<float>();
		gameObject.GetComponent<Transform>().translation.y = translation["y"].as<float>();
		gameObject.GetComponent<Transform>().translation.z = translation["z"].as<float>();
		//scale vec3
		const YAML::Node& scale = node["Scale"];
		gameObject.GetComponent<Transform>().scale .x = scale["x"].as<float>();
		gameObject.GetComponent<Transform>().scale.y = scale["y"].as<float>();
		gameObject.GetComponent<Transform>().scale.z = scale["z"].as<float>();
		//rotation vec3
		const YAML::Node& rotation = node["Rotation"];
		gameObject.GetComponent<Transform>().rotation.x = rotation["x"].as<float>();
		gameObject.GetComponent<Transform>().rotation.y = rotation["y"].as<float>();
		gameObject.GetComponent<Transform>().rotation.z = rotation["z"].as<float>();
	}

	void Scene::LoadPointLight(YAML::Node node, Entity gameObject) {

		gameObject.AddComponent<PointLight>();

		//color vec3
		const YAML::Node& color = node["Color"];
		gameObject.GetComponent<PointLight>().color.x = color["x"].as<float>();
		gameObject.GetComponent<PointLight>().color.y = color["y"].as<float>();
		gameObject.GetComponent<PointLight>().color.z = color["z"].as<float>();
		//radius float
		gameObject.GetComponent<PointLight>().radius = node["Radius"].as<float>();
		//intensity float
		gameObject.GetComponent<PointLight>().intensity = node["Intensity"].as<float>();
	}

	void Scene::LoadMeshFilter(YAML::Node node, Entity gameObject) {

		if (gameObject.HasComponent<MeshFilter>()) {
			return;
		}
		gameObject.AddComponent<MeshFilter>();

		//model string
		std::string modelPath = node["Model"].as<std::string>();

		std::shared_ptr<Model> model = Model::createModelFromFile(device, modelPath);
		gameObject.GetComponent<MeshFilter>().model = model;
	}

	void Scene::LoadMeshRenderer(YAML::Node node, Entity gameObject) {

		gameObject.AddComponent<MeshRenderer>();

		//Add meshfilter to list of referential then loop through referentials at end?

		if (!gameObject.HasComponent<MeshFilter>()) {
			LoadMeshFilter(node["MeshFilter"], gameObject);
		}
		gameObject.GetComponent<MeshRenderer>().mesh = gameObject.GetComponent<MeshFilter>();

		if (!gameObject.HasComponent<Material>()) {

		}
	}

	Shader Scene::LoadShader(YAML::Node node) {
		Shader shader;
		shader.vertShaderPath = node["VertexShaderPath"].as<std::string>();
		shader.fragShaderPath = node["FragmentShaderPath"].as<std::string>();

		return shader;
	}

	void Scene::LoadMaterial(YAML::Node node, Entity gameObject) {

		gameObject.AddComponent<Material>();

		gameObject.GetComponent<Material>().shader = std::make_shared<Shader>(LoadShader(node["Shader"]));
		gameObject.GetComponent<Material>().diffuseMap = Texture::createTextureFromFile(device, node["DiffuseMap"].as<std::string>());
		gameObject.GetComponent<Material>().normalMap = Texture::createTextureFromFile(device, node["NormalMap"].as<std::string>());
		gameObject.GetComponent<Material>().ambientOcclusionMap = Texture::createTextureFromFile(device, node["AmbientOcclusionMap"].as<std::string>());
		gameObject.GetComponent<Material>().glossMap = Texture::createTextureFromFile(device, node["GlossMap"].as<std::string>());
	}

	void Scene::LoadCamera(YAML::Node node, Entity gameObject) {

	}
	*/
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

	template<>
	void Scene::OnComponentAdded<Material>(Entity entity, Material& component)
	{

	}
}