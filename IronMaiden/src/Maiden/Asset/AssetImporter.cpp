#include "maidenpch.hpp"
#include "H_AssetImporter.h"
#include "../Core/H_Logger.hpp"

namespace Madam
{
	void AssetImporter::Init()
	{
		s_Serializers.clear();
		s_Serializers[AssetType::TEXTURE] = CreateScope<TextureSerializer>();
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