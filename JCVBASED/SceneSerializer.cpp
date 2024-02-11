#include "H_SceneSerializer.hpp"
#include "H_Game_Object.hpp"

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
			uuid = node.as<Digestion::UUID>();
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
			out << YAML::Key << "MeshRenderer" << YAML::Value << "True";
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

	SceneSerializer::SceneSerializer(Scene* scene) : m_Scene(scene) {

	}
	
	void SceneSerializer::Serialize(const std::string& filepath) {

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		entt::registry& reg = m_Scene->registry;
		reg.view<entt::entity>().each([&](auto entityID) {

			Entity entity = { entityID, m_Scene };
			if (!entity) return;

			SerializeEntity(out, entity);

		});
		
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath) {
		//Not Implemented
	}

	bool SceneSerializer::Deserialize(const std::string& filepath) {
		return false;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath) {
		return false;
	}

	
}