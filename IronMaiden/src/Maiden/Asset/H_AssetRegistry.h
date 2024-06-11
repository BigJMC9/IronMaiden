#pragma once
#include "maidenpch.hpp"
#include "../Core/H_Utils.hpp"
#include "../Core/Main/Core.hpp"
#include "H_Metadata.h"

#include <filesystem>

namespace Madam::Asset
{
	class AssetRegistry
	{
	public:
		AssetMetadata& operator[](const UUID uuid);
		AssetMetadata& Get(const UUID uuid);
		const AssetMetadata& Get(const UUID uuid) const;

		size_t count() const;
		bool contains(const UUID uuid) const;
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
	};
}