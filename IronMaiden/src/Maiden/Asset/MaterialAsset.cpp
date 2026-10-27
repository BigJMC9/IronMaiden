#include "maidenpch.hpp"
#include "H_MaterialAsset.h"
#include "H_AssetSystem.h"
#include "../Scene/Components.hpp"

namespace Madam
{
	Ref<CMaterial> MaterialAsset::GetRuntimeMaterial(AssetManager& manager)
	{
		if (!runtimeMaterial)
			runtimeMaterial = CreateRef<CMaterial>();
		const auto definition = manager.GetShaderDefinition(shader);
		runtimeMaterial->shaderDriven = true;
		runtimeMaterial->shaderValid = definition.IsValid();
		const std::string shaderPath = definition.IsValid() ? definition.fragmentPath.string() : "";
		bool bindingsChanged = runtimeMaterial->textureBindings.size() != definition.textures.size();
		if (!bindingsChanged)
		{
			auto old = runtimeMaterial->textureBindings.begin();
			for (const auto& input : definition.textures)
			{
				if (old->first != input.binding)
				{
					bindingsChanged = true;
					break;
				}
				++old;
			}
		}
		if (runtimeMaterial->shaderFragmentPath != shaderPath || bindingsChanged)
			++runtimeMaterial->descriptorLayoutVersion;
		runtimeMaterial->shaderFragmentPath = shaderPath;
		runtimeMaterial->expectedTextureCount = definition.textures.size();
		runtimeMaterial->textureBindings.clear();

		auto texture = [&manager](const UUID& handle) -> Ref<Texture>
		{
			if (handle == null || manager.GetAssetType(handle) != AssetType::TEXTURE)
				return nullptr;
			return std::dynamic_pointer_cast<Texture>(manager.GetAsset(handle));
		};

		for (const auto& input : definition.textures)
		{
			const auto selected = textures.find(input.name);
			runtimeMaterial->textureBindings[input.binding] =
				selected == textures.end() ? nullptr : texture(selected->second);
		}
		auto named = [&](const char* name) -> Ref<Texture>
		{
			const auto input = textures.find(name);
			return input == textures.end() ? nullptr : texture(input->second);
		};
		runtimeMaterial->diffuse_map = named("diffuseMap");
		runtimeMaterial->normal_map = named("normalMap");
		runtimeMaterial->ambient_occlusion_map = named("ambientOcclusionMap");
		runtimeMaterial->gloss_map = named("glossMap");
		return runtimeMaterial;
	}
}
