#pragma once
#ifndef H_COMPONENTS
#define H_COMPONENTS

#include "maidenpch.hpp"
#include "../Rendering/H_Mesh.h"
#include "../Rendering/H_Texture.h"
#include "ScriptableEntity.hpp"
#include "../Rendering/H_Camera.hpp"

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

/*#define GENERATE_COMPONENT(...) Entity entity; \
void SetComponentParams(Entity _entity) { entity = _entity; }*/



namespace Madam {

	struct CUniqueIdentifier {
		//GENERATE_COMPONENT()
		UUID uuid = UUID();

		CUniqueIdentifier() = default;
		CUniqueIdentifier(UUID _uuid) {
			uuid = _uuid;
		};
	};

	struct CCamera {
		//GENERATE_COMPONENT()
		CCamera() {
			Rendering::CameraData _cameraData = Rendering::CameraHandle::GetDefaultCameraData();
			cameraHandle = CreateRef<Rendering::CameraHandle>(_cameraData);
		};
		CCamera(Rendering::CameraData _cameraData) {
			cameraHandle = CreateRef<Rendering::CameraHandle>(_cameraData);
		};
		CCamera(const CCamera& source) {
			cameraHandle = CreateRef<Rendering::CameraHandle>(source.cameraHandle->GetCameraData());
		};

		const glm::mat4& GetProjection() const {
			return cameraHandle->GetProjection();
		}

		const glm::mat4& GetView() const {
			return cameraHandle->GetView();
		}

		const glm::mat4& GetInverseView() const {
			return cameraHandle->GetInverseView();
		}

		const glm::vec3 GetPosition() const {
			return glm::vec3(cameraHandle->GetInverseView()[3]);
		}

		//Returns reference to camera data
		Rendering::CameraData& GetMutableCameraData() {
			return cameraHandle->GetMutableCameraData();
		}

		//Returns copy of camera data
		Rendering::CameraData GetCameraData() {
			return cameraHandle->GetCameraData();
		}

		void PushCameraData(Rendering::CameraData cameraData) {
			cameraHandle->SetCameraData(cameraData);
		}

		void SetAsMainCamera() {
			cameraHandle->SetMain();
		}

		Ref<Rendering::CameraHandle> cameraHandle = nullptr;
	};

	struct CTag {
		//GENERATE_COMPONENT()
		std::string tag = "Untagged";

		CTag() = default;
		CTag(const CTag& other) = default;
		CTag(const std::string& _tag)
			: tag(_tag) {}

		operator std::string& () { return tag; }
		operator const std::string& () const { return tag; }
	};

	struct CMetadata {
		//GENERATE_COMPONENT()
		std::string name = "Object";
		CMetadata() = default;
		CMetadata(const CMetadata&) = default;
		CMetadata(std::string _name) {
			name = _name;
		}
		//Parent
		//Tag
		//Layer
		//Icon
	};

	//Not Component, needs to be moved
	struct CShader {
		std::string vertShaderPath;
		std::string fragShaderPath;

		CShader() = default;
		//Shader(Shader&) = default;
		CShader(const CShader&) = default;
	};
	//Remove
	struct CMaterial {
		Ref<CShader> shader = nullptr;

		Ref<Texture> diffuseMap = nullptr;
		Ref<Texture> normalMap = nullptr;
		Ref<Texture> ambientOcclusionMap = nullptr;
		Ref<Texture> glossMap = nullptr;

		CMaterial() = default;
		//Material(Material&) = default;
		CMaterial(const CMaterial&) = default;
	};

	struct CMeshRenderer {
		//GENERATE_COMPONENT()

		CMeshRenderer() = default;

		Ref<StaticMesh> mesh = nullptr;
		Ref<CMaterial> material = nullptr;

		Ref<StaticMesh> GetMesh() {
			return mesh;
		}

		Ref<CMaterial> GetMaterial() {
			return material;
		}
	};

	struct CPointLight {
		//GENERATE_COMPONENT()
		glm::vec3 color{ 1.0f, 1.0f, 1.0f };
		float radius = 5.0f;
		float intensity = 1.0f;

		CPointLight() = default;
		//PointLight(PointLight&) = default;
		CPointLight(const CPointLight&) = default;
	};

	struct CRelationship {
		UUID parent = UUID("");
		std::vector<UUID> children;

		CRelationship() = default;
		CRelationship(const CRelationship& other) = default;
		CRelationship(UUID parent) : parent(parent) {}
	};

