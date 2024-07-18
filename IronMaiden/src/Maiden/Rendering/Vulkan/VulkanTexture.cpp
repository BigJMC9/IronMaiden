#include "maidenpch.hpp"
#include "H_VulkanTexture.h"
#include "../H_Renderer.hpp"

// libs
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Madam
{
    VulkanTexture::VulkanTexture(const TextureData& textureData) : data{ textureData }
    {
        Load(Rendering::Renderer::GetDevice());
    }

	VulkanTexture::VulkanTexture(const TextureData& textureData, const std::filesystem::path& filepath) : data{ textureData }, filepath{ filepath }
	{
        Load(Rendering::Renderer::GetDevice());
	}

	VulkanTexture::~VulkanTexture()
	{
		vkDestroySampler(Rendering::Renderer::GetDevice().device(), textureSampler, nullptr);
	}

	void VulkanTexture::Load(Device& device)
	{
		VkFormat format = Rendering::VulkanImage::GetVkFormat(data.format);
		VkExtent3D extent;
		extent.width = data.width;
		extent.height = data.height;
		extent.depth = data.depth;

		if (filepath.empty()) 
		{
			return;
		}

        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filepath.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        data.width = texWidth;
        data.height = texHeight;
        if (!pixels) 
		{
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* bufData;
        vkMapMemory(device.device(), stagingBufferMemory, 0, imageSize, 0, &bufData);
        memcpy(bufData, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device.device(), stagingBufferMemory);

        stbi_image_free(pixels);

        Rendering::Image::ImageData imageData;
        imageData.width = data.width;
        imageData.height = data.height;
        imageData.depth = data.depth;
        imageData.format = data.format;
        imageData.mipLevels = data.mipLevels;
        imageData.arrayLayers = data.arrayLayers;
        imageData.multisample = data.multisample;
        imageData.usage = Rendering::Image::Usage::Texture;
        image = CreateRef<Rendering::VulkanImage>(imageData);

        Rendering::VulkanImage::TransitionImageLayout(device, *image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        Rendering::VulkanImage::CopyBufferToImage(device, stagingBuffer, *image);
        Rendering::VulkanImage::TransitionImageLayout(device, *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // If we generate mip maps then the final image will alerady be READ_ONLY_OPTIMAL
        // mDevice.generateMipmaps(mTextureImage, mFormat, texWidth, texHeight, mMipLevels);
        //textureLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
        vkFreeMemory(device.device(), stagingBufferMemory, nullptr);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = GetVkSamplerFilter(data.samplerFilter);
        samplerInfo.minFilter = GetVkSamplerFilter(data.samplerFilter);

        samplerInfo.addressModeU = GetVkSamplerWrap(data.samplerWrap);
        samplerInfo.addressModeV = GetVkSamplerWrap(data.samplerWrap);
        samplerInfo.addressModeW = GetVkSamplerWrap(data.samplerWrap);

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        // this fields useful for percentage close filtering for shadow maps
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(data.mipLevels);

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
	}
}