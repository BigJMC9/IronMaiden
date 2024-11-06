#include "maidenpch.hpp"
#include "H_SceneSerializer.hpp"
#include "H_Scene.hpp"
#include "H_Entity.hpp"
#include "../Core/H_Application.hpp"
#include "../Project/H_Project.h"
#include "../Asset/H_AssetSystem.h"
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
	struct convert<std::filesystem::path>
	{
		static Node encode(const std::filesystem::path& rhs)
		{
			Node node;
			node.push_back(rhs.string());
			return node;
		}

		static bool decode(const Node& node, std::filesystem::path& rhs)
		{
			rhs = std::filesystem::u8path(node.as<std::string>());
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
	struct convert<Ref<Madam::CShader>>
	{
		static Node encode(const Ref<Madam::CShader>& shader)
		{
			Node node;
			node.push_back(shader->vertShaderPath);
			node.push_back(shader->fragShaderPath);
			return node;
		}

		static bool decode(const Node& node, Ref<Madam::CShader>& shader)
		{
			shader->vertShaderPath = node[0].as<std::string>();
			shader->fragShaderPath = node[1].as<std::string>();
			return true;
		}
	};

	template<>
	struct convert<Madam::CShader>
	{
		static Node encode(const Madam::CShader& shader)
		{
			Node node;
			node.push_back(shader.vertShaderPath);
			node.push_back(shader.fragShaderPath);
			return node;
		}

		static bool decode(const Node& node, Madam::CShader& shader)
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

	YAML::Emitter& operator<<(YAML::Emitter& out, const CShader s)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << s.vertShaderPath << s.fragShaderPath << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Ref<CShader> s)
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
		out << YAML::Key << "Entity" << YAML::Value << entity.GetComponent<CUniqueIdentifier>().uuid;

		if (entity.HasComponent<CTransform>()) {
			out << YAML::Key << "Transform";
			out << YAML::BeginMap;

			CTransform& transform = entity.GetComponent<CTransform>();
			out << YAML::Key << "Translation" << YAML::Value << transform.translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.scale;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CMaterial>()) {
			out << YAML::Key << "Material";
			out << YAML::BeginMap;

			CMaterial& material = entity.GetComponent<CMaterial>();
			out << YAML::Key << "Shader" << YAML::Value << material.shader;
			out << YAML::Key << "Diffuse" << YAML::Value << material.diffuseMap->GetUUID();
			out << YAML::Key << "Normal" << YAML::Value << material.normalMap->GetUUID();
			out << YAML::Key << "AO" << YAML::Value << material.ambientOcclusionMap->GetUUID();
			out << YAML::Key << "Gloss" << YAML::Value << material.glossMap->GetUUID();
			out << YAML::EndMap;
		}

		if (entity.HasComponent<CMeshRenderer>()) {
			out << YAML::Key << "MeshRenderer";
			out << YAML::BeginMap;

			CMeshRenderer& meshRenderer = entity.GetComponent<CMeshRenderer>();
			out << YAML::Key << "Material" << YAML::Value << "true";
			out << YAML::Key << "StaticMesh" << YAML::Value << meshRenderer.GetMesh()->GetFilepath().string();
			out << YAML::EndMap;
		}

		if (entity.HasComponent<CPointLight>()) {
			out << YAML::Key << "PointLight";
			out << YAML::BeginMap;

			CPointLight& pointLight = entity.GetComponent<CPointLight>();
			out << YAML::Key << "Color" << YAML::Value << pointLight.color;
			out << YAML::Key << "Intensity" << YAML::Value << pointLight.intensity;
			out << YAML::Key << "Radius" << YAML::Value << pointLight.radius;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<CCamera>()) {
			out << YAML::Key << "Camera";
			out << YAML::BeginMap;

			CCamera& camera = entity.GetComponent<CCamera>();
			if (camera.GetCameraData().projectionType == Rendering::CameraData::ProjectionType::Perspective) {
				out << YAML::Key << "Perspective" << YAML::Value << camera.GetCameraData().perspective;
			}
			else if (camera.GetCameraData().projectionType == Rendering::CameraData::ProjectionType::Orthographic) {
				out << YAML::Key << "Orthographic" << YAML::Value << camera.GetCameraData().orthographic;
			}
			out << YAML::Key << "ViewPosition" << YAML::Value << camera.GetPosition();
			out << YAML::Key << "ViewDirection" << YAML::Value << glm::vec3(0.f); //temporary solution
			out << YAML::Key << "Main" << YAML::Value << camera.cameraHandle->IsMain();
			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	SceneSerializer::SceneSerializer(Ref<Scene> scene, Device& _device) : m_Scene(scene), device(_device) {

	}
	
	void SceneSerializer::Serialize(const std::filesystem::path& filePath) {

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

		std::ofstream fout(filePath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filePath) {
		//Not Implemented
	}

	bool SceneSerializer::Deserialize(const std::filesystem::path& filePath) {
		std::string fileType = filePath.extension().string();

		if (fileType != ".scene") {
			MADAM_ERROR("Error loading scene: Parsed scene is wrong file type");
			return false;
		}

		std::ifstream file(filePath);

		if (!file.good()) {
			MADAM_ERROR("Error loading scene: Scene file does not exist");
			return false;
		}

		YAML::Node node;
		try {
			node = YAML::LoadFile(filePath.string().c_str());
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

				std::cout << deserializedEntity.GetComponent<CUniqueIdentifier>().uuid << ", Handle: " << (uint32_t)deserializedEntity.GetHandle() << std::endl;

				auto transformNode = entity["Transform"];
				if (transformNode) 
				{
					CTransform& transform = deserializedEntity.GetComponent<CTransform>();
					transform.translation = transformNode["Translation"].as<glm::vec3>();
					transform.rotation = transformNode["Rotation"].as<glm::vec3>();
					transform.scale = transformNode["Scale"].as<glm::vec3>();;
				}

				auto materialNode = entity["Material"];
				if (materialNode) {
					CMaterial& material = deserializedEntity.AddComponent<CMaterial>();
					CShader shader = materialNode["Shader"].as<CShader>();
					material.shader = std::make_shared<CShader>(shader);
					UUID uuid = materialNode["Diffuse"].as<UUID>();
					TextureData textureData;
					material.diffuseMap = std::static_pointer_cast<Texture>(Project::Get().getAssetManager().GetAsset(uuid));
					uuid = materialNode["Normal"].as<UUID>();
					material.normalMap = std::static_pointer_cast<Texture>(Project::Get().getAssetManager().GetAsset(uuid));
					uuid = materialNode["AO"].as<UUID>();
					material.ambientOcclusionMap = std::static_pointer_cast<Texture>(Project::Get().getAssetManager().GetAsset(uuid));
					uuid = materialNode["Gloss"].as<UUID>();
					material.glossMap = std::static_pointer_cast<Texture>(Project::Get().getAssetManager().GetAsset(uuid));
				}

				auto pointLightNode = entity["PointLight"];
				if (pointLightNode) {
					CPointLight& pointLight = deserializedEntity.AddComponent<CPointLight>();
					pointLight.color = pointLightNode["Color"].as<glm::vec3>();
					pointLight.intensity = pointLightNode["Intensity"].as<float>();
					pointLight.radius = pointLightNode["Radius"].as<float>();
				}

				auto meshRendererNode = entity["MeshRenderer"];
				if (meshRendererNode) {
					std::string material = meshRendererNode["Material"].as<std::string>();
					CMeshRenderer& meshRenderer = deserializedEntity.AddComponent<CMeshRenderer>();
					std::filesystem::path filepath = meshRendererNode["StaticMesh"].as<std::filesystem::path>();
					deserializedEntity.GetComponent<CMeshRenderer>().mesh = StaticMesh::Create(Project::Get().getProjectDirectory() / std::filesystem::u8path("Assets") / filepath);
					if (material == "true") {
						
						if (deserializedEntity.HasComponent<CMaterial>()) {
							deserializedEntity.GetComponent<CMeshRenderer>().material = std::make_shared<CMaterial>(deserializedEntity.GetComponent<CMaterial>());
						}
						
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
					CCamera& camera = deserializedEntity.AddComponent<CCamera>(cameraData);
					camera.cameraHandle->SetProjection();
					
					camera.cameraHandle->SetViewDirection(cameraNode["ViewPosition"].as<glm::vec3>(), cameraNode["ViewDirection"].as<glm::vec3>());
					if (cameraNode["Main"].as<bool>()) {
						MADAM_CORE_INFO("Is Main Camera");
						camera.cameraHandle->SetMain();
						isMain = true;
					}
					else {
						MADAM_CORE_INFO("Is Not Main Camera");
					}
				}
			}
		}

		//Will need to be updated for runtime
		/*if (!isMain) {
			Entity camera = newScene.CreateEntity();
			camera.GetComponent<Metadata>().name = "Editor Camera";
			camera.GetComponent<Transform>().translation.z = -2.5f;
			Rendering::CameraData cameraData;
			cameraData.projectionType = Rendering::CameraData::ProjectionType::Perspective;
			cameraData.perspective = Rendering::CameraData::Perspective(glm::radians(50.0f), Application::Get().getAspectRatio(), 0.1f, 1000.0f);
			camera.AddComponent<Camera>(cameraData);
			camera.GetComponent<Camera>().cameraHandle->SetViewDirection(glm::vec3(0.f, 2.0f, 0.f), glm::vec3(0.f, 0.f, 0.f));
			camera.GetComponent<Camera>().cameraHandle->SetMain();
		}*/

		m_Scene = std::make_shared<Scene>(std::move(newScene));
		Application::Get().SwitchScenes(m_Scene);

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath) {
		return false;
	}
}