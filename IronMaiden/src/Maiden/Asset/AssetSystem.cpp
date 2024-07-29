#include "maidenpch.hpp"
#include "H_AssetSystem.h"
#include "../Platform/Platforms.hpp"
#include "../Project/H_Project.h"
#include "H_AssetImporter.h"
#include <filesystem>
#include <fstream>

namespace Madam
{
	static AssetMetadata s_NullMetadata;

	AssetManager::AssetManager(std::filesystem::path projectPath)
	{
		this->projectPath = projectPath;
		registry = AssetRegistry();
		loadRegistry();
		AssetImporter::Init();
	}

	AssetManager::~AssetManager()
	{

	}

	AssetType AssetManager::GetAssetType(UUID uuid)
	{
		if (!isAssetHandleValid(uuid))
			return AssetType::NONE;

		const AssetMetadata& metadata = GetMetadata(uuid);
		return metadata.assetType;
	}

	Ref<Asset> AssetManager::GetAsset(UUID uuid)
	{
		AssetMetadata& metadata = GetInternalMetadata(uuid);
		if (!metadata.isValid())
			return nullptr;

		Ref<Asset> asset = nullptr;
		if (!metadata.isLoaded)
		{
			metadata.isLoaded = AssetImporter::tryLoadData(metadata, asset);
			if (!metadata.isLoaded)
			{
				return nullptr;
			}

			loadedAssets[uuid] = asset;
		}
		else
		{
			asset = loadedAssets[uuid];
		}

		return asset;
	}

	const AssetMetadata& AssetManager::GetMetadata(UUID uuid)
	{
		if (registry.contains(uuid))
			return registry[uuid];

		return s_NullMetadata;
	}

	const AssetMetadata& AssetManager::GetMetadata(std::filesystem::path path)
	{
		for (auto& [handle, metadata] : registry)
		{
			if (metadata.filepath == path)
				return metadata;
		}

		return s_NullMetadata;
	}

	const AssetMetadata& AssetManager::GetMetadata(const Ref<Asset>& asset)
	{
		return GetMetadata(asset->handle);
	}

	AssetMetadata& AssetManager::GetMutableMetadata(UUID uuid)
	{
		if (registry.contains(uuid))
			return registry[uuid];

		return s_NullMetadata;
	}

	AssetMetadata& AssetManager::GetMutableMetadata(std::filesystem::path path)
	{
		for (auto& [handle, metadata] : registry)
		{
			if (metadata.filepath == path)
				return metadata;
		}

		return s_NullMetadata;
	}

	AssetMetadata& AssetManager::GetInternalMetadata(UUID uuid)
	{
		if (registry.contains(uuid))
			return registry[uuid];

		return s_NullMetadata;
	}

	void AssetManager::loadRegistry()
	{
		std::filesystem::path resourcePath = projectPath / std::filesystem::u8path(registryFile());
		std::ifstream metadataFile;
		if (Platform::OpenFile(metadataFile, Project::Get().getProjectInfo().assetMetaPath))
		{
			registry.Deserialize(metadataFile);
			metadataFile.close();
		}
		else if (Platform::OpenFile(metadataFile, resourcePath))
		{
			registry.Deserialize(metadataFile);
			metadataFile.close();
		}
		else 
		{
			std::ofstream metadataOutputFile;
			if (Platform::OutputFile(metadataOutputFile, resourcePath))
			{
				metadataOutputFile.close();
			}
		}
		scanAssets();
	}

	bool AssetManager::saveMetaData()
	{
		std::filesystem::path resourcePath = projectPath / std::filesystem::u8path(registryFile());
		std::ofstream metadataFile;
		if (Platform::OutputFile(metadataFile, Project::Get().getProjectInfo().assetMetaPath))
		{
			registry.Serialize(metadataFile);
			metadataFile.close();
			return true;
		}
		else if (Platform::OutputFile(metadataFile, resourcePath))
		{
			registry.Serialize(metadataFile);
			metadataFile.close();
			return true;
		}
		else
		{
			return false;
		}
	}

	//optimize this
	bool AssetManager::scanAssets()
	{
		std::filesystem::path assetPath = projectPath / assets;
		if (!std::filesystem::exists(assetPath))
		{
			MADAM_CORE_ERROR("Asset path does not exist: " + assetPath.string());
			return false;
		}
		for (const auto& entry : std::filesystem::recursive_directory_iterator(assetPath))
		{
			if (entry.is_directory())
			{
				MADAM_CORE_INFO("Directory: " + entry.path().string());
			}
			else if (entry.is_regular_file())
			{
				MADAM_CORE_INFO("File: " + entry.path().string());
				bool doesExist = false;
				if (!GetMetadata(entry.path()).isValid())
				{
					setMetaData(entry.path());
				}
				/*if (!registry.contains(entry.path()))
				{
					setMetaData(entry.path());
				}*/
			}
		}
		return true;
	}

	void AssetManager::appendMetaData(const std::filesystem::path& path)
	{
		if (!GetMetadata(path).isValid())
		{
			setMetaData(path);
		}
	}

	void AssetManager::setMetaData(const std::filesystem::path& path)
	{
		AssetMetadata metadata;
		metadata.filepath = path;
		metadata.uuid = UUID();
		metadata.assetType = assetExtensionMap[path.extension().string()];
		registry[metadata.uuid] = metadata;
	}
}