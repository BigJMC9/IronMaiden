#include "maidenpch.hpp"
#include "H_Texture.h"
#include "Vulkan/H_VulkanTexture.h"

namespace Madam
{
	Ref<Texture> Texture::Create(const TextureData& textureData)
	{
		return CreateRef<VulkanTexture>(textureData);
	}

	Ref<Texture> Texture::Create(const TextureData& textureData, const std::filesystem::path& filepath)
	{
		return CreateRef<VulkanTexture>(textureData, filepath);
	}
}