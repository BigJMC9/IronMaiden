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
		LoadRegistry();
		AssetImporter::Init();
	}

	AssetManager::~AssetManager()
	{

	}

	AssetType AssetManager::GetAssetType(UUID uuid)
	{
		if (!IsAssetHandleValid(uuid)) {
			return AssetType::NONE;
		}
		const AssetMetadata& metadata = GetMetadata(uuid);
		return metadata.assetType;
	}

	Ref<Asset> AssetManager::GetAsset(UUID uuid)
	{
		AssetMetadata& metadata = GetInternalMetadata(uuid);
		if (!metadata.IsValid()) {
			return nullptr;
		}
		Ref<Asset> asset = nullptr;
		if (!metadata.isLoaded) {
			metadata.isLoaded = AssetImporter::tryLoadData(metadata, asset);
			if (!metadata.isLoaded) {
				return nullptr;
			}

			loadedAssets[uuid] = asset;
		}
		else {
			asset = loadedAssets[uuid];
		}
		return asset;
	}

	const AssetMetadata& AssetManager::GetMetadata(UUID uuid)
	{
		if (registry.contains(uuid)) {
			return registry[uuid];
		}
		return s_NullMetadata;
	}

	const AssetMetadata& AssetManager::GetMetadata(std::filesystem::path path)
	{
		for (auto& [handle, metadata] : registry) {
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
		if (registry.contains(uuid)) {
			return registry[uuid];
		}
		return s_NullMetadata;
	}

	AssetMetadata& AssetManager::GetMutableMetadata(std::filesystem::path path)
	{
		for (auto& [handle, metadata] : registry) {
			if (metadata.filepath == path) {
				return metadata;
			}
		}
		return s_NullMetadata;
	}

	void AssetManager::RemoveMetadata(UUID uuid)
	{
		registry.remove(uuid);
	}

	void AssetManager::RemoveMetadata(std::filesystem::path path)
	{
		if (std::filesystem::is_directory(path)) {
			for (auto& [handle, metadata] : registry) {
				if (Platform::isChildOf(path, metadata.filepath)) {
					registry.remove(metadata.uuid);
				}
			}
		}
		else {
			for (auto& [handle, metadata] : registry) {
				if (metadata.filepath == path) {
					registry.remove(metadata.uuid);
				}
			}
		}
	}

	AssetMetadata& AssetManager::GetInternalMetadata(UUID uuid)
	{
		if (registry.contains(uuid)) {
			return registry[uuid];
		}

		return s_NullMetadata;
	}

	void AssetManager::LoadRegistry()
	{
		std::filesystem::path resourcePath = projectPath / std::filesystem::u8path(RegistryFile());
		std::ifstream metadataFile;
		if (Platform::OpenFile(metadataFile, Project::Get().getProjectInfo().assetMetaPath)) {
			registry.Deserialize(metadataFile);
			metadataFile.close();
		}
		else if (Platform::OpenFile(metadataFile, resourcePath)) {
			registry.Deserialize(metadataFile);
			metadataFile.close();
		}
		else {
			std::ofstream metadataOutputFile;
			if (Platform::OutputFile(metadataOutputFile, resourcePath)) {
				metadataOutputFile.close();
			}
		}
		ScanAssets();
	}

	bool AssetManager::SaveMetaData()
	{
		std::filesystem::path resourcePath = projectPath / std::filesystem::u8path(RegistryFile());
		std::ofstream metadataFile;
		if (Platform::OutputFile(metadataFile, Project::Get().getProjectInfo().assetMetaPath)) {
			registry.Serialize(metadataFile);
			metadataFile.close();
			return true;
		}
		else if (Platform::OutputFile(metadataFile, resourcePath)) {
			registry.Serialize(metadataFile);
			metadataFile.close();
			return true;
		}
		else {
			return false;
		}
	}

	//optimize this
	bool AssetManager::ScanAssets()
	{
		std::filesystem::path assetPath = projectPath / assets;
		if (!std::filesystem::exists(assetPath)) {
			MADAM_CORE_ERROR("Asset path does not exist: " + assetPath.string());
			return false;
		}
		for (const auto& entry : std::filesystem::recursive_directory_iterator(assetPath)) {
			if (entry.is_directory()) {
				MADAM_CORE_INFO("Directory: " + entry.path().string());
			}
			else if (entry.is_regular_file()) {
				MADAM_CORE_INFO("File: " + entry.path().string());
				bool doesExist = false;
				if (!GetMetadata(entry.path()).IsValid()) {
					SetMetaData(entry.path());
				}
				/*if (!registry.contains(entry.path()))
				{
					setMetaData(entry.path());
				}*/
			}
		}
		return true;
	}

	void AssetManager::AppendMetaData(const std::filesystem::path& path)
	{
		if (!GetMetadata(path).IsValid()) {
			SetMetaData(path);
		}
	}

	void AssetManager::SetMetaData(const std::filesystem::path& path)
	{
		AssetMetadata metadata;
		metadata.filepath = path;
		metadata.uuid = UUID();
		metadata.assetType = assetExtensionMap[path.extension().string()];
		registry[metadata.uuid] = metadata;
	}
}