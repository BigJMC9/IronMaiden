#pragma once

#include "maidenpch.hpp"
#include "../Core/H_CoreUtils.hpp"
#include "../Core/H_Logger.hpp"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
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
				glm::quat rotation = glm::quatLookAt(glm::normalize(direction), up);

				// Create the rotation and translation matrices
				glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
				glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

				// Combine them to form the view matrix
				viewMatrix = glm::inverse(translationMatrix * rotationMatrix);

				// For the inverse view matrix, transpose the view matrix (since it's orthonormal)
				inverseViewMatrix = glm::inverse(viewMatrix);
			}

			void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f }) {
				MADAM_CORE_ASSERT(((target - position).x != .0f && (target - position).y != .0f && (target - position).z != .0f), "Direction provided is zero!");
				SetViewDirection(position, target - position, up);
			}

			void SetViewYXZ(glm::vec3 position, glm::quat rotation) {

				glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
				glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
				viewMatrix = glm::inverse(translationMatrix * rotationMatrix);

				inverseViewMatrix = glm::inverse(viewMatrix);
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