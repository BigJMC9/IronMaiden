#include "maidenpch.hpp"
#include "H_VulkanTexture.h"
#include "../H_Renderer.hpp"
#include <future>
#include <mutex>
#include <system_error>
#include <unordered_map>

// libs
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Madam
{
	namespace
	{
		struct DecodedTexture
		{
			int width = 0;
			int height = 0;
			int channels = 0;
			std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> pixels{ nullptr, stbi_image_free };
		};
		struct PendingDecode
		{
			std::filesystem::file_time_type modified;
			std::shared_future<std::shared_ptr<DecodedTexture>> result;
		};
		std::mutex decodeMutex;
		std::unordered_map<std::wstring, PendingDecode> pendingDecodes;
	}

	void VulkanTexture::Prefetch(const std::filesystem::path& path)
	{
		if (path.empty()) return;
		const auto normalized = std::filesystem::absolute(path).lexically_normal();
		std::error_code error;
		const auto modified = std::filesystem::last_write_time(normalized, error);
		if (error) return;
		std::lock_guard lock(decodeMutex);
		const auto key = normalized.wstring();
		if (pendingDecodes.find(key) != pendingDecodes.end()) return;
		// Keep abandoned drags from retaining decoded images indefinitely.
		if (pendingDecodes.size() >= 8)
		{
			for (auto it = pendingDecodes.begin(); it != pendingDecodes.end();)
				if (it->second.result.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
					it = pendingDecodes.erase(it);
				else ++it;
			if (pendingDecodes.size() >= 8) return;
		}
		try
		{
			pendingDecodes.emplace(key, PendingDecode{ modified,
				std::async(std::launch::async, [normalized]() -> std::shared_ptr<DecodedTexture>
				{
					auto decoded = std::make_shared<DecodedTexture>();
					decoded->pixels.reset(stbi_load(normalized.string().c_str(), &decoded->width,
						&decoded->height, &decoded->channels, STBI_rgb_alpha));
					return decoded;
				}).share() });
		}
		catch (const std::system_error&)
		{
			// The normal synchronous load remains available if a worker cannot start.
		}
	}

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

        int texWidth = 0, texHeight = 0, texChannels = 0;
		std::shared_ptr<DecodedTexture> prefetched;
		std::shared_future<std::shared_ptr<DecodedTexture>> pending;
		bool matchingPrefetch = false;
		const auto normalized = std::filesystem::absolute(filepath).lexically_normal();
		std::error_code stampError;
		const auto modified = std::filesystem::last_write_time(normalized, stampError);
		{
			std::lock_guard lock(decodeMutex);
			auto found = pendingDecodes.find(normalized.wstring());
			if (found != pendingDecodes.end())
			{
				matchingPrefetch = !stampError && found->second.modified == modified;
				pending = found->second.result; // release a stale worker outside the cache lock
				pendingDecodes.erase(found);
			}
		}
		if (matchingPrefetch)
		{
			try { prefetched = pending.get(); }
			catch (const std::exception&) { /* Decode again on this thread below. */ }
		}
		if (prefetched && prefetched->pixels)
		{
			texWidth = prefetched->width;
			texHeight = prefetched->height;
			texChannels = prefetched->channels;
		}
		const bool usingPrefetch = prefetched && prefetched->pixels;
		stbi_uc* pixels = usingPrefetch ? prefetched->pixels.get() :
			stbi_load(filepath.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        _flags |= TextureFlags::Loaded;
        if (!pixels) 
		{
            if (!usingPrefetch) stbi_image_free(pixels);
            MADAM_ERROR("Asset {0}: Failed to load texture image!", handle);
            const std::filesystem::path debugpath(std::filesystem::u8path("resources\\textures\\debug.png"));
            pixels = stbi_load(debugpath.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            _flags |= TextureFlags::Error;
            _flags &= ~TextureFlags::Loaded;
            if (!pixels)
            {
                MADAM_CORE_ERROR("STBI FAILURE: {0}", stbi_failure_reason());
                throw std::runtime_error("Missing Application Files:\nresources\\textures\\debug.png");
                std::terminate();
            }
        }
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        data.width = texWidth;
        data.height = texHeight;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* bufData;
        vkMapMemory(device.device(), stagingBufferMemory, 0, imageSize, 0, &bufData);
        memcpy(bufData, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device.device(), stagingBufferMemory);

        if (!usingPrefetch) stbi_image_free(pixels);

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

        // Record both transitions and the copy together. Each old helper submitted
        // separately and waited for the entire graphics queue to become idle.
        VkCommandBuffer upload = device.beginSingleTimeCommands();
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image->GetImage();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = data.mipLevels;
        barrier.subresourceRange.layerCount = data.arrayLayers;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(upload, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkBufferImageCopy copy{};
        copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.imageSubresource.layerCount = data.arrayLayers;
        copy.imageExtent = { data.width, data.height, 1 };
        vkCmdCopyBufferToImage(upload, stagingBuffer, image->GetImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(upload, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        device.endSingleTimeCommands(upload);
        static_cast<VkDescriptorImageInfo*>(image->GetDescriptorInfo())->imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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
        
        imageDescInfo = *((VkDescriptorImageInfo*)image->GetDescriptorInfo());
        imageDescInfo.sampler = textureSampler;
	}
}
