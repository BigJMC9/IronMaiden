#pragma once
#include "../Core/H_Utils.hpp"
#include "AssetUtils.h"
#include <filesystem>

namespace Madam::Asset
{
	struct AssetMetadata
	{
		UUID uuid = UUID("");
		AssetType assetType = AssetType::NONE;
		std::filesystem::path filePath;

		bool isLoaded = false;
		bool isVirtual = false; // ?? maybe
	};
}