#pragma once
#include "maidenpch.hpp"
#include "H_AssetRegistry.h"
#include "AssetUtils.h"
#include <chrono>

namespace Madam::Asset
{
	class Asset;
	enum class AssetType;

	class AssetManager
	{

	public:
		AssetManager(std::filesystem::path projectPath);
		~AssetManager();

		bool updateMetaData();
		bool scanAssets();
	private:
		void loadRegistry();
		void setMetaData(const std::filesystem::path& path);

		std::filesystem::path projectPath;
		//std::filesystem::file_time_type lastWriteTime;
		AssetRegistry registry;
	};
}