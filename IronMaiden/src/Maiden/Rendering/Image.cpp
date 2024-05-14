#include "maidenpch.hpp"
#include "H_Image.hpp"

namespace Madam {
	namespace Rendering {

		Image::Image(ImageSpecification imageSpecs) : width{ imageSpecs.width }, height{ imageSpecs.height }
		{
			specifications = imageSpecs;
			imageInfo = {};
			VkFormat vulkanFormat = specifications.GetFormat();
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = specifications.depth;
			imageInfo.mipLevels = specifications.mipLevels;
			imageInfo.arrayLayers = specifications.arrayLayers;
			imageInfo.format = vulkanFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = GetUsage(specifications.usuage, specifications.format);
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = 0;

			Renderer::GetDevice().createImageWithInfo(
				imageInfo,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				view.image,
				view.imageMemory);

			view.info = {};

			view.info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view.info.image = view.image;
			view.info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view.info.format = vulkanFormat;
			view.info.subresourceRange.aspectMask = specifications.GetAspect();
			view.info.subresourceRange.baseMipLevel = 0;
			view.info.subresourceRange.levelCount = specifications.mipLevels;
			view.info.subresourceRange.baseArrayLayer = 0;
			view.info.subresourceRange.layerCount = specifications.arrayLayers;
			if (vkCreateImageView(Renderer::GetDevice().device(), &view.info, nullptr, &view.view) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
			memory = view.imageMemory;
			image = view.image;
		}

		Image::~Image() {
			Destroy();
		}

		void Image::Destroy() {
			if (!isDestroyed) {
				vkDestroyImageView(Renderer::GetDevice().device(), view.view, nullptr);
				vkDestroyImage(Renderer::GetDevice().device(), image, nullptr);
				vkFreeMemory(Renderer::GetDevice().device(), memory, nullptr);
				isDestroyed = true;
			}
		}
	}
}