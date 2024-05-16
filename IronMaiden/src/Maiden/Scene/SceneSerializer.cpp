#include "maidenpch.hpp"
#include "H_SceneSerializer.hpp"
#include "H_Scene.hpp"
#include "H_Entity.hpp"
#include "../Core/H_Application.hpp"
//#include "H_components.hpp"

//libs
#ifndef MADAM_DYNAMIC_LINK
	#ifndef YAML_CPP_STATIC_DEFINE
		#define YAML_CPP_STATIC_DEFINE
	#endif
#endif
#include <yaml-cpp/yaml.h>

namespace YAML {

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

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
	struct convert<Madam::UUID>
	{
		static Node encode(const Madam::UUID& uuid)
		{
			Node node;
			node.push_back((std::string)uuid);
			return node;
		}

		static bool decode(const Node& node, Madam::UUID& uuid)
		{
			uuid = Madam::UUID(node.as<std::string>());
			return true;
		}
	};

	template<>
	struct convert<Ref<Madam::Shader>>
	{
		static Node encode(const Ref<Madam::Shader>& shader)
		{
			Node node;
			node.push_back(shader->vertShaderPath);
			node.push_back(shader->fragShaderPath);
			return node;
		}

		static bool decode(const Node& node, Ref<Madam::Shader>& shader)
		{
			shader->vertShaderPath = node[0].as<std::string>();
			shader->fragShaderPath = node[1].as<std::string>();
			return true;
		}
	};

	template<>
	struct convert<Madam::Shader>
	{
		static Node encode(const Madam::Shader& shader)
		{
			Node node;
			node.push_back(shader.vertShaderPath);
			node.push_back(shader.fragShaderPath);
			return node;
		}

		static bool decode(const Node& node, Madam::Shader& shader)
		{
			shader.vertShaderPath = node[0].as<std::string>();
			shader.fragShaderPath = node[1].as<std::string>();
			return true;
		}
	};

	template<>
	struct convert<Madam::Rendering::CameraData::Perspective>
	{
		static Node encode(const Madam::Rendering::CameraData::Perspective& perspective)
		{
			Node node;
			node["Fov"] = perspective.fovy;
			node["Aspect"] = perspective.aspect;
			node["Near"] = perspective.near;
			node["Far"] = perspective.far;
			return node;
		}

		static bool decode(const Node& node, Madam::Rendering::CameraData::Perspective& perspective)
		{
			perspective.fovy = node["Fov"].as<float>();
			perspective.aspect = node["Aspect"].as<float>();
			perspective.near = node["Near"].as<float>();
			perspective.far = node["Far"].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Madam::Rendering::CameraData::Orthographic>
	{
		static Node encode(const Madam::Rendering::CameraData::Orthographic& orthographic)
		{
			Node node;
			node["Size"] = orthographic.size;
			node["Aspect"] = orthographic.aspect;
			node["Near"] = orthographic.near;
			node["Far"] = orthographic.far;
			return node;
		}

		static bool decode(const Node& node, Madam::Rendering::CameraData::Orthographic& orthographic)
		{
			orthographic.size = node["Size"].as<float>();
			orthographic.aspect = node["Aspect"].as<float>();
			orthographic.near = node["Near"].as<float>();
			orthographic.far = node["Far"].as<float>();
			return true;
		}
	};
}

namespace Madam {

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

	YAML::Emitter& operator<<(YAML::Emitter& out, const Ref<Shader> s)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << s->vertShaderPath << s->fragShaderPath << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Madam::Rendering::CameraData::Perspective& p)
	{
		out << YAML::Flow;
		out << YAML::BeginMap;
		out << YAML::Key << "Fov" << YAML::Value << p.fovy;
		out << YAML::Key << "Aspect" << YAML::Value << p.aspect;
		out << YAML::Key << "Near" << YAML::Value << p.near;
		out << YAML::Key << "Far" << YAML::Value << p.far;
		out << YAML::EndMap;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Madam::Rendering::CameraData::Orthographic& o)
	{
		out << YAML::Flow;
		out << YAML::BeginMap;
		out << YAML::Key << "Size" << YAML::Value << o.size;
		out << YAML::Key << "Aspect" << YAML::Value << o.aspect;
		out << YAML::Key << "Near" << YAML::Value << o.near;
		out << YAML::Key << "Far" << YAML::Value << o.far;
		out << YAML::EndMap;
		return out;
	}

	//Optimise, Update to be more reference friendly
	static void SerializeEntity(YAML::Emitter& out, Entity entity) {

		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity.GetComponent<UniqueIdentifier>().uuid;

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
			std::cout << "Debug: " << entity.GetComponent<UniqueIdentifier>().uuid << ", Saving MeshFilter: " << std::endl;
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

		if (entity.HasComponent<Camera>()) {
			out << YAML::Key << "Camera";
			out << YAML::BeginMap;

			Camera& camera = entity.GetComponent<Camera>();
			if (camera.getCameraData().projectionType == Rendering::CameraData::ProjectionType::Perspective) {
				out << YAML::Key << "Perspective" << YAML::Value << camera.getCameraData().perspective;
			}
			else if (camera.getCameraData().projectionType == Rendering::CameraData::ProjectionType::Orthographic) {
				out << YAML::Key << "Orthographic" << YAML::Value << camera.getCameraData().orthographic;
			}
			out << YAML::Key << "ViewPosition" << YAML::Value << camera.getPosition();
			out << YAML::Key << "ViewDirection" << YAML::Value << glm::vec3(0.f); //temporary solution
			out << YAML::Key << "Main" << YAML::Value << camera.cameraHandle->isMain();
		}

		out << YAML::EndMap;
	}

