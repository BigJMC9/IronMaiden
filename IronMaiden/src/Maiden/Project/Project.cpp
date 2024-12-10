#include "maidenpch.hpp"
#include "H_Project.h"
#include "../Asset/H_AssetSystem.h"
#include "../Core/H_Logger.hpp"
#include "../Platform/Platforms.hpp"
#include <fstream>

namespace Madam
{
	Project* Project::instance = nullptr;
	bool Project::instanceFlag = false;

	Project::Project(std::filesystem::path projectDirectory) : projectDirectory(projectDirectory)
	{
		std::ifstream confFile;
		if (!Platform::OpenFile(confFile, projectDirectory / "ProjectInfo.conf"))
		{
			return;
		}

		std::string line;
		while (std::getline(confFile, line))
		{
			std::string key = line.substr(0, line.find(':'));
			std::string value = line.substr(line.find(':') + 2);
			if (key == "ProjectName")
			{
				projectInfo.projectName = value;
			}
			else if (key == "ProjectVersion")
			{
				projectInfo.projectVersion = value;
			}
			else if (key == "ProjectAuthor")
			{
				projectInfo.projectAuthor = value;
			}
			else if (key == "AssetPackLocation")
			{
				projectInfo.assetMetaPath = std::filesystem::u8path(value);
			}
		}
		confFile.close();

		clear();
		instanceFlag = true;
		instance = this;
		init();
	}

	Project::~Project()
	{
		delete assetManager;
	}

	void Project::init()
	{
		assetManager = new AssetManager(projectDirectory);
	}

	Project& Project::Get()
	{
		if (!instanceFlag)
		{
			clear();
			new Project(Application::Get().GetConfig().projectsDirectory / "temp");
		}
		return *instance;
	}

	void Project::clear()
	{
		if (instanceFlag || instance != nullptr)
		{
			instanceFlag = false;
			delete instance;
			instance = nullptr;
		}
	}

	bool Project::createProject(ProjectConfig config)
	{
		std::filesystem::path projectConfigDirectory;
		projectConfigDirectory = config.projectsDirectory;
		projectConfigDirectory /= config.projectName;
		if (std::filesystem::exists(projectConfigDirectory))
		{
			MADAM_CORE_ERROR("Project Directory already exists: ");
			return false;
		}
		MADAM_CORE_INFO("Creating Project: " + projectConfigDirectory.string());
		if (!Platform::CreateDirectory(projectConfigDirectory))
		{
			return false;
		}

		std::vector<std::filesystem::path> projectDirectories;
		for (const std::filesystem::path& path : DEFAULT_PROJECT_DIRECTORIES)
		{
			projectDirectories.push_back(projectConfigDirectory / path);
		}

		if (!Platform::CreateDirectory(projectDirectories))
		{
			return false;
		}
		std::ofstream confFile;
		if (!Platform::OutputFile(confFile, projectConfigDirectory / "ProjectInfo.conf"))
		{
			MADAM_ERROR("Failed to create projectInfo.conf");
			return false;
		}
		confFile << "ProjectName: " << config.projectName << std::endl;
		confFile << "ProjectVersion: " << config.projectVersion << std::endl;
		confFile << "ProjectAuthor: " << config.projectAuthor << std::endl;
		std::filesystem::path assetMetaPath = projectConfigDirectory / "Resources" / "Resources.mmeta";
		confFile << "AssetPackLocation: " << assetMetaPath.string() << std::endl;
		confFile.close();

		std::ofstream assetMetaFile;
		if (!Platform::OutputFile(assetMetaFile, projectConfigDirectory / "Resources" / "Resources.mmeta"))
		{
			MADAM_ERROR("Failed to create Resources.mmeta");
			return false;
			
		}

		assetMetaFile << "Registry: " << std::endl;
		assetMetaFile.close();
		return true;
	}

	bool Project::loadProject(std::filesystem::path projectDirectory)
	{
		Project* temp = new Project(projectDirectory);
		if (temp->instanceFlag)
		{
			return true;
		}
		else
		{
			clear();
			return false;
		}
	}

	bool Project::newProject(ProjectConfig config)
	{
		if (createProject(config))
		{
			if (loadProject(config.projectsDirectory / config.projectName))
			{
				return true;
			}
		}
		else
		{
			if (loadProject(config.projectsDirectory / config.projectName))
			{
				return false;
			}
		}
		return false;
	}

	bool Project::saveProject()
	{
		std::filesystem::path projectDirectory = Project::Get().getProjectDirectory();
		ProjectInfo projectInfo = Project::Get().getProjectInfo();

		std::ofstream confFile;
		if (!Platform::OutputFile(confFile, projectDirectory / "ProjectInfo.conf"))
		{
			MADAM_CORE_ERROR("Failed to create projectInfo.conf");
			return false;
		}

		confFile << "ProjectName: " << projectInfo.projectName << std::endl;
		confFile << "ProjectVersion: " << projectInfo.projectVersion << std::endl;
		confFile << "ProjectAuthor: " << projectInfo.projectAuthor << std::endl;
		std::filesystem::path assetMetaPath = projectInfo.assetMetaPath;
		confFile << "AssetPackLocation: " << assetMetaPath.string() << std::endl;
		confFile.close();

		Project::Get().assetManager->SaveMetaData();
		return true;
	}

	ProjectInfo Project::getProjectInfo() const
	{
		return projectInfo;
	}

	AssetManager& Project::getAssetManager() const
	{
		return *assetManager;
	}
}