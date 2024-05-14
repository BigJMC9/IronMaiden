#pragma once
#include "maidenpch.hpp"
#include "H_Renderer.hpp"
#include "../Core/H_Utils.hpp"
#include "H_RenderUtils.hpp"

namespace Madam {
	namespace Rendering {
		struct ImageSpecification {
			uint32_t width = 0, height = 0, depth = 1;
			uint32_t mipLevels = 1, arrayLayers = 1;
			MultiSample multiSample = MultiSample::None;
			
			ImageFormat format = ImageFormat::RGBA;
			ImageLayoutType layout = ImageLayoutType::Color;
			ImageUsageType usuage = ImageUsageType::None;
			bool transfer = false;
			//bool isSwapChain = false;

			VkFormat GetFormat() {
				switch (format)
				{
				case Madam::Rendering::ImageFormat::None:
					MADAM_CORE_ERROR("Image format not specified");
					break;
				case Madam::Rendering::ImageFormat::RGB:
					return VK_FORMAT_R8G8B8_UNORM;
					break;
				case Madam::Rendering::ImageFormat::RGBA:
					return VK_FORMAT_R8G8B8A8_UNORM;
					break;
				case Madam::Rendering::ImageFormat::RGBA16F:
					return VK_FORMAT_R16G16B16A16_SFLOAT;
					break;
				case Madam::Rendering::ImageFormat::RGBA32F:
					return VK_FORMAT_R32G32B32A32_SFLOAT;
					break;
				case Madam::Rendering::ImageFormat::SRGB:
					return VK_FORMAT_R8G8B8A8_SRGB;
					break;
				case Madam::Rendering::ImageFormat::D32F:
					return VK_FORMAT_D32_SFLOAT;
					break;
				case Madam::Rendering::ImageFormat::D32FS8:
					return VK_FORMAT_D32_SFLOAT_S8_UINT;
					break;
				case Madam::Rendering::ImageFormat::D24S8:
					return VK_FORMAT_D24_UNORM_S8_UINT;
					break;
				default:
					MADAM_CORE_ERROR("Image format not specified");
					return VK_FORMAT_UNDEFINED;
					break;
				}

				return VK_FORMAT_UNDEFINED;
			}

			VkImageLayout GetLayout() {
				switch (layout)
				{
				case Madam::Rendering::ImageLayoutType::Undefined:
					return VK_IMAGE_LAYOUT_UNDEFINED;
					break;
				case Madam::Rendering::ImageLayoutType::Color:
					return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					break;
				case Madam::Rendering::ImageLayoutType::DepthStencil:
					return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					break;
				case Madam::Rendering::ImageLayoutType::Shader:
					return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					break;
				case Madam::Rendering::ImageLayoutType::Present:
					return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					break;
				default:
					MADAM_CORE_ERROR("Image layout not specified");
					return VK_IMAGE_LAYOUT_UNDEFINED;
					break;
				}

				return VK_IMAGE_LAYOUT_UNDEFINED;
			}

			VkImageAspectFlags GetAspect() {
				switch (format)
				{
				case Madam::Rendering::ImageFormat::D32F:
					return VK_IMAGE_ASPECT_DEPTH_BIT;
					break;
				case Madam::Rendering::ImageFormat::D32FS8:
					return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
					break;
				case Madam::Rendering::ImageFormat::D24S8:
					return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
					break;
				default:
					return VK_IMAGE_ASPECT_COLOR_BIT;
					break;
				}
			}
		};

		struct ImageView {
			VkImageView view;
			VkImage image;
			VkDeviceMemory imageMemory;
			VkImageViewCreateInfo info;
		};

		class Image
		{
		public:
			Image(ImageSpecification imageSpecs);
			~Image();

		inline int getWidth() const { return width; }
		inline int getHeight() const { return height; }

			uint32_t width = 0, height = 0;
			VkImageCreateInfo imageInfo;
			VkImage image = NULL;
			ImageView view;
			ImageSpecification specifications;
			VkDeviceMemory memory = NULL;
			bool isDestroyed = false;

			void Destroy();			

			VkImageUsageFlags GetUsage(ImageUsageType usage, ImageFormat format) {
				switch (usage)
				{
				case Madam::Rendering::ImageUsageType::None:
					MADAM_CORE_ERROR("Image set to 0 (None)");
					return 0;
					break;
				case Madam::Rendering::ImageUsageType::Texture:
					return VK_IMAGE_USAGE_SAMPLED_BIT;
					break;
				case Madam::Rendering::ImageUsageType::Attachment:
					if (format == ImageFormat::D32F || format == ImageFormat::D32FS8 || format == ImageFormat::D24S8)
						return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
					else
						return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
					break;
				case Madam::Rendering::ImageUsageType::Storage:
					return VK_IMAGE_USAGE_STORAGE_BIT;
					break;
				case Madam::Rendering::ImageUsageType::HostRead:
					return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
					break;
				default:
					MADAM_CORE_ERROR("Image usage not specified");
					return 0;
					break;
				}

				return 0;
			}

			friend class SwapChain;
			friend class Renderer;
		};

		
	}
}