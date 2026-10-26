#include "maidenpch.hpp"
#include "H_AssetImporter.h"
#include "../Core/H_Logger.hpp"

namespace Madam
{
	void AssetImporter::Init()
	{
		s_Serializers.clear();
		s_Serializers[AssetType::TEXTURE] = CreateScope<TextureSerializer>();
		s_Serializers[AssetType::MATERIAL] = CreateScope<MaterialSerializer>();
		s_Serializers[AssetType::MESH] = CreateScope<MeshSerializer>();
	}

	bool AssetImporter::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset)
	{
		auto serializer = s_Serializers.find(metadata.assetType);
		if (serializer == s_Serializers.end() || !asset || asset->GetAssetType() != metadata.assetType)
			return false;
		return serializer->second->serialize(metadata, asset);
	}

	bool AssetImporter::tryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset)
	{
		if (s_Serializers.find(metadata.assetType) == s_Serializers.end())
		{
			MADAM_CORE_WARN("There's currently no importer for assets {0} of type {1}", metadata.filepath.stem().string(), assetTypeToString(metadata.assetType));
			return false;
		}

		return s_Serializers[metadata.assetType]->tryLoadData(metadata, asset);
	}

	std::unordered_map<AssetType, Scope<AssetSerializer>> AssetImporter::s_Serializers;
}
