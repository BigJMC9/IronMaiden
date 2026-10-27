#pragma once
#ifndef H_COMPONENTS
#define H_COMPONENTS

#include "maidenpch.hpp"
#include "../Rendering/H_Mesh.h"
#include "../Rendering/H_Texture.h"
#include "ScriptableEntity.hpp"
#include "../Rendering/H_Camera.hpp"
#include <map>

#define INCLUDE_GLM_UTILS
#include "../Core/H_Logger.hpp"

//libs
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#ifdef far
#undef far
#endif
#ifdef near
#undef near
#endif

namespace Madam
{

	struct CUniqueIdentifier
	{
		UUID uuid = UUID();

		CUniqueIdentifier() = default;
		CUniqueIdentifier(UUID _uuid)
		{
			uuid = _uuid;
		};
	};

	struct CCamera
	{
		CCamera()
		{
			Rendering::CameraData _cameraData = Rendering::CameraHandle::GetDefaultCameraData();
			cameraHandle = CreateRef<Rendering::CameraHandle>(_cameraData);
		};
		CCamera(Rendering::CameraData _cameraData)
		{
			cameraHandle = CreateRef<Rendering::CameraHandle>(_cameraData);
		};
		CCamera(const CCamera& source)
		{
			cameraHandle = CreateRef<Rendering::CameraHandle>(source.cameraHandle->GetCameraData());
			editorOnly = source.editorOnly;
		};

		const glm::mat4& GetProjection() const
		{
			return cameraHandle->GetProjection();
		}

		const glm::mat4& GetView() const
		{
			return cameraHandle->GetView();
		}

		const glm::mat4& GetInverseView() const
		{
			return cameraHandle->GetInverseView();
		}

		const glm::vec3 GetPosition() const
		{
			return glm::vec3(cameraHandle->GetInverseView()[3]);
		}

		Rendering::CameraData& GetMutableCameraData() const
		{
			return cameraHandle->GetMutableCameraData();
		}

		Rendering::CameraData GetCameraData() const
		{
			return cameraHandle->GetCameraData();
		}

		void PushCameraData(Rendering::CameraData cameraData)
		{
			cameraHandle->SetCameraData(cameraData);
		}

		void SetAsMainCamera()
		{
			cameraHandle->SetMain();
		}

		Ref<Rendering::CameraHandle> cameraHandle = nullptr;
		bool editorOnly = false;
	};

	struct CTag
	{
		std::string tag = "Untagged";

		CTag() = default;
		CTag(const CTag& other) = default;
		CTag(const std::string& _tag) : tag(_tag) {}

		operator std::string& () { return tag; }
		operator const std::string& () const { return tag; }
	};

	struct CMetadata
	{
		std::string name = "Object";
		bool is_error_entity = false;
		bool is_hidden_entity = false;
		CMetadata() = default;
		CMetadata(const CMetadata&) = default;
		CMetadata(std::string name)
		{
			this->name = name;
		}
		CMetadata(std::string name, bool is_hidden)
		{
			this->name = name;
			is_hidden_entity = is_hidden;
		}
	};

	// Not Component, needs to be moved
	struct CShader
	{
		std::string vertShaderPath;
		std::string fragShaderPath;

		CShader() = default;
		CShader(const CShader&) = default;
	};

	struct CMaterial
	{
		Ref<CShader> shader = nullptr;

		Ref<Texture> diffuse_map = nullptr;
		Ref<Texture> normal_map = nullptr;
		Ref<Texture> ambient_occlusion_map = nullptr;
		Ref<Texture> gloss_map = nullptr;
		std::string shaderFragmentPath;
		bool shaderDriven = false;
		bool shaderValid = false;
		std::map<uint32_t, Ref<Texture>> textureBindings;
		size_t expectedTextureCount = 0;
		std::uint64_t descriptorLayoutVersion = 0;
		struct DescriptorFrame
		{
			VkDescriptorSet set = VK_NULL_HANDLE;
			std::uint64_t layoutVersion = 0;
			std::map<uint32_t, Ref<Texture>> textures;
		};
		std::vector<DescriptorFrame> descriptorFrames;
		Ref<Texture> cached_diffuse_map = nullptr;
		Ref<Texture> cached_normal_map = nullptr;
		Ref<Texture> cached_ambient_occlusion_map = nullptr;
		Ref<Texture> cached_gloss_map = nullptr;

		bool is_custom = false;
		float ax = 1.0f;
		float dx0 = 0.0f;
		float dx1 = 1.0f;
		float dx2 = 0.0f;
		float dx3 = 1.0f;

		CMaterial() = default;
		CMaterial(const CMaterial&) = default;
	};

	struct CMeshRenderer
	{

		CMeshRenderer() = default;

		Ref<StaticMesh> mesh = nullptr;
		UUID materialAsset = UUID("");

		// Update so no dangling pointer
		Ref<CMaterial> material = nullptr; 

		Ref<StaticMesh> GetMesh() const
		{
			return mesh;
		}

		Ref<CMaterial> GetMaterial() const
		{
			return material;
		}
	};

	struct CPointLight
	{
		glm::vec3 color{ 1.0f, 1.0f, 1.0f };
		float radius = 0.5f;
		float intensity = 1.0f;
		float range = 10.0f;
		bool enabled = true;

		CPointLight() = default;
		CPointLight(const CPointLight&) = default;
	};

	struct CDirectionalLight
	{
		glm::vec3 color{ 1.0f };
		float intensity = 1.0f;
		bool enabled = true;
	};

	struct CSpotLight
	{
		glm::vec3 color{ 1.0f };
		float intensity = 1.0f;
		float range = 15.0f;
		float innerAngle = 20.0f;
		float outerAngle = 30.0f;
		bool enabled = true;
	};

	struct CRelationship
	{
		UUID parent = UUID("");
		std::vector<UUID> children;

		CRelationship() = default;
		CRelationship(const CRelationship& other) = default;
		CRelationship(UUID parent) : parent(parent) {}
	};

	struct CTransform
	{
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::quat rotation{ 1.f, 0.0f, 0.0f, 0.0f };

		glm::mat4 TransformMatrix()
		{

			glm::mat4 rotation_matrix
			{
				{
					glm::pow(rotation.w, 2) + glm::pow(rotation.x, 2) - glm::pow(rotation.y, 2) - glm::pow(rotation.z, 2),
					(2 * rotation.x * rotation.y) + (2 * rotation.z * rotation.w),
					(2 * rotation.x * rotation.z) - (2 * rotation.y * rotation.w),
					0.0f
				},
				{
					(2 * rotation.x * rotation.y) - (2 * rotation.z * rotation.w),
					glm::pow(rotation.w, 2) - glm::pow(rotation.x, 2) + glm::pow(rotation.y, 2) - glm::pow(rotation.z, 2),
					(2 * rotation.y * rotation.z) + (2 * rotation.x * rotation.w),
					0.0f
				},
				{
					(2 * rotation.x * rotation.z) + (2 * rotation.y * rotation.w),
					(2 * rotation.y * rotation.z) - (2 * rotation.x * rotation.w),
					glm::pow(rotation.w, 2) - glm::pow(rotation.x, 2) - glm::pow(rotation.y, 2) + glm::pow(rotation.z, 2),
					0.0f
				},
				{
					0.0f,
					0.0f,
					0.0f,
					glm::pow(rotation.w, 2) + glm::pow(rotation.x, 2) + glm::pow(rotation.y, 2) + glm::pow(rotation.z, 2)
				}
			};

			glm::mat4 transformation_matrix = rotation_matrix;
			transformation_matrix[0] = scale.x * transformation_matrix[0];
			transformation_matrix[1] = scale.y * transformation_matrix[1];
			transformation_matrix[2] = scale.z * transformation_matrix[2];
			transformation_matrix[3] = glm::vec4(translation, transformation_matrix[3][3]);

			glm::quat quad = glm::quat_cast(rotation_matrix);
			return transformation_matrix;
		}

		glm::mat4 NormalMatrix()
		{
			glm::mat4 transform_matrix = TransformMatrix();
			glm::mat4 normal_matrix = glm::transpose(glm::inverse(transform_matrix));
			return normal_matrix;
		}

		bool UpdateTransform(const glm::mat4& transform)
		{
			glm::mat4 local_matrix(transform);

			translation = glm::vec3(local_matrix[3]);
			glm::mat4 rotation_matrix = local_matrix;


			scale.x = glm::length(local_matrix[0]);
			rotation_matrix[0] = glm::normalize(local_matrix[0]);
			scale.y = glm::length(local_matrix[1]);
			rotation_matrix[1] = glm::normalize(local_matrix[1]);
			scale.z = glm::length(local_matrix[2]);
			rotation_matrix[2] = glm::normalize(local_matrix[2]);

			rotation = glm::quat_cast(rotation_matrix);

			glm::mat3 shear_matrix = glm::mat3(local_matrix);

			return true;
		}
		CTransform() = default;
		CTransform(const CTransform&) = default;

		operator glm::mat4()
		{
			return TransformMatrix();
		}
	};

	struct CCSharpScript
	{
		UUID script = UUID("");
		std::string className;
	};

	struct CNativeScript
	{

		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*instantiate_script)();
		void (*destroy_script)(CNativeScript*);

		std::function<void(ScriptableEntity*)> onCreate;
		std::function<void(ScriptableEntity*)> onStart;
		std::function<void(ScriptableEntity*)> onUpdate;
		std::function<void(ScriptableEntity*)> onLateUpdate;
		std::function<void(ScriptableEntity*)> onRender;
		std::function<void(ScriptableEntity*)> onDestroy;


		CNativeScript() = default;
		CNativeScript(const CNativeScript&) = default;

		void DestroyInstance()
		{
			if (!Instance) return;
			if (onDestroy) onDestroy(Instance);
			if (destroy_script) destroy_script(this);
			else Instance = nullptr;
		}

		template<typename T>
		void Bind()
		{
			instantiate_script = []() { return static_cast<ScriptableEntity*>(new T()); };
			destroy_script = [](CNativeScript* nsc) { delete (T*)nsc->Instance; nsc->Instance = nullptr; };

			onCreate = [](ScriptableEntity* Instance) { ((T*)Instance)->Create(); };
			onDestroy = [](ScriptableEntity* Instance) { ((T*)Instance)->Destroy(); };
			onStart = [](ScriptableEntity* Instance) { ((T*)Instance)->Start(); };
			onUpdate = [](ScriptableEntity* Instance) { ((T*)Instance)->Update(); };
			onLateUpdate = [](ScriptableEntity* Instance) { ((T*)Instance)->LateUpdate(); };
			onRender = [](ScriptableEntity* Instance) { ((T*)Instance)->Render(); };
		}
	};

	template<typename... Component>
	struct ComponentGroup {};

	using AllComponents =
		ComponentGroup<CTransform, CMeshRenderer, CCamera, CPointLight, CDirectionalLight,
			CSpotLight, CNativeScript, CCSharpScript>;
}
#endif
