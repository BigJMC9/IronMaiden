#pragma once
#include "../Core/H_CoreUtils.hpp"
#include "AssetUtils.h"
#include <filesystem>

namespace Madam
{
	struct AssetMetadata
	{
		UUID uuid = UUID("");
		AssetType assetType = AssetType::NONE;
		std::filesystem::path filepath;

		bool isLoaded = false;
		bool isGPUObject = false;
		bool IsValid() const { return std::string(uuid) != ""; }
	};
}