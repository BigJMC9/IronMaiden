#include "maidenpch.hpp"
#include "H_VulkanImage.h"

namespace Madam
{
	namespace Rendering
	{
		VulkanImage::VulkanImage()
		{

		}

		VulkanImage::VulkanImage(ImageData imageData)
		{
			data = imageData;
			VkImageAspectFlags aspectMask = 0;
			VkImageLayout imageLayout;
			VkImageUsageFlags usage = 0x00000000;
			VkExtent3D extent;
			extent.width = data.width;
			extent.height = data.height;
			extent.depth = data.depth;

			if (data.usage == Image::Usage::Attachment)
			{
				if (data.format == Image::Format::D32F || data.format == Image::Format::D32FS8 || data.format == Image::Format::D24S8)
				{
					usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				}
				else
				{
					usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				}
			}
			if (data.usage == Image::Usage::Texture)
			{
				usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			}
			if (data.usage == Image::Usage::Storage)
			{
				usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}

			VkFormat vulkanFormat = GetVkFormat(data.format);
			VkImageCreateInfo imageInfo;
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = extent.width;
			imageInfo.extent.height = extent.height;
			imageInfo.extent.depth = extent.depth;
			imageInfo.mipLevels = data.mipLevels;
			imageInfo.arrayLayers = data.arrayLayers;
			imageInfo.format = vulkanFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //Update this, should be previous image layout
			imageInfo.usage = usage;
			imageInfo.queueFamilyIndexCount = 0;
			imageInfo.pQueueFamilyIndices = nullptr;
			imageInfo.samples = GetVkSampleCount(data.multisample);
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = 0;
			imageInfo.pNext = nullptr;

			Renderer::GetDevice().createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = vulkanFormat;
			viewInfo.subresourceRange.aspectMask = GetVkAspectMask(data.format);
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = data.mipLevels;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = data.arrayLayers;
			if (vkCreateImageView(Renderer::GetDevice().device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}

			imageDescInfo.imageView = imageView;
			imageDescInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		VulkanImage::VulkanImage(VkImageCreateInfo imageInfo, VkImageViewCreateInfo viewInfo)
		{
			Renderer::GetDevice().createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

			viewInfo.image = image;

			if (vkCreateImageView(Renderer::GetDevice().device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}

			data.format = GetFormat(imageInfo.format);
			data.mipLevels = imageInfo.mipLevels;
			data.arrayLayers = imageInfo.arrayLayers;
			data.multisample = GetSampleCount(imageInfo.samples);
			data.width = imageInfo.extent.width;
			data.height = imageInfo.extent.height;
			data.depth = imageInfo.extent.depth;
		}

		VulkanImage::~VulkanImage()
		{
			if (!isDestroyed)
			{
				vkDestroyImageView(Renderer::GetDevice().device(), imageView, nullptr);
				vkDestroyImage(Renderer::GetDevice().device(), image, nullptr);
				vkFreeMemory(Renderer::GetDevice().device(), imageMemory, nullptr);
				isDestroyed = true;
			}
		}

		void VulkanImage::TransitionImageLayout(Device& device, VulkanImage& image, VkImageLayout srcLayout, VkImageLayout dstLayout)
		{
			device.transitionImageLayout(image.image, GetVkFormat(image.data.format), srcLayout, dstLayout, image.data.mipLevels, image.data.arrayLayers);
			image.imageDescInfo.imageLayout = dstLayout;
		}

		void VulkanImage::CopyBufferToImage(Device& device, VkBuffer buffer, VulkanImage& image)
		{
			device.copyBufferToImage(buffer, image.image, image.data.width, image.data.height, image.data.arrayLayers);
		}
	}
}