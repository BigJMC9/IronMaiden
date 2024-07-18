#pragma once
#include "../Core/H_Utils.hpp"
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

		virtual bool operator==(const Asset& other) const
		{
			return handle == other.handle;
		}

		virtual bool operator!=(const Asset& other) const
		{
			return !(*this == other);
		}

	};
}