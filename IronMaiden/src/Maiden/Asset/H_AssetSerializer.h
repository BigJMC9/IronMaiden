#pragma once
#include "H_Metadata.h"
#include "Asset.h"
#include "../Rendering/H_Mesh.h"

namespace Madam
{
	class AssetSerializer
	{
	public:
		virtual void serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const = 0;
		virtual bool tryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const = 0;
	};

	class TextureSerializer : public AssetSerializer
	{
	public:
		virtual void serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override {}
		virtual bool tryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};

	class MeshSerializer : public AssetSerializer
	{
	public:
		virtual void serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override {}
		virtual bool tryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};

	/*class SceneAssetSerializer : public AssetSerializer
	{
	public:
		virtual void serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
		virtual bool tryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};*/
}
