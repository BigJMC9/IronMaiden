#include "maidenpch.hpp"
#include "H_AssetRegistry.h"
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
			Node node;
			node.push_back((std::string)uuid);
			return node;
		}

		static bool decode(const Node& node, Madam::UUID& uuid)
		{
			uuid = Madam::UUID(node.as<std::string>());
			return true;
		}
	};

	template<>
	struct convert<Madam::Asset::AssetMetadata>
	{
		static Node encode(const Madam::Asset::AssetMetadata& metadata)
		{
			Node node;
			node["UUID"] = metadata.uuid;
			node["Type"] = Madam::Asset::assetTypeToString(metadata.assetType);
			node["File"] = metadata.filePath.string();
			node["IsVirtual"] = metadata.isVirtual;
			return node;
		}

		static bool decode(const Node& node, Madam::Asset::AssetMetadata& metadata)
		{
			metadata.uuid = node["UUID"].as<Madam::UUID>();
			metadata.assetType = Madam::Asset::assetTypeMap[node["Type"].as<std::string>()];
			metadata.filePath = std::filesystem::u8path(node["File"].as<std::string>());
			metadata.isVirtual = node["IsVirtual"].as<bool>();
			return true;
		}
	};
}

namespace Madam::Asset 
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const AssetMetadata& metaData) {
		out << YAML::Flow;
		out << YAML::BeginMap;
		out << YAML::Key << "UUID" << YAML::Value << metaData.uuid;
		out << YAML::Key << "Type" << YAML::Value << assetTypeToString(metaData.assetType);
		out << YAML::Key << "File" << YAML::Value << metaData.filePath.string();
		out << YAML::Key << "IsVirtual" << YAML::Value << metaData.isVirtual;
		out << YAML::EndMap;
		return out;
	}

	AssetMetadata& AssetRegistry::operator[](const UUID uuid)
	{
		return assetRegistry[uuid];
	}

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
		out << YAML::Key << "Registry" << YAML::Value << YAML::BeginSeq;
		for (const auto& [uuid, metadata] : assetRegistry)
		{
			out << metadata;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		file << out.c_str();
		return true;
	}

	bool AssetRegistry::Deserialize(std::ifstream& file)
	{
		assetRegistry.clear();
		YAML::Node node = YAML::Load(file);
		if (!node["Registry"])
		{
			std::cout << "AssetRegistry node does not exist" << std::endl;
			return false;
		}
		else if (node["Registry"].IsDefined())
		{
			std::cout << "AssetRegistry node is null" << std::endl;
			return false;
		}
		else
		{
			for (const auto& metadata : node)
			{
				if (!metadata)
				{
					std::cout << "AssetMetadata node does not exist" << std::endl;
				}
				else if (metadata.IsNull())
				{
					std::cout << "AssetMetadata node is null" << std::endl;
				}
				else
				{
					AssetMetadata assetMetadata = metadata.as<AssetMetadata>();
					assetRegistry[assetMetadata.uuid] = assetMetadata;
				}
			}
		}
		return true;
	}
}