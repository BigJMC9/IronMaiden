#include "H_SceneSerializer.hpp"
#include "H_Game_Object.hpp"
//#include "H_components.hpp"

//std
#include <fstream>

//libs
#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Digestion::UUID>
	{
		static Node encode(const Digestion::UUID& uuid)
		{
			Node node;
			node.push_back((std::string)uuid);
			return node;
		}

		static bool decode(const Node& node, Digestion::UUID& uuid)
		{
			uuid = Digestion::UUID(node.as<std::string>());
			return true;
		}
	};

	template<>
	struct convert<std::shared_ptr<Digestion::Shader>>
	{
		static Node encode(const std::shared_ptr<Digestion::Shader>& shader)
		{
			Node node;
			node.push_back(shader->vertShaderPath);
			node.push_back(shader->fragShaderPath);
			return node;
		}

		static bool decode(const Node& node, std::shared_ptr<Digestion::Shader>& shader)
		{
			shader->vertShaderPath = node[0].as<std::string>();
			shader->fragShaderPath = node[1].as<std::string>();
			return true;
		}
	};

	template<>
	struct convert<Digestion::Shader>
	{
		static Node encode(const Digestion::Shader& shader)
		{
			Node node;
			node.push_back(shader.vertShaderPath);
			node.push_back(shader.fragShaderPath);
			return node;
		}

		static bool decode(const Node& node, Digestion::Shader& shader)
		{
			shader.vertShaderPath = node[0].as<std::string>();
			shader.fragShaderPath = node[1].as<std::string>();
			return true;
		}
	};
}

