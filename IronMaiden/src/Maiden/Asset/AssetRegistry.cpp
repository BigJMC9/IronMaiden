#include "maidenpch.hpp"
#include "H_AssetRegistry.h"
#include "../Core/H_Logger.hpp"
#include <fstream>

#ifndef MADAM_DYNAMIC_LINK
	#ifndef YAML_CPP_STATIC_DEFINE
		#define YAML_CPP_STATIC_DEFINE
	#endif
#endif
#include <yaml-cpp/yaml.h>

namespace YAML {
	template<>
	struct convert<Madam::UUID>
	{
		static Node encode(const Madam::UUID& uuid)
		{
			return Node((std::string)uuid);
		}

		static bool decode(const Node& node, Madam::UUID& uuid)
		{
			uuid = Madam::UUID(node.as<std::string>());
			return true;
		}
	};

	template<>
	struct convert<Madam::AssetMetadata>
	{
		static Node encode(const Madam::AssetMetadata& metadata)
		{
			Node node;
			node["UUID"] = metadata.uuid;
			node["Type"] = Madam::assetTypeToString(metadata.assetType);
			node["File"] = metadata.filepath.string();
			node["IsGPUObject"] = metadata.isGPUObject;
			return node;
		}

		static bool decode(const Node& node, Madam::AssetMetadata& metadata)
		{
			metadata.uuid = node["UUID"].as<Madam::UUID>();
			auto type = Madam::assetTypeMap.find(node["Type"].as<std::string>());
			if (type == Madam::assetTypeMap.end())
				return false;
			metadata.assetType = type->second;
			metadata.filepath = std::filesystem::u8path(node["File"].as<std::string>());
			metadata.isGPUObject = node["IsGPUObject"].as<bool>(false);
			return true;
		}
	};
}

namespace Madam
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const AssetMetadata& metaData) {
		out << YAML::Flow;
		out << YAML::BeginMap;
		out << YAML::Key << "UUID" << YAML::Value << metaData.uuid;
		out << YAML::Key << "Type" << YAML::Value << assetTypeToString(metaData.assetType);
		out << YAML::Key << "File" << YAML::Value << metaData.filepath.string();
		out << YAML::Key << "IsGPUObject" << YAML::Value << metaData.isGPUObject;
		out << YAML::EndMap;
		return out;
	}

	AssetMetadata& AssetRegistry::operator[](const UUID uuid)
	{
		return assetRegistry[uuid];
	}

	/*AssetMetadata& AssetRegistry::operator[](const std::filesystem::path path)
	{
		return assetRegistry[assetFileMap[path]];
	}*/

	AssetMetadata& AssetRegistry::Get(const UUID uuid)
	{
		return assetRegistry[uuid];
	}

	const AssetMetadata& AssetRegistry::Get(const UUID uuid) const
	{
		return assetRegistry.at(uuid);
	}


	size_t AssetRegistry::count() const
	{
		return assetRegistry.size();
	}


	bool AssetRegistry::contains(const UUID uuid) const
	{
		return assetRegistry.find(uuid) != assetRegistry.end();
	}

	/*bool AssetRegistry::contains(const std::filesystem::path path) const
	{
		return assetFileMap.find(path) != assetFileMap.end();
	}*/

	size_t AssetRegistry::remove(const UUID uuid)
	{
		return assetRegistry.erase(uuid);
	}

	void AssetRegistry::clear()
	{
		assetRegistry.clear();
	}

	bool AssetRegistry::Serialize(std::ofstream& file) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "m_registry" << YAML::Value << YAML::BeginSeq;
		for (const auto& [uuid, metadata] : assetRegistry)
		{
			out << metadata;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		file << out.c_str();
		file.flush();
		return out.good() && file.good();
	}

	bool AssetRegistry::Deserialize(std::ifstream& file)
	{
		try
		{
			YAML::Node node = YAML::Load(file);
			YAML::Node entries = node["m_registry"];
			if (!entries || (!entries.IsNull() && !entries.IsSequence()))
				return false;
			std::unordered_map<UUID, AssetMetadata> loaded;
			if (entries.IsSequence())
			{
				for (const auto& entry : entries)
				{
					AssetMetadata metadata = entry.as<AssetMetadata>();
					if (!metadata.IsValid() || metadata.filepath.empty() || loaded.count(metadata.uuid))
						return false;
					loaded.emplace(metadata.uuid, std::move(metadata));
				}
			}
			assetRegistry = std::move(loaded);
			return true;
		}
		catch (const YAML::Exception& e)
		{
			MADAM_CORE_ERROR("Asset registry could not be parsed: {0}", e.what());
			return false;
		}
	}
}
