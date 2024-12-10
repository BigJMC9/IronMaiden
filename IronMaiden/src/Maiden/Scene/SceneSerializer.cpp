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
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::quat& rhs)
		{
			if (!node.IsSequence() || node.size() > 4 || node.size() < 3)
			{
				return false;
			}
			
			if (node.size() == 3)
			{
				glm::vec3 angles;
				angles.x = node[0].as<float>();
				angles.y = node[1].as<float>();
				angles.z = node[2].as<float>();
				rhs = glm::quat(angles);
				return true;
			}
			else
			{
				rhs.x = node[0].as<float>();
				rhs.y = node[1].as<float>();
				rhs.z = node[2].as<float>();
				rhs.w = node[3].as<float>();
				return true;
			}
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

	YAML::Emitter& operator<<(YAML::Emitter& out, const UUID uuid)
	{
		out << (std::string)uuid;
		return out;
	}

	template<typename T>
	YAML::Emitter& operator<<(YAML::Emitter& out, const std::vector<T>& v)
	{
		out << YAML::BeginSeq;
		for (size_t i = 0; i < v.size(); i++)
		{
			out << v[i];
		}
		out << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const std::vector<UUID>& v)
	{
		out << YAML::BeginSeq;
		for (size_t i = 0; i < v.size(); i++)
		{
			out << v[i];
		}
		out << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
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

	static std::optional<YAML::Node> GetNode(const YAML::Node& parentNode, const std::string& nodeName)
	{
		try {
			if (!parentNode[nodeName].IsDefined())
			{
				MADAM_ERROR("The node \"{0}\" is not defined in the YAML Parent Node.", nodeName);
				return std::nullopt;
			}
			if (parentNode[nodeName].IsNull())
			{
				MADAM_ERROR("The node \"{0}\" is NULL.", nodeName);
				return std::nullopt;
			}
			return parentNode[nodeName];
		}
		catch (const YAML::BadConversion& e)
		{
			MADAM_ERROR("Failed to convert the node \"{0}\" to the specified type. \n{1}", nodeName, e.what());
		}
		catch (const YAML::Exception& e)
		{
			MADAM_ERROR("Unable to read the node \"{0}\". \nA YAML exception occurred. \n{1}", nodeName, e.what());
		}
		catch (const std::exception& e)
		{
			MADAM_ERROR("Unable to read the node \"{0}\". \nA standard exception occurred. \n{1}", nodeName, e.what());
		}
		catch (...)
		{
			MADAM_ERROR("Unable to read the node \"{0}\". \nAn unknown exception occurred.");
		}
		return std::nullopt;
	}

	template <typename T>
	static std::optional<T> GetNodeValue(const YAML::Node& parentNode, const std::string& nodeName)
	{
		try {
			if (!parentNode[nodeName].IsDefined())
			{
				MADAM_ERROR("The node \"{0}\" is not defined in the YAML Parent Node.", nodeName);
				return std::nullopt;
			}
			if (parentNode[nodeName].IsNull())
			{
				MADAM_ERROR("The node \"{0}\" is NULL.", nodeName);
				return std::nullopt;
			}
			return parentNode[nodeName].as<T>();
		}
		catch (const YAML::BadConversion& e)
		{
			MADAM_ERROR("Failed to convert the node \"{0}\" to the specified type. \n{1}", nodeName, e.what());
		}
		catch (const YAML::Exception& e)
		{
			MADAM_ERROR("Unable to read the node \"{0}\". \nA YAML exception occurred. \n{1}", nodeName, e.what());
		}
		catch (const std::exception& e)
		{
			MADAM_ERROR("Unable to read the node \"{0}\". \nA standard exception occurred. \n{1}", nodeName, e.what());
		}
		catch (...)
		{
			MADAM_ERROR("Unable to read the node \"{0}\". \nAn unknown exception occurred.");
		}
		return std::nullopt;
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

		if (entity.HasComponent<CMetadata>()) {
			out << YAML::Key << "Metadata";
			out << YAML::BeginMap;

			CMetadata& metadata = entity.GetComponent<CMetadata>();
			out << YAML::Key << "Name" << YAML::Value << metadata.name;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CRelationship>()) {
			out << YAML::Key << "Relationship";
			out << YAML::BeginMap;

			CRelationship& relationship = entity.GetComponent<CRelationship>();
			out << YAML::Key << "Parent" << YAML::Value << relationship.parent;
			out << YAML::Key << "Children" << YAML::Value << relationship.children;

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
			out << YAML::Key << "IsPrimative" << YAML::Value << meshRenderer.GetMesh()->IsPrimative();
			if (!meshRenderer.GetMesh()->IsPrimative())
			{
				out << YAML::Key << "StaticMesh" << YAML::Value << meshRenderer.GetMesh()->GetFilepath().string();
			}
			else
			{
				out << YAML::Key << "StaticMesh" << YAML::Value << meshRenderer.GetMesh()->GetPrimativeAsString();
			}
			MADAM_CORE_INFO(meshRenderer.GetMesh()->GetFilepath().string());
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

		std::filesystem::path fileName = filePath.stem();
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Version" << YAML::Value << Application::Get().GetConfig().version;
		out << YAML::Key << "Scene" << YAML::Value << fileName.string();
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
			MADAM_ERROR("Failed to load scene: Parsed scene is wrong file type");
			return false;
		}

		std::ifstream file(filePath);

		if (!file.good()) {
			MADAM_ERROR("Failed to load scene: Scene file does not exist");
			return false;
		}

		YAML::Node parentNode;
		try {
			parentNode = YAML::LoadFile(filePath.string().c_str());
		}
		catch (YAML::ParserException e) {
			std::stringstream ss;
			ss << e.what();
			std::cout << "YAML has failed to load scene file: " << ss.str() << std::endl;
			return false;
		}

		if (!parentNode["Scene"]) {
			MADAM_ERROR("Failed to load scene: Scene Node does not exist.");
			return false;
		}

		Scene newScene{};
		std::string version = GetNodeValue<std::string>(parentNode, "Version").has_value() ? GetNodeValue<std::string>(parentNode, "Version").value() : "";
		if (version == null) {
			MADAM_ERROR("Failed to load scene: Unable to read version of the scene file");
			return false;
		}

		size_t buildVersionPos = version.find_last_of('.');

		if (buildVersionPos == std::string::npos) {
			MADAM_ERROR("Failed to load scene: Unable to read version of the scene file. \nUnable to find build version of the provided version string.");
			return false;
		}

		std::string relevantVersion = version.substr(0, buildVersionPos);
		std::string applicationRelevantVersion = Application::Get().GetConfig().version.substr(0, buildVersionPos);


		if (applicationRelevantVersion != relevantVersion) {
			MADAM_CORE_ERROR("Failed to load scene: The Application version and Scene file version do not match\nApplication Version: {0}\nScene Version: {1}", Application::Get().GetConfig().version, version);
			return false;
		}

		
		std::string sceneName = GetNodeValue<std::string>(parentNode, "Scene").has_value() ? GetNodeValue<std::string>(parentNode, "Version").value() : "";
		if (sceneName == null)
		{
			MADAM_ERROR("Failed to load scene: Unable to obtain Scene Name");
			return false;
		}
		//Put in a entity serializer
		bool isMain = false;

		YAML::Node entities = GetNode(parentNode, "Entities").has_value() ? GetNode(parentNode, "Entities").value() : YAML::Node{};
		if (entities) {
			MADAM_INFO("Entities size: {0}", entities.size());
			for (auto entity : entities) {
				try
				{
					Entity deserializedEntity = newScene.CreateEntity(entity["Entity"].as<UUID>());

					std::cout << deserializedEntity.GetComponent<CUniqueIdentifier>().uuid << ", Handle: " << (uint32_t)deserializedEntity.GetHandle() << std::endl;

					auto transformNode = entity["Transform"];
					if (transformNode)
					{
						CTransform& transform = deserializedEntity.GetComponent<CTransform>();
						transform.translation = transformNode["Translation"].as<glm::vec3>();
						transform.rotation = transformNode["Rotation"].as<glm::quat>();
						transform.scale = transformNode["Scale"].as<glm::vec3>();
					}
					auto metadataNode = entity["Metadata"];
					if (metadataNode)
					{
						CMetadata& metadata = deserializedEntity.GetComponent<CMetadata>();
						metadata.name = metadataNode["Name"].as<std::string>();
					}
					auto relationshipNode = entity["Relationship"];
					if (relationshipNode)
					{
						CRelationship& relationship = deserializedEntity.GetComponent<CRelationship>();
						relationship.parent = relationshipNode["Parent"].as<UUID>();
						relationship.children = relationshipNode["Children"].as<std::vector<UUID>>();
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
						bool isPrimative = meshRendererNode["IsPrimative"].as<bool>();
						if (!isPrimative)
						{
							std::filesystem::path filepath = meshRendererNode["StaticMesh"].as<std::filesystem::path>();
							deserializedEntity.GetComponent<CMeshRenderer>().mesh = StaticMesh::Create(Project::Get().getProjectDirectory() / std::filesystem::u8path("Assets") / filepath);
						}
						else
						{
							MeshPrimatives primative = static_cast<MeshPrimatives>(meshPrimativesMap[meshRendererNode["StaticMesh"].as<std::string>()]);
							deserializedEntity.GetComponent<CMeshRenderer>().mesh = StaticMesh::Create(primative);
						}
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
							camera.cameraHandle->SetMain();
							deserializedEntity.GetComponent<CMetadata>().isHiddenEntity = true;
							isMain = true;
						}
					}
				}
				catch (const YAML::BadConversion& e)
				{
					MADAM_ERROR("Failed to convert the node \"{0}\" to the specified type. \n{1}", "Entity", e.what());
				}
				catch (const YAML::Exception& e)
				{
					MADAM_ERROR("Unable to read the node \"{0}\". \nA YAML exception occurred. \n{1}", "Entity", e.what());
				}
				catch (const std::exception& e)
				{
					MADAM_ERROR("Unable to read the node \"{0}\". \nA standard exception occurred. \n{1}", "Entity", e.what());
				}
				catch (...)
				{
					MADAM_ERROR("Unable to read the node \"{0}\". \nAn unknown exception occurred.");
				}
			}
		}
		else
		{
			MADAM_ERROR("Failed to load scene: Unable to read the node \"Entities\"");
		}

		//Will need to be updated for runtime
		/*if (!isMain) {
			Entity camera = newScene.CreateEntity();
			camera.GetComponent<Metadata>().name = "Editor Camera";
			camera.GetComponent<Transform>().translation.z = -2.5f;
			Rendering::CameraData cameraData;
			cameraData.projectionType = Rendering::CameraData::ProjectionType::Perspective;
			cameraData.perspective = Rendering::CameraData::Perspective(glm::radians(50.0f), Application::Get().GetAspectRatio(), 0.1f, 1000.0f);
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