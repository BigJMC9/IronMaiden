#include "maidenpch.hpp"
#include "H_SceneManager.hpp"
#include "H_SceneSerializer.hpp"

namespace Madam
{
	IrmResult SceneManager::LoadScene(const std::filesystem::path& file_path)
	{
		Ref<Scene> new_scene = CreateRef<Scene>();
		IrmResult result = SceneSerializer::Deserialize(file_path, new_scene);
		if (result == IRM_SUCCESS)
		{
			SwitchScene(new_scene);
		}
		return result;
	}

	IrmResult SceneManager::SaveScene(const std::filesystem::path& file_path) const
	{
		if (!m_activeScene)
		{
			return IRM_ERROR_INVALID_PARAM;
		}
		return SceneSerializer::Serialize(file_path, m_activeScene);
	}
}
