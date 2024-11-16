#pragma once
#ifndef H_COMPONENTS
#define H_COMPONENTS

#include "maidenpch.hpp"
#include "../Rendering/H_Mesh.h"
#include "../Rendering/H_Texture.h"
#include "ScriptableEntity.hpp"
#include "../Rendering/H_Camera.hpp"

//libs
#define INCLUDE_GLM
#include "../Utils/H_Utils.h"

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
		glm::vec3 color;
		float radius = 5.0f;
		float intensity = 1.0f;

		CPointLight() = default;
		//PointLight(PointLight&) = default;
		CPointLight(const CPointLight&) = default;
	};

	struct CTransform {
		//GENERATE_COMPONENT()
		glm::vec3 translation{}; // (position offset)
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 transform() {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{translation.x, translation.y, translation.z, 1.0f}
			};
		}

		glm::mat3 normalMatrix() {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			const glm::vec3 invScale = 1.0f / scale;

			return glm::mat3{
				{
					invScale.x * (c1 * c3 + s1 * s2 * s3),
					invScale.x * (c2 * s3),
					invScale.x * (c1 * s2 * s3 - c3 * s1),
				},
				{
					invScale.y * (c3 * s1 * s2 - c1 * s3),
					invScale.y * (c2 * c3),
					invScale.y * (c1 * c3 * s2 + s1 * s3),
				},
				{
					invScale.z * (c2 * s1),
					invScale.z * (-s2),
					invScale.z * (c1 * c2),
				},
			};
		}

		bool UpdateTransform (const glm::mat4& transform) {

			using namespace glm;
			using T = float;

			mat4 localMatrix(transform);

			// Normalize the matrix.
			if (epsilonEqual(localMatrix[3][3], static_cast<float>(0), epsilon<T>()))
				return false;

			MADAM_CORE_ASSERT(epsilonEqual(localMatrix[3][3], static_cast<T>(1), static_cast<T>(0.00001)), "");

			MADAM_CORE_ASSERT(
				epsilonEqual(localMatrix[0][3], static_cast<T>(0), epsilon<T>()) &&
				epsilonEqual(localMatrix[1][3], static_cast<T>(0), epsilon<T>()) &&
				epsilonEqual(localMatrix[2][3], static_cast<T>(0), epsilon<T>()), "");

			// Next take care of translation (easy).
			translation = vec3(localMatrix[3]);
			localMatrix[3] = vec4(0, 0, 0, localMatrix[3].w);

#if 0
			vec3 Pdum3;
#endif
			vec3 row[3];


			// Now get scale and shear.
			for (length_t i = 0; i < 3; ++i)
				for (length_t j = 0; j < 3; ++j)
					row[i][j] = localMatrix[i][j];

			// Compute X scale factor and normalize first row.
			scale.x = length(row[0]);
			row[0] = detail::scale(row[0], static_cast<T>(1));
			scale.y = length(row[1]);
			row[1] = detail::scale(row[1], static_cast<T>(1));
			scale.z = length(row[2]);
			row[2] = detail::scale(row[2], static_cast<T>(1));

			// At this point, the matrix (in rows[]) is orthonormal.
			// Check for a coordinate system flip.  If the determinant
			// is -1, then negate the matrix and the scaling factors.
#if 0
			Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
			if (dot(Row[0], Pdum3) < 0)
			{
				for (length_t i = 0; i < 3; i++)
				{
					scale[i] *= static_cast<T>(-1);
					Row[i] *= static_cast<T>(-1);
				}
			}
#endif

			rotation.y = asin(-row[0][2]);
			if (cos(rotation.y) != 0) {
				rotation.x = atan2(row[1][2], row[2][2]);
				rotation.z = atan2(row[0][1], row[0][0]);
			}
			else {
				rotation.x = atan2(-row[2][0], row[1][1]);
				rotation.z = 0;
			}


			return true;
		}

		CTransform() = default;
		//Transform(Transform&) = default;
		CTransform(const CTransform&) = default;
		//Transform(const glm::mat4& transform) : m_transform(transform) {}

		operator glm::mat4() {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{translation.x, translation.y, translation.z, 1.0f}
			};
		}
		//operator const glm::mat4& () const { return m_transform; }
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