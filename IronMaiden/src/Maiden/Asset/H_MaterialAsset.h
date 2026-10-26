#pragma once

#include "Asset.h"
#include <map>

namespace Madam
{
	class AssetManager;
	struct CMaterial;

	// A project material contains asset references only. GPU descriptors live in CMaterial.
	class MaterialAsset : public Asset
	{
	public:
		UUID shader = UUID("editor://shaders/texture_shader.frag");
		std::map<std::string, UUID> textures;

		static AssetType GetStaticType() { return AssetType::MATERIAL; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		Ref<CMaterial> GetRuntimeMaterial(AssetManager& manager);

	private:
		Ref<CMaterial> runtimeMaterial;
	};
}
