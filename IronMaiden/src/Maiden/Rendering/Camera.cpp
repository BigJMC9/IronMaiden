#include "maidenpch.hpp"
#include "H_Camera.hpp"
#include "../Core/H_Application.hpp"

namespace Madam {

	Rendering::CameraHandle* Rendering::CameraHandle::instance = new Rendering::CameraHandle();
	bool Rendering::CameraHandle::hasInitialized = false;
	Rendering::CameraData Rendering::CameraHandle::defaultCameraData = CameraData
	{
		Rendering::CameraData::ProjectionType::Perspective,
		Rendering::CameraData::Perspective(glm::radians(50.0f), static_cast<float>(1920)/1080, 0.1f, 1000.0f)
	};

	namespace Rendering {
		void CameraHandle::SetProjection() {
			switch (_cameraData.projectionType)
			{
			case CameraData::ProjectionType::Perspective:
			{
				MADAM_CORE_ASSERT(glm::abs(_cameraData.perspective.aspect - std::numeric_limits<float>::epsilon()) > 0.0f, "Camera Perspective aspect ratio is <= 0.0f");
				float tanHalfFovy = tan(_cameraData.perspective.fovy / 2.f);
				projectionMatrix = glm::mat4{ 0.0f };
				projectionMatrix[0][0] = 1.f / (_cameraData.perspective.aspect * tanHalfFovy);
				projectionMatrix[1][1] = 1.f / (tanHalfFovy);
				projectionMatrix[2][2] = _cameraData.perspective.far / (_cameraData.perspective.far - _cameraData.perspective.near);
				projectionMatrix[2][3] = 1.f;
				projectionMatrix[3][2] = -(_cameraData.perspective.far * _cameraData.perspective.near) / (_cameraData.perspective.far - _cameraData.perspective.near);
			}
			break;
			case CameraData::ProjectionType::Orthographic:
			{
				projectionMatrix = glm::mat4{ 1.0f };
				projectionMatrix[0][0] = 2.f / (_cameraData.orthographic.size * _cameraData.orthographic.aspect);
				projectionMatrix[1][1] = 2.f / (-_cameraData.orthographic.size);
				projectionMatrix[2][2] = 1.f / (_cameraData.orthographic.far - _cameraData.orthographic.near);
				projectionMatrix[3][0] = 0;
				projectionMatrix[3][1] = 0;
				projectionMatrix[3][2] = -_cameraData.orthographic.near / (_cameraData.orthographic.far - _cameraData.orthographic.near);
			}
			break;
			
			case CameraData::ProjectionType::None:
				MADAM_ERROR("Camera projection type not set");
			break;
			default:
				break;
			}
		}

		CameraHandle& CameraHandle::GetMain()
		{
			if (!hasInitialized) {
				MADAM_CORE_INFO("Get Main called, instance not initialized!");
				Entity entity = Application::Get().getScene().CreateEntity("Editor Camera");
				entity.GetComponent<CTransform>().translation.z = -2.5f;
				entity.AddComponent<CCamera>(defaultCameraData);
				entity.GetComponent<CCamera>().cameraHandle->SetViewYXZ(entity.GetComponent<CTransform>().translation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
				entity.GetComponent<CCamera>().cameraHandle->SetProjection();
				entity.GetComponent<CCamera>().cameraHandle->SetMain();
			}
			return *instance;
		}
	}
}