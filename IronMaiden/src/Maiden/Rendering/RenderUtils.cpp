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
	}
}