namespace Digestion {

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Shader s)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << s.vertShaderPath << s.fragShaderPath << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const std::shared_ptr<Shader> s)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << s->vertShaderPath << s->fragShaderPath << YAML::EndSeq;
		return out;
	}

	//Optimise, Update to be more reference friendly
	static void SerializeEntity(YAML::Emitter& out, Entity entity) {

		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<Transform>()) {
			out << YAML::Key << "Transform";
			out << YAML::BeginMap;

			Transform& transform = entity.GetComponent<Transform>();
			out << YAML::Key << "Translation" << YAML::Value << transform.translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.scale;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<MeshFilter>()) {
			out << YAML::Key << "MeshFilter";
			out << YAML::BeginMap;

			MeshFilter& meshFilter = entity.GetComponent<MeshFilter>();
			std::cout << "Debug: " << entity.GetUUID() << ", Saving MeshFilter: " << std::endl;
			out << YAML::Key << "Model" << YAML::Value << meshFilter.model->getFile();

			out << YAML::EndMap;
		}

		if (entity.HasComponent<Material>()) {
			out << YAML::Key << "Material";
			out << YAML::BeginMap;

			Material& material = entity.GetComponent<Material>();
			out << YAML::Key << "Shader" << YAML::Value << material.shader;
			out << YAML::Key << "Diffuse" << YAML::Value << material.diffuseMap->getFile();
			out << YAML::Key << "Normal" << YAML::Value << material.normalMap->getFile();
			out << YAML::Key << "AO" << YAML::Value << material.ambientOcclusionMap->getFile();
			out << YAML::Key << "Gloss" << YAML::Value << material.glossMap->getFile();
			out << YAML::EndMap;
		}

		if (entity.HasComponent<MeshRenderer>()) {
			out << YAML::Key << "MeshRenderer" << YAML::Value << "true";
		}

		if (entity.HasComponent<PointLight>()) {
			out << YAML::Key << "PointLight";
			out << YAML::BeginMap;

			PointLight& pointLight = entity.GetComponent<PointLight>();
			out << YAML::Key << "Color" << YAML::Value << pointLight.color;
			out << YAML::Key << "Intensity" << YAML::Value << pointLight.intensity;
			out << YAML::Key << "Radius" << YAML::Value << pointLight.radius;
			out << YAML::EndMap;
		}

		//Add after component Camera is updated
		/*if (entity.HasComponent<Camera>()) {

		}*/

		out << YAML::EndMap;
	}

	SceneSerializer::SceneSerializer(Scene* scene, Device& _device) : m_Scene(scene), device(_device) {

	}
	
	void SceneSerializer::Serialize(const std::string& filePath) {

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		entt::registry& reg = m_Scene->registry;
		reg.view<entt::entity>().each([&](auto entityID) {

			std::cout << "Entity: " << (uint32_t)entityID << std::endl;
			Entity entity = { entityID, m_Scene };
			if (!entity) return;

			SerializeEntity(out, entity);

		});
		
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filePath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filePath) {
		//Not Implemented
	}

	bool SceneSerializer::Deserialize(const std::string& filePath) {
		std::string defaultFileType = ".scene";
		std::string fileType = filePath.substr(filePath.size() - defaultFileType.size());

		if (fileType != defaultFileType) {
			std::cout << "Error loading scene: Parsed scene is wrong file type" << std::endl;
			return false;
		}

		std::ifstream file(filePath);

		if (!file.good()) {
			std::cout << "Error loading scene: Scene does not exist" << std::endl;
			return false;
		}

		YAML::Node node;
		try {
			node = YAML::LoadFile(filePath);
		}
		catch (YAML::ParserException e) {
			std::cout << "Failed to load .scene file" << std::endl;
			return false;
		}

		if (!node["Scene"]) {
			return false;
		}

		std::string sceneName = node["Scene"].as<std::string>();
		auto entities = node["Entities"];
		if (entities) {
			for (auto entity : entities) {
				
				Entity deserializedEntity = m_Scene->CreateEntity(entity["Entity"].as<UUID>());

				std::cout << deserializedEntity.GetUUID() << ", Handle: " << (uint32_t)deserializedEntity.GetHandle() << std::endl;

				auto transformNode = entity["Transform"];
				if (transformNode) 
				{
					Transform& transform = deserializedEntity.GetComponent<Transform>();
					transform.translation = transformNode["Translation"].as<glm::vec3>();
					transform.rotation = transformNode["Rotation"].as<glm::vec3>();
					transform.scale = transformNode["Scale"].as<glm::vec3>();;
				}
				
				auto meshFilterNode = entity["MeshFilter"];
				if (meshFilterNode) {
					MeshFilter& meshFilter = deserializedEntity.AddComponent<MeshFilter>();
					std::string path = meshFilterNode["Model"].as<std::string>();
					std::cout << "Loading Model: " << path << std::endl;
					meshFilter.model = Model::createModelFromFile(device, path);
				}

				auto materialNode = entity["Material"];
				if (materialNode) {
					Material& material = deserializedEntity.AddComponent<Material>();
					Shader shader = materialNode["Shader"].as<Shader>();
					material.shader = std::make_shared<Shader>(shader);
					std::string path = materialNode["Diffuse"].as<std::string>();
					material.diffuseMap = Texture::createTextureFromFile(device, path);
					path = materialNode["Normal"].as<std::string>();
					material.normalMap = Texture::createTextureFromFile(device, path);
					path = materialNode["AO"].as<std::string>();
					material.ambientOcclusionMap = Texture::createTextureFromFile(device, path);
					path = materialNode["Gloss"].as<std::string>();
					material.glossMap = Texture::createTextureFromFile(device, path);
				}

				auto pointLightNode = entity["PointLight"];
				if (pointLightNode) {
					PointLight& pointLight = deserializedEntity.AddComponent<PointLight>();
					pointLight.color = pointLightNode["Color"].as<glm::vec3>();
					pointLight.intensity = pointLightNode["Intensity"].as<float>();
					pointLight.radius = pointLightNode["Radius"].as<float>();
				}

				auto meshRendererNode = entity["MeshRenderer"];
				if (meshRendererNode) {
					std::string isTrue = meshRendererNode.as<std::string>();
					if (isTrue == "true") {
						MeshRenderer& meshRenderer = deserializedEntity.AddComponent<MeshRenderer>();
						if (deserializedEntity.HasComponent<Material>()) {
							deserializedEntity.GetComponent<MeshRenderer>().material = std::make_shared<Material>(deserializedEntity.GetComponent<Material>());
						}
						deserializedEntity.GetComponent<MeshRenderer>().mesh = deserializedEntity.GetComponent<MeshFilter>();
					}
				}
			}
		}
		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath) {
		return false;
	}

	
}