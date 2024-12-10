#pragma once
#include "maidenpch.hpp"
#include "../Core/H_CoreUtils.hpp"
#include "../Core/Main/Core.hpp"
#include "H_Metadata.h"

#include <filesystem>

namespace Madam
{
	class AssetRegistry
	{
	public:
		AssetMetadata& operator[](const UUID uuid);
		//AssetMetadata& operator[](const std::filesystem::path path);
		AssetMetadata& Get(const UUID uuid);
		const AssetMetadata& Get(const UUID uuid) const;

		size_t count() const;
		bool contains(const UUID uuid) const;
		//bool contains(const std::filesystem::path path) const;
		size_t remove(const UUID uuid);
		void clear();

		auto begin() { return assetRegistry.begin(); }
		auto end() { return assetRegistry.end(); }
		auto begin() const { return assetRegistry.cbegin(); }
		auto end() const { return assetRegistry.cend(); }

		bool Serialize(std::ofstream& file) const;
		bool Deserialize(std::ifstream& file);

	private:
		std::unordered_map<UUID, AssetMetadata> assetRegistry;

		// How would I do this?
		//std::unordered_map<std::filesystem::path, UUID> assetFileMap;
	};
}