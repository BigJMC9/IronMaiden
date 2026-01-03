#pragma once

#include "maidenpch.hpp"
#include "../Core/H_Logger.hpp"
#include "../Events/H_EventSystem.h"
#include "H_Scene.hpp"

#include <filesystem>

namespace Madam
{
	class SceneSerializer;

	class MADAM_API SceneManager
	{
	public:
		SceneManager() = default;

		Scene& GetActiveScene()
		{
			MADAM_CORE_ASSERT(m_activeScene, "Active scene is null");
			return *m_activeScene;
		}

		Ref<Scene>& GetActiveSceneRef() { return m_activeScene; }
		const Ref<Scene>& GetActiveSceneRef() const { return m_activeScene; }

		bool HasActiveScene() const { return m_activeScene != nullptr; }

		void SetActiveScene(Ref<Scene> scene)
		{
			m_activeScene = std::move(scene);
		}

		void SwitchScene(Ref<Scene> scene)
		{
			m_activeScene = std::move(scene);
			SceneChangeEvent e;
			Events::EventSystem::Get().PushEvent(&e, true);
		}

		IrmResult LoadScene(const std::filesystem::path& file_path);
		IrmResult SaveScene(const std::filesystem::path& file_path) const;

	private:
		Ref<Scene> m_activeScene = nullptr;
	};
}
