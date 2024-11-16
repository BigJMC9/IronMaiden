#pragma once

#include "maidenpch.hpp"

<<<<<<< HEAD
#define INCLUDE_UTILS
#define INCLUDE_LOGGER
#define INCLUDE_GLM
#include "../Utils/H_Utils.h"

=======
#include "glm/glm.hpp"
>>>>>>> parent of d44d5be (Added Skybox.)
#ifdef near
	#undef near
#endif
#ifdef far
	#undef far
#endif


namespace Madam {

	namespace Rendering {
		struct MADAM_API CameraData {
			struct Perspective {
				Perspective(float _fovy = glm::radians(50.0f), float _aspect = (16 / 9), float _near = 0.1f, float _far = 100.0f) : fovy(_fovy), aspect(_aspect), near(_near), far(_far) {}
				float fovy;
				float aspect;
				float near;
				float far;
			};
			struct Orthographic {
				Orthographic(float _size = 50, float _aspect = (16 / 9), float _near = 0.1f, float _far = 100.0f) : size(_size), aspect(_aspect), near(_near), far(_far) {}
				float size;
				float aspect;
				float near;
				float far;
			};

			enum class ProjectionType {
				None = 0,
				Perspective = 1,
				Orthographic = 2
			};

			ProjectionType projectionType = ProjectionType::None;

			Perspective perspective;
			Orthographic orthographic;

		};

		class MADAM_API CameraHandle {
		public:
			CameraHandle(CameraData cameraData) : _cameraData{ cameraData } {
				
			}
			~CameraHandle()
			{
				if (IsMain())
				{
					MADAM_INFO("Deleting main camera");
					instance = nullptr;
					hasInitialized = false;
				}
			}

			void SetCameraData(CameraData cameraData) {
				_cameraData = cameraData;
			}

			void SetMain() {
				hasInitialized = true;
				instance = this;
			}

			CameraData GetCameraData() {
				return _cameraData;
			}

			CameraData& GetMutableCameraData() {
				return _cameraData;
			}

			CameraHandle& Get() {
				return *this;
			}

			static void SetDefaultCameraData(CameraData cameraData) {
				defaultCameraData = cameraData;
			}

			static CameraData GetDefaultCameraData() {
				return defaultCameraData;
			}

			static CameraHandle& GetMain();

			bool IsMain() {
				if (instance == this) {
					return true;
				}
				else {
					return false;
				}
			}

			const glm::mat4& GetProjection() const {
				return projectionMatrix;
			}

			const glm::mat4& GetView() const {
				return viewMatrix;
			}

			const glm::mat4& GetInverseView() const {
				return inverseViewMatrix;
			}

			const glm::vec3 GetPosition() const {
				return glm::vec3(inverseViewMatrix[3]);
			}

			void SetProjection();

			void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f }) {
				const glm::vec3 w{ glm::normalize(direction) };
				const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
				const glm::vec3 v{ glm::cross(w, u) };

				viewMatrix = glm::mat4{ 1.f };
				viewMatrix[0][0] = u.x;
				viewMatrix[1][0] = u.y;
				viewMatrix[2][0] = u.z;
				viewMatrix[0][1] = v.x;
				viewMatrix[1][1] = v.y;
				viewMatrix[2][1] = v.z;
				viewMatrix[0][2] = w.x;
				viewMatrix[1][2] = w.y;
				viewMatrix[2][2] = w.z;
				viewMatrix[3][0] = -glm::dot(u, position);
				viewMatrix[3][1] = -glm::dot(v, position);
				viewMatrix[3][2] = -glm::dot(w, position);

				inverseViewMatrix = glm::mat4{ 1.f };
				inverseViewMatrix[0][0] = u.x;
				inverseViewMatrix[0][1] = u.y;
				inverseViewMatrix[0][2] = u.z;
				inverseViewMatrix[1][0] = v.x;
				inverseViewMatrix[1][1] = v.y;
				inverseViewMatrix[1][2] = v.z;
				inverseViewMatrix[2][0] = w.x;
				inverseViewMatrix[2][1] = w.y;
				inverseViewMatrix[2][2] = w.z;
				inverseViewMatrix[3][0] = position.x;
				inverseViewMatrix[3][1] = position.y;
				inverseViewMatrix[3][2] = position.z;
			}

			void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f }) {
				MADAM_CORE_ASSERT(((target - position).x != .0f && (target - position).y != .0f && (target - position).z != .0f), "Direction provided is zero!");
				SetViewDirection(position, target - position, up);
			}

			void SetViewYXZ(glm::vec3 position, glm::vec3 rotation) {
				const float c3 = glm::cos(rotation.z);
				const float s3 = glm::sin(rotation.z);
				const float c2 = glm::cos(rotation.x);
				const float s2 = glm::sin(rotation.x);
				const float c1 = glm::cos(rotation.y);
				const float s1 = glm::sin(rotation.y);
				const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
				const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
				const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
				viewMatrix = glm::mat4{ 1.f };
				viewMatrix[0][0] = u.x;
				viewMatrix[1][0] = u.y;
				viewMatrix[2][0] = u.z;
				viewMatrix[0][1] = v.x;
				viewMatrix[1][1] = v.y;
				viewMatrix[2][1] = v.z;
				viewMatrix[0][2] = w.x;
				viewMatrix[1][2] = w.y;
				viewMatrix[2][2] = w.z;
				viewMatrix[3][0] = -glm::dot(u, position);
				viewMatrix[3][1] = -glm::dot(v, position);
				viewMatrix[3][2] = -glm::dot(w, position);

				inverseViewMatrix = glm::mat4{ 1.f };
				inverseViewMatrix[0][0] = u.x;
				inverseViewMatrix[0][1] = u.y;
				inverseViewMatrix[0][2] = u.z;
				inverseViewMatrix[1][0] = v.x;
				inverseViewMatrix[1][1] = v.y;
				inverseViewMatrix[1][2] = v.z;
				inverseViewMatrix[2][0] = w.x;
				inverseViewMatrix[2][1] = w.y;
				inverseViewMatrix[2][2] = w.z;
				inverseViewMatrix[3][0] = position.x;
				inverseViewMatrix[3][1] = position.y;
				inverseViewMatrix[3][2] = position.z;
			}

			bool operator==(const CameraHandle& other) const {
				return this == &other;
			}

		private:
			CameraHandle() {

			}
			static CameraHandle* instance; //for main camera
			static bool hasInitialized;
			static CameraData defaultCameraData;
			Rendering::CameraData _cameraData;

			glm::mat4 viewMatrix{ 1.f };
			glm::mat4 projectionMatrix{ 1.f };
			glm::mat4 inverseViewMatrix{ 1.f };
		};	
	}
}