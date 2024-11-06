#include "maidenpch.hpp"
#include "H_RenderUtils.hpp"
#include "H_Renderer.hpp"

namespace Madam {
	namespace Rendering {

		static VkFormat GetFormat(RenderPassAttachmentType attachmentType) {
			MADAM_CORE_INFO("Get Format is being called");
			switch (attachmentType)
			{
			case Madam::Rendering::RenderPassAttachmentType::Color:
				return Renderer::GetSwapChain().getSwapChainImageFormat(); //Getting null error here?? Fixed, swapchain passing in format and uses different constructor
				break;
			case Madam::Rendering::RenderPassAttachmentType::Depth:
				return Renderer::GetDevice().findSupportedFormat(
					{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
					VK_IMAGE_TILING_OPTIMAL,
					VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				break;
			case Madam::Rendering::RenderPassAttachmentType::Stencil:
				return Renderer::GetDevice().findSupportedFormat(
					{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
					VK_IMAGE_TILING_OPTIMAL,
					VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				break;
			case Madam::Rendering::RenderPassAttachmentType::DepthStencil:
				return Renderer::GetDevice().findSupportedFormat(
					{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
					VK_IMAGE_TILING_OPTIMAL,
					VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				break;
			default:
				return Renderer::GetDevice().findSupportedFormat(
					{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
					VK_IMAGE_TILING_OPTIMAL,
					VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				break;
			}
		}

		static VkFormat GetFormat(ImageFormat format) {
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
		}
	}
}