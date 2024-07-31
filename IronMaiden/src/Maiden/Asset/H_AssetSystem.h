#pragma once
#include "maidenpch.hpp"
#include "H_AssetRegistry.h"
#include "AssetUtils.h"
#include <chrono>

namespace Madam
{
	class Asset;
	enum class AssetType;

	class AssetManager
	{

	public:
		AssetManager(std::filesystem::path projectPath);
		~AssetManager();

		AssetType GetAssetType(UUID uuid);
		Ref<Asset> GetAsset(UUID uuid);

		const AssetMetadata& GetMetadata(UUID uuid);
		const AssetMetadata& GetMetadata(std::filesystem::path path);
		const AssetMetadata& GetMetadata(const Ref<Asset>& asset);

		AssetMetadata& GetMutableMetadata(UUID uuid);
		AssetMetadata& GetMutableMetadata(std::filesystem::path path);

		void RemoveMetadata(UUID uuid);
		void RemoveMetadata(std::filesystem::path path);

		bool IsAssetHandleValid(UUID uuid) { return GetMetadata(uuid).IsValid(); }

		bool SaveMetaData();
		bool ScanAssets();

		void AppendMetaData(const std::filesystem::path& path);
	private:
		void LoadRegistry();
		void SetMetaData(const std::filesystem::path& path);

		AssetMetadata& GetInternalMetadata(UUID uuid);

		std::filesystem::path projectPath;
		AssetRegistry registry;
		std::unordered_map<UUID, Ref<Asset>> loadedAssets;
	};
}