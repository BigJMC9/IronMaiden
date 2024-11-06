#include "maidenpch.hpp"
#include "H_AssetSerializer.h"
#include "../Project/H_Project.h"
#include "../Rendering/H_Texture.h"
#include "../Rendering/H_Renderer.hpp"

namespace Madam
{
	bool TextureSerializer::tryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
        TextureData data;
        asset = Texture::Create(data, metadata.filepath);
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
}