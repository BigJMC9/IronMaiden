#pragma once
#include "maidenpch.hpp"
#include "H_Metadata.h"
#include "H_AssetSerializer.h"
#include "Asset.h"

namespace Madam
{
	class AssetImporter
	{
	public:
		static void Init();
		static void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset);
		static void Serialize(const Ref<Asset>& asset);
		static bool tryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset);

	private:
		static std::unordered_map<AssetType, Scope<AssetSerializer>> s_Serializers;
	};
}