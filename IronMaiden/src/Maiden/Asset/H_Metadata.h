#pragma once
#include "../Core/H_Utils.hpp"
#include "AssetUtils.h"
#include <filesystem>

namespace Madam
{
	struct AssetMetadata
	{
		UUID uuid = UUID("");
		AssetType assetType = AssetType::NONE;
		std::filesystem::path filePath;

		bool isLoaded = false;
		bool isVirtual = false; // ?? maybe
		bool isValid() const { return std::string(uuid) != ""; }
	};
}