	SceneSerializer::SceneSerializer(Ref<Scene> scene, Device& _device) : m_Scene(scene), device(_device) {

	}
	
	void SceneSerializer::Serialize(const std::string& rawfilePath, bool isFullPath) {

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		entt::registry& reg = m_Scene->Reg();
		reg.view<entt::entity>().each([&](auto entityID) {

			std::cout << "Entity: " << (uint32_t)entityID << std::endl;
			Entity entity = { entityID, m_Scene.get()};
			if (!entity) return;

			SerializeEntity(out, entity);

		});
		
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::string filePath = "";
		if (!isFullPath) {
			filePath = Application::Get().getConfig().projectFolder + Application::Get().getConfig().internals + rawfilePath;
		}
		else {
			filePath = rawfilePath;
		}

		std::ofstream fout(filePath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filePath) {
		//Not Implemented
	}

	bool SceneSerializer::Deserialize(const std::string& rawFilePath, bool isFullPath) {
		std::string defaultFileType = ".scene";
		std::string fileType = rawFilePath.substr(rawFilePath.size() - defaultFileType.size());

		if (fileType != defaultFileType) {
			std::cout << "Error loading scene: Parsed scene is wrong file type" << std::endl;
			return false;
		}

		std::string filePath = "";
		if (!isFullPath) {
			filePath = Application::Get().getConfig().internals + rawFilePath;
		}
		else {
			filePath = rawFilePath;
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
			std::stringstream ss;
			ss << e.what();
			std::cout << "Failed to load .scene file: " << ss.str() << std::endl;
			return false;
		}

		if (!node["Scene"]) {
			return false;
		}

		Scene newScene{};
		std::string sceneName = node["Scene"].as<std::string>();
		bool isMain = false;
		auto entities = node["Entities"];
		if (entities) {
			for (auto entity : entities) {
				
				Entity deserializedEntity = newScene.CreateEntity(entity["Entity"].as<UUID>());

				std::cout << deserializedEntity.GetComponent<UniqueIdentifier>().uuid << ", Handle: " << (uint32_t)deserializedEntity.GetHandle() << std::endl;

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
					std::string prefix = Application::Get().getConfig().projectFolder + Application::Get().getConfig().internals;
					std::string path = prefix + materialNode["Diffuse"].as<std::string>();
					material.diffuseMap = Texture::createTextureFromFile(device, path);
					path = prefix + materialNode["Normal"].as<std::string>();
					material.normalMap = Texture::createTextureFromFile(device, path);
					path = prefix + materialNode["AO"].as<std::string>();
					material.ambientOcclusionMap = Texture::createTextureFromFile(device, path);
					path = prefix + materialNode["Gloss"].as<std::string>();
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

				auto cameraNode = entity["Camera"];
				if (cameraNode) {
					Rendering::CameraData cameraData;
					auto perspectiveNode = cameraNode["Perspective"];
					auto orthographicNode = cameraNode["Orthographic"];
					if (perspectiveNode) {
						cameraData.projectionType = Rendering::CameraData::ProjectionType::Perspective;
						cameraData.perspective = Rendering::CameraData::Perspective(perspectiveNode["Fov"].as<float>(), perspectiveNode["Aspect"].as<float>(), perspectiveNode["Near"].as<float>(), perspectiveNode["Far"].as<float>());
					}
					else if (orthographicNode) {
						cameraData.projectionType = Rendering::CameraData::ProjectionType::Orthographic;
						cameraData.orthographic = Rendering::CameraData::Orthographic(orthographicNode["Size"].as<float>(), orthographicNode["Aspect"].as<float>(), orthographicNode["Near"].as<float>(), orthographicNode["Far"].as<float>());
					}
					else {
						cameraData.projectionType = Rendering::CameraData::ProjectionType::None;
					}
					Camera& camera = deserializedEntity.AddComponent<Camera>(cameraData);
					camera.cameraHandle->setProjection();
					
					camera.cameraHandle->setViewDirection(cameraNode["ViewPosition"].as<glm::vec3>(), cameraNode["ViewDirection"].as<glm::vec3>());
					if (cameraNode["Main"].as<bool>()) {
						MADAM_CORE_INFO("Is Main Camera");
						camera.cameraHandle->setMain();
						isMain = true;
					}
					else {
						MADAM_CORE_INFO("Is Not Main Camera");
					}
				}
			}
		}

		//Will need to be updated for runtime
		if (!isMain) {
			Entity camera = newScene.CreateEntity();
			camera.GetComponent<GameObject>().name = "Editor Camera";
			camera.GetComponent<Transform>().translation.z = -2.5f;
			Rendering::CameraData cameraData;
			cameraData.projectionType = Rendering::CameraData::ProjectionType::Perspective;
			cameraData.perspective = Rendering::CameraData::Perspective(glm::radians(50.0f), Application::Get().getAspectRatio(), 0.1f, 1000.0f);
			camera.AddComponent<Camera>(cameraData);
			camera.GetComponent<Camera>().cameraHandle->setViewDirection(glm::vec3(0.f, 2.0f, 0.f), glm::vec3(0.f, 0.f, 0.f));
			camera.GetComponent<Camera>().cameraHandle->setMain();
		}

		m_Scene = std::make_shared<Scene>(std::move(newScene));
		Application::Get().PrimeReserve(m_Scene);
		Application::Get().SwitchScenes(true);

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath) {
		return false;
	}
}