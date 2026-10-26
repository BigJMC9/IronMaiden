#pragma once
#include "AssetUtils.h"

namespace Madam
{
	class Asset
	{
	public:
		UUID handle;

		virtual ~Asset() {}

		static AssetType GetStaticType() { return AssetType::NONE; }
		virtual AssetType GetAssetType() const { return AssetType::NONE; }

		virtual UUID GetUUID() const { return handle; }

		virtual bool operator==(const Asset& other) const
		{
			return handle == other.handle;
		}

		virtual bool operator!=(const Asset& other) const
		{
			return !(*this == other);
		}

	};

	class SourceAsset final : public Asset
	{
	public:
		explicit SourceAsset(AssetType sourceType) : type(sourceType) {}
		AssetType GetAssetType() const override { return type; }
	private:
		AssetType type;
	};
}
