#include "maidenpch.hpp"
#include "H_AssetSystem.h"
#include <filesystem>
#include <fstream>

namespace Madam::Asset
{
	AssetManager::AssetManager(std::filesystem::path projectPath)
	{
		this->projectPath = projectPath;
		registry = AssetRegistry();
		loadRegistry();
	}

	AssetManager::~AssetManager()
	{

	}

	void AssetManager::loadRegistry()
	{
		std::filesystem::path resourcePath = projectPath / std::filesystem::u8path(registryFile());
		if (!std::filesystem::exists(resourcePath))
		{
			std::cout << "file path does not exist: " << resourcePath.string() << std::endl;
			return;
		}
		std::ifstream metadataFile;
		metadataFile.open(resourcePath);
		if (metadataFile.is_open())
		{
			registry.Deserialize(metadataFile);
			metadataFile.close();
		}
		else
		{
			std::cout << "Failed to open: " << resourcePath << std::endl;
		}
	}

	bool AssetManager::updateMetaData()
	{
		std::filesystem::path resourcePath = projectPath / std::filesystem::u8path(registryFile());
		std::ofstream metadataFile(resourcePath);
		if (metadataFile.is_open())
		{
			registry.Serialize(metadataFile);
			metadataFile.close();
			return true;
		}
		else
		{
			std::cout << "Failed to open: " << resourcePath << std::endl;
			return false;
		}
	}

	bool AssetManager::scanAssets()
	{
		std::filesystem::path assetPath = projectPath / assets;
		if (!std::filesystem::exists(assetPath))
		{
			std::cout << "Asset path does not exist: " << assetPath.string() << std::endl;
			return false;
		}
		for (const auto& entry : std::filesystem::recursive_directory_iterator(assetPath))
		{
			if (entry.is_directory())
			{
				std::cout << "Directory: " << entry.path().string() << std::endl;
			}
			else if (entry.is_regular_file())
			{
				std::cout << "File: " << entry.path().string() << std::endl;
				bool doesExist = false;
				for (auto& [uuid, metadata] : registry)
				{
					if (metadata.filePath == entry.path())
					{
						doesExist = true;
						break;
					}
				}
				if (!doesExist)
				{
					setMetaData(entry.path());
				}
			}
		}
		return true;
	}

	void AssetManager::setMetaData(const std::filesystem::path& path)
	{
		AssetMetadata metadata;
		metadata.filePath = path;
		metadata.uuid = UUID();
		metadata.assetType = assetExtensionMap[path.extension().string()];
		registry[metadata.uuid] = metadata;
	}
}