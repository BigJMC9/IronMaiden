#include "maidenpch.hpp"
#include "H_Camera.hpp"

namespace Madam {

	Rendering::CameraHandle* Rendering::CameraHandle::instance = new Rendering::CameraHandle();
	bool Rendering::CameraHandle::hasInitialized = false;

	namespace Rendering {
		void CameraHandle::setProjection() {
			switch (cameraData.projectionType)
			{
			case CameraData::ProjectionType::Perspective:
			{
				MADAM_CORE_ASSERT(glm::abs(cameraData.perspective.aspect - std::numeric_limits<float>::epsilon()) > 0.0f, "Camera Perspective aspect ratio is <= 0.0f");
				float tanHalfFovy = tan(cameraData.perspective.fovy / 2.f);
				projectionMatrix = glm::mat4{ 0.0f };
				projectionMatrix[0][0] = 1.f / (cameraData.perspective.aspect * tanHalfFovy);
				projectionMatrix[1][1] = 1.f / (tanHalfFovy);
				projectionMatrix[2][2] = cameraData.perspective.far / (cameraData.perspective.far - cameraData.perspective.near);
				projectionMatrix[2][3] = 1.f;
				projectionMatrix[3][2] = -(cameraData.perspective.far * cameraData.perspective.near) / (cameraData.perspective.far - cameraData.perspective.near);
			}
			break;
			case CameraData::ProjectionType::Orthographic:
			{
				projectionMatrix = glm::mat4{ 1.0f };
				projectionMatrix[0][0] = 2.f / (cameraData.orthographic.size * cameraData.orthographic.aspect);
				projectionMatrix[1][1] = 2.f / (-cameraData.orthographic.size);
				projectionMatrix[2][2] = 1.f / (cameraData.orthographic.far - cameraData.orthographic.near);
				projectionMatrix[3][0] = 0;
				projectionMatrix[3][1] = 0;
				projectionMatrix[3][2] = -cameraData.orthographic.near / (cameraData.orthographic.far - cameraData.orthographic.near);
			}
			break;
			
			case CameraData::ProjectionType::None:
				MADAM_ERROR("Camera projection type not set");
			break;
			default:
				break;
			}
		}

		void CameraHandle::cameraBehaviour() {
			
		}
	}
}