	struct CTransform {
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::quat rotation{ 1.f, 0.0f, 0.0f, 0.0f };

		glm::mat4 transform() {

			glm::mat4 rotationMatrix
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

			glm::mat4 transformationMatrix = rotationMatrix;
			transformationMatrix[0] = scale.x * transformationMatrix[0];
			transformationMatrix[1] = scale.y * transformationMatrix[1];
			transformationMatrix[2] = scale.z * transformationMatrix[2];
			transformationMatrix[3] = glm::vec4(translation, transformationMatrix[3][3]);

			glm::quat quad = glm::quat_cast(rotationMatrix);
			return transformationMatrix;
		}

		glm::mat4 normalMatrix() {
			glm::mat4 transformMatrix = transform();
			glm::mat4 normalMatrix = glm::transpose(glm::inverse(transformMatrix));
			return normalMatrix;
		}

		bool UpdateTransform (const glm::mat4& transform) {
			glm::mat4 localMatrix(transform);

			/*if (glm::epsilonEqual(localMatrix[3][3], static_cast<float>(0), glm::epsilon<float>()))
				return false;

			MADAM_CORE_ASSERT(glm::epsilonEqual(localMatrix[3][3], static_cast<float>(1), static_cast<float>(0.00001)), "");

			MADAM_CORE_ASSERT(
				glm::epsilonEqual(localMatrix[0][3], static_cast<float>(0), glm::epsilon<float>()) &&
				glm::epsilonEqual(localMatrix[1][3], static_cast<float>(0), glm::epsilon<float>()) &&
				glm::epsilonEqual(localMatrix[2][3], static_cast<float>(0), glm::epsilon<float>()), "");*/
			translation = glm::vec3(localMatrix[3]);
			glm::mat4 rotationMatrix = localMatrix;
			
			
			scale.x = glm::length(localMatrix[0]);
			rotationMatrix[0] = glm::normalize(localMatrix[0]);
			scale.y = glm::length(localMatrix[1]);
			rotationMatrix[1] = glm::normalize(localMatrix[1]);
			scale.z = glm::length(localMatrix[2]);
			rotationMatrix[2] = glm::normalize(localMatrix[2]);

			rotation = glm::quat_cast(rotationMatrix);

			glm::mat3 shearMatrix = glm::mat3(localMatrix);

			return true;
		}
		CTransform() = default;
		CTransform(const CTransform&) = default;

		operator glm::mat4() {
			return transform();
		}
	};

	//Maybe set default functions? virtual functions may need to be avoided
	struct CNativeScript{
		//GENERATE_COMPONENT()
		ScriptableEntity* Instance = nullptr;


		//std::function<void()> InstantiateScript;
		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(CNativeScript*);
		//std::function<void()> DestroyScript;

		std::function<void(ScriptableEntity*)> onCreate;
		std::function<void(ScriptableEntity*)> onStart;
		std::function<void(ScriptableEntity*)> onUpdate;
		std::function<void(ScriptableEntity*)> onLateUpdate;
		std::function<void(ScriptableEntity*)> onRender;
		std::function<void(ScriptableEntity*)> onDestroy;
		

		CNativeScript() = default;
		CNativeScript(const CNativeScript&) = default;
		//NativeScriptComponent(const NativeScriptComponent&) = default;

		//Bind ?!?! How do we work this with dll?
		//Pain （πーπ）
		//At this point I wonder if C# monobehaviour would be better for this (╥﹏╥)
		template<typename T>
		void Bind() {
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](CNativeScript* nsc) { delete (T*)nsc->Instance; nsc->Instance = nullptr; };

			onCreate = [](ScriptableEntity* Instance) { ((T*)Instance)->Create(); };
			onDestroy = [](ScriptableEntity* Instance) { ((T*)Instance)->Destroy(); };
			onStart = [](ScriptableEntity* Instance) { ((T*)Instance)->Start(); };
			onUpdate = [](ScriptableEntity* Instance) { ((T*)Instance)->Update(); };
			onLateUpdate = [](ScriptableEntity* Instance) { ((T*)Instance)->LateUpdate(); };
			onRender = [](ScriptableEntity* Instance) { ((T*)Instance)->Render(); };
		}
	};

	template<typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents =
		ComponentGroup<CTransform, CMeshRenderer, CCamera, CPointLight, CNativeScript>;
}
#endif