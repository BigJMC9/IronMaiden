#include "maidenpch.hpp"
#include "H_AssetSerializer.h"
#include "../Project/H_Project.h"
#include "../Rendering/H_Texture.h"
#include "../Rendering/H_Renderer.hpp"
#include "H_MaterialAsset.h"
#include "../Core/H_Logger.hpp"
#include <fstream>

#ifndef MADAM_DYNAMIC_LINK
	#ifndef YAML_CPP_STATIC_DEFINE
		#define YAML_CPP_STATIC_DEFINE
	#endif
#endif
#include <yaml-cpp/yaml.h>

namespace Madam
{
	bool TextureSerializer::tryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
        TextureData data;
        asset = Texture::Create(data, metadata.filepath);
		if (!asset)
			return false;
		asset->handle = metadata.uuid;

        auto texture = std::dynamic_pointer_cast<Texture>(asset);
        if (texture) 
        {
            //bool result = texture->loaded();
            return true;
        }
        else 
        {
            return false;
        }
	}

    bool MeshSerializer::tryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
    {
        asset = StaticMesh::Create(metadata.filepath);
		if (!asset)
			return false;
        asset->handle = metadata.uuid;

        auto mesh = std::dynamic_pointer_cast<StaticMesh>(asset);
        if (mesh)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

	bool MaterialSerializer::serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		auto material = std::dynamic_pointer_cast<MaterialAsset>(asset);
		if (!material)
			return false;

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Material" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "Version" << YAML::Value << 2;
		out << YAML::Key << "Shader" << YAML::Value << (std::string)material->shader;
		out << YAML::Key << "Textures" << YAML::Value << YAML::BeginMap;
		for (const auto& [name, texture] : material->textures)
			out << YAML::Key << name << YAML::Value << (std::string)texture;
		out << YAML::EndMap << YAML::EndMap << YAML::EndMap;
		if (!out.good())
			return false;

		const auto tempPath = std::filesystem::path(metadata.filepath.wstring() + L".tmp");
		std::ofstream file(tempPath, std::ios::binary | std::ios::trunc);
		if (!file)
			return false;
		file << out.c_str();
		file.flush();
		const bool written = file.good();
		file.close();
		if (!written || !MoveFileExW(tempPath.c_str(), metadata.filepath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			std::error_code error;
			std::filesystem::remove(tempPath, error);
			return false;
		}
		return true;
	}

	bool MaterialSerializer::tryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		try
		{
			YAML::Node root = YAML::LoadFile(metadata.filepath.string());
			YAML::Node materialNode = root["Material"];
			if (!materialNode || !materialNode.IsMap())
				return false;
			const int version = materialNode["Version"].as<int>();
			if (version != 1 && version != 2)
				return false;
			YAML::Node textures = materialNode["Textures"];
			if (!textures || !textures.IsMap())
				return false;

			auto material = CreateRef<MaterialAsset>();
			material->handle = metadata.uuid;
			if (version == 1)
			{
				material->textures["diffuseMap"] = UUID(textures["Diffuse"].as<std::string>(""));
				material->textures["normalMap"] = UUID(textures["Normal"].as<std::string>(""));
				material->textures["ambientOcclusionMap"] = UUID(textures["AO"].as<std::string>(""));
				material->textures["glossMap"] = UUID(textures["Gloss"].as<std::string>(""));
			}
			else
			{
				material->shader = UUID(materialNode["Shader"].as<std::string>("editor://shaders/texture_shader.frag"));
				for (const auto& input : textures)
					material->textures[input.first.as<std::string>()] = UUID(input.second.as<std::string>(""));
			}
			asset = material;
			return true;
		}
		catch (const YAML::Exception& e)
		{
			MADAM_CORE_ERROR("Unable to load material {0}: {1}", metadata.filepath.string(), e.what());
			return false;
		}
	}
}
