#pragma once
#include "maidenpch.hpp"
#include "Vulkan/H_VulkanDevice.hpp"

namespace Madam {
	namespace Rendering {

		enum class TextureWrap {
			Repeat = 0,
			Clamp = 1,
			Mirror = 2
		};

		enum class TextureFilter
		{
			Linear = 0,
			Nearest = 1,
			Cubic = 2
		};

		enum class MultiSample {
			None = VK_SAMPLE_COUNT_1_BIT,
			MSAAx2 = VK_SAMPLE_COUNT_2_BIT,
			MSAAx4 = VK_SAMPLE_COUNT_4_BIT,
			MSAAx8 = VK_SAMPLE_COUNT_8_BIT
		};

		enum class ImageFormat {
			None = 0,
			RGB,
			RGBA,
			RGBA16F,
			RGBA32F,
			SRGB,
			D32F,
			D32FS8,
			D24S8
		};

		enum class ImageLayoutType {
			Undefined = 0,
			Color,
			DepthStencil,
			Shader,
			Present
		};

		enum class ImageUsageType {
			None = 0,
			Texture,
			Attachment,
			Storage,
			HostRead
		};

		enum class LoadType
		{
			DontCare = 0,
			Inherit = 1,
			Clear = 2,
			Load = 3
		};

		enum class StoreType
		{
			DontCare = 0,
			Store = 1,
		};

		enum class RenderPassAttachmentType
		{
			Color = 0,
			Depth = 1,
			Stencil = 2,
			DepthStencil = 3
		};

		struct RenderpassAttachment {

		};

		constexpr VkAttachmentLoadOp GetLoadOp(LoadType loadType) {
			switch (loadType)
			{
			case Madam::Rendering::LoadType::Inherit:
				return VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			case Madam::Rendering::LoadType::Clear:
				return VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			case Madam::Rendering::LoadType::Load:
				return VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			default:
				return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
			}
		}

		constexpr VkAttachmentStoreOp GetStoreOp(StoreType storeType) {
			switch (storeType)
			{
			case Madam::Rendering::StoreType::DontCare:
				return VK_ATTACHMENT_STORE_OP_DONT_CARE;
				break;
			case Madam::Rendering::StoreType::Store:
				return VK_ATTACHMENT_STORE_OP_STORE;
				break;
			default:
				return VK_ATTACHMENT_STORE_OP_DONT_CARE;
				break;
			}
		}

		static VkFormat GetFormat(RenderPassAttachmentType attachmentType);
		static VkFormat GetFormat(ImageFormat imageFormat);
	}
}