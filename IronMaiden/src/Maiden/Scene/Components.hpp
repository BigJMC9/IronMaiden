#pragma once

#include "maidenpch.hpp"
#include "../Core/H_Utils.hpp"
#include "H_Model.hpp"
#include "H_Texture.hpp"
#include "ScriptableEntity.hpp"
#include "../Rendering/H_Camera.hpp"
#include "../Math/Math.hpp"


namespace Madam {

	struct Camera {
		//Shared pointer to camera data?
		//Rendering::CameraData cameraData = Rendering::CameraData();
		Ref<Rendering::CameraHandle> cameraHandle;

		const glm::mat4& getProjection() const {
			return cameraHandle->getProjection();
		}

		const glm::mat4& getView() const {
			return cameraHandle->getView();
		}

		const glm::mat4& getInverseView() const {
			return cameraHandle->getInverseView();
		}

		const glm::vec3 getPosition() const {
			return glm::vec3(cameraHandle->getInverseView()[3]);
		}

		//Returns reference to camera data
		Rendering::CameraData& CameraData() {
			return cameraHandle->CameraData();
		}

		//Returns copy of camera data
		Rendering::CameraData getCameraData() {
			return cameraHandle->CameraData();
		}

		void pushCameraData(Rendering::CameraData cameraData) {
			cameraHandle->setCameraData(cameraData);
		}

		void setAsMainCamera() {
			cameraHandle->setMain();
		}

		Camera(Rendering::CameraData _cameraData) {
			cameraHandle = std::make_shared<Rendering::CameraHandle>(_cameraData);
		};
		Camera() {
			Rendering::CameraData cameraData;
			cameraHandle = std::make_shared<Rendering::CameraHandle>(cameraData);
		};
		//Camera(Camera&) = default;
		Camera(const Camera& source) {
			cameraHandle = std::make_shared<Rendering::CameraHandle>(source.cameraHandle->CameraData());
		};
	};

	struct UniqueIdentifier {
		UUID uuid = UUID();

		UniqueIdentifier() = default;
		UniqueIdentifier(UUID _uuid) {
			uuid = _uuid;
		};
	};

	struct Tag {
		std::string tag = "Untagged";

		Tag() = default;
		Tag(const Tag& other) = default;
		Tag(const std::string& _tag)
			: tag(_tag) {}

		operator std::string& () { return tag; }
		operator const std::string& () const { return tag; }
	};

	struct GameObject {
		std::string name = "Object";

		GameObject() = default;
		//Object(Object&) = default;
		GameObject(const GameObject&) = default;
		GameObject(std::string _name) {
			name = _name;
		}
		//Parent
		//Tag
		//Layer
		//Icon
	};

	struct MaidenInternal {
		std::string name = "Object";
	};

	struct MeshFilter {
		Ref<Model> model;

		MeshFilter() = default;
		//MeshFilter(MeshFilter&) = default;
		MeshFilter(const MeshFilter&) = default;
	};

	//Not Component, needs to be moved
	struct Shader {
		std::string vertShaderPath;
		std::string fragShaderPath;

		Shader() = default;
		//Shader(Shader&) = default;
		Shader(const Shader&) = default;
	};

	struct Material {
		Ref<Shader> shader = nullptr;

		Ref<Texture> diffuseMap = nullptr;
		Ref<Texture> normalMap = nullptr;
		Ref<Texture> ambientOcclusionMap = nullptr;
		Ref<Texture> glossMap = nullptr;

		Material() = default;
		//Material(Material&) = default;
		Material(const Material&) = default;
	};

	struct MeshRenderer {

		MeshFilter mesh;
		Ref<Material> material = nullptr;

		MeshRenderer() = default;
		//MeshRenderer(MeshRenderer&) = default;
		MeshRenderer(const MeshRenderer&) = default;

		Ref<Model> getModel() {
			return mesh.model;
		}

		Ref<Material> getMaterial() {
			return material;
		}
	};

	struct PointLight {
		glm::vec3 color;
		float radius = 5.0f;
		float intensity = 1.0f;

		PointLight() = default;
		//PointLight(PointLight&) = default;
		PointLight(const PointLight&) = default;
	};
	using namespace Madam::Math;
	//Add Quaternion as rotation
	struct Transform {
		Vector3 translation{}; // (position offset)
		Vector3 scale{ 1.f, 1.f, 1.f };
		Quaternion rotation{};

		glm::mat4 transformMatrix() {
			return translate(glm::mat4(1.0f), (glm::vec3)translation) *
				toMat4(rotation.value) *
				glm::scale(glm::mat4(1.0f), (glm::vec3)scale);
		}

		glm::mat3 normalMatrix() {
			return glm::mat3(glm::transpose(glm::inverse(transformMatrix())));
		}


		Transform() = default;
		//Transform(Transform&) = default;
		Transform(const Transform&) = default;
		//Transform(const glm::mat4& transform) : transformMatrix(transform) {}

		operator glm::mat4() {
			return transformMatrix();
		}

		void UpdateTransform(const glm::mat4& transform) {
			DecomposeTransformMatrix(transform, translation, rotation, scale);
		}

		Transform& operator=(const glm::mat4& transform) {
			DecomposeTransformMatrix(transform, translation, rotation, scale);
			return *this;
		}
		//operator const glm::mat4& () const { return transformMatrix; }
	};

	//Maybe set default functions? virtual functions may need to be avoided
	struct NativeScriptComponent{

		ScriptableEntity* Instance = nullptr;


		//std::function<void()> InstantiateScript;
		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);
		//std::function<void()> DestroyScript;

		std::function<void(ScriptableEntity*)> onCreate;
		std::function<void(ScriptableEntity*)> onStart;
		std::function<void(ScriptableEntity*)> onUpdate;
		std::function<void(ScriptableEntity*)> onLateUpdate;
		std::function<void(ScriptableEntity*)> onRender;
		std::function<void(ScriptableEntity*)> onDestroy;
		

		NativeScriptComponent() = default;
		NativeScriptComponent(const NativeScriptComponent&) = default;
		//NativeScriptComponent(const NativeScriptComponent&) = default;

		//Bind ?!?! How do we work this with dll?
		//Pain （πーπ）
		//At this point I wonder if C# monobehaviour would be better for this (╥﹏╥)
		template<typename T>
		void Bind() {
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete (T*)nsc->Instance; nsc->Instance = nullptr; };

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
		ComponentGroup<Transform, MeshRenderer,
		MeshFilter, Camera, PointLight, NativeScriptComponent>;
}