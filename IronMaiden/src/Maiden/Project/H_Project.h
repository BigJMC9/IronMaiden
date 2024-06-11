#pragma once
#include "maidenpch.hpp"
#include "../Core/Main/Core.hpp"
#include "../Core/H_Application.hpp"
#include <filesystem>

namespace Madam
{

	const std::filesystem::path DEFAULT_PROJECT_DIRECTORIES[] =
	{
		"Assets",
		"Library",
		"Resources",
		"Logs"
	};
	namespace Asset {
		class AssetManager;
	}

	struct ProjectInfo
	{
	public:
		ProjectInfo() = default;
		std::string projectName = "";
		std::string projectVersion = "";
		std::string projectAuthor = "";
		std::filesystem::path assetMetaPath = "";
		std::filesystem::path currentScene = "";
		std::filesystem::path projectsDirectory = "";
		// Settings

	};

	typedef ProjectInfo ProjectConfig;

	class MADAM_API Project {
	private:
		Project(std::filesystem::path projectDirectory);
		~Project();
	public:
		static Project& Get();
		static void clear();

		static bool createProject(ProjectConfig config);
		static bool loadProject(std::filesystem::path projectDirectory);
		static bool newProject(ProjectConfig config);
		static bool saveProject();
		Asset::AssetManager& getAssetManager() const;
		ProjectInfo getProjectInfo() const;
		std::filesystem::path getProjectDirectory() const { return projectDirectory; }
		std::vector<std::filesystem::path> getProjectDirectories() const 
		{
			if (projectDirectory.empty())
			{
				return {};
			}
			std::vector<std::filesystem::path> temp;
			for (const std::filesystem::path& path : DEFAULT_PROJECT_DIRECTORIES)
			{
				temp.push_back(projectDirectory / path);
			}
			return temp;
		}
		std::filesystem::path getResourcesDirectory() const 
		{
			std::filesystem::path temp = projectDirectory / "Resources";
			return temp; 
		}

	private:
		static Project* instance;
		static bool instanceFlag;

		std::filesystem::path projectDirectory;
		ProjectInfo projectInfo;
		Asset::AssetManager* assetManager = nullptr;
	};
}