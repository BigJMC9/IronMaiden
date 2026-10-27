#pragma once
#include "maidenpch.hpp"
#include "H_AssetRegistry.h"
#include "H_ShaderDefinition.h"
#include "AssetUtils.h"
#include <chrono>

namespace Madam
{
	class Asset;
	class SourceAsset;
	class MaterialAsset;
	enum class AssetType;
	struct AssetReference
	{
		UUID handle = UUID("");
		AssetType type = AssetType::NONE;
		std::filesystem::path path;
		bool editorResource = false;
	};

	class AssetManager
	{

	public:
		AssetManager(std::filesystem::path projectPath, std::filesystem::path registryLocation = {});
		~AssetManager();

		AssetType GetAssetType(UUID uuid);
		Ref<Asset> GetAsset(UUID uuid);
		bool IsAssetLoaded(UUID uuid) const { return loadedAssets.find(uuid) != loadedAssets.end(); }
		Ref<MaterialAsset> CreateMaterial(const std::filesystem::path& path);
		Ref<SourceAsset> CreateShader(const std::filesystem::path& fragmentPath);
		Ref<SourceAsset> CreateCSharpScript(const std::filesystem::path& path);
		bool CompileShader(UUID shader, std::string* diagnostic = nullptr) const;
		bool SaveAsset(const Ref<Asset>& asset);
		std::filesystem::path GetAbsolutePath(const std::filesystem::path& path) const;
		std::filesystem::path GetRegistryLocation() const;
		std::filesystem::path ResolveAssetPath(UUID handle) const;
		std::vector<AssetReference> ListAssets(AssetType type) const;
		ShaderDefinition GetShaderDefinition(UUID shader) const;

		const AssetMetadata& GetMetadata(UUID uuid);
		const AssetMetadata& GetMetadata(std::filesystem::path path);
		const AssetMetadata& GetMetadata(const Ref<Asset>& asset);

		void RemoveMetadata(UUID uuid);
		void RemoveMetadata(std::filesystem::path path);
		bool MoveMetadata(const std::filesystem::path& oldPath, const std::filesystem::path& newPath);

		bool IsAssetHandleValid(UUID uuid) { const auto& metadata = GetMetadata(uuid); return metadata.IsValid() && !metadata.isMissing; }

		bool SaveMetaData();
		bool ScanAssets();

		void AppendMetaData(const std::filesystem::path& path);
	private:
		void LoadRegistry();
		void SetMetaData(const std::filesystem::path& path);
		void RebuildPathIndex();
		std::filesystem::path NormalizeAssetPath(const std::filesystem::path& path, bool recoverLegacy = false) const;

		AssetMetadata& GetInternalMetadata(UUID uuid);

		std::filesystem::path projectPath;
		std::filesystem::path registryPath;
		AssetRegistry m_registry;
		std::unordered_map<std::filesystem::path, UUID> pathIndex;
		std::unordered_map<UUID, Ref<Asset>> loadedAssets;
		struct CachedShaderDefinition
		{
			std::filesystem::path source;
			std::filesystem::path vertex;
			std::filesystem::file_time_type sourceTime{};
			std::filesystem::file_time_type vertexTime{};
			uintmax_t sourceSize = 0;
			uintmax_t vertexSize = 0;
			ShaderDefinition definition;
		};
		mutable std::unordered_map<UUID, CachedShaderDefinition> shaderDefinitions;
		bool registryWritable = true;
	};
}
