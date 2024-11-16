#pragma once
#include "maidenpch.hpp"
#include "../H_Renderer.hpp"
#include "../H_Image.hpp"

namespace Madam
{
	namespace Rendering
	{
		class VulkanImage : public Image
		{
		public:
			VulkanImage();
			VulkanImage(ImageData imageData);
			VulkanImage(VkImageCreateInfo imageInfo, VkImageViewCreateInfo viewInfo);
			virtual ~VulkanImage() override;

			virtual void Resize(const uint32_t width, const uint32_t height) override
			{
				data.width = width;
				data.height = height;
			}

			virtual void Resize(const uint32_t width, const uint32_t height, const uint32_t depth) override
			{
				data.width = width;
				data.height = height;
				data.depth = depth;
			}

			virtual float GetAspectRatio() const override
			{
				return static_cast<float>(data.width) / data.height;
			}

			virtual uint32_t GetWidth() const override { return data.width; }
			virtual uint32_t GetHeight() const override { return data.height; }
			virtual uint32_t GetDepth() const override { return data.depth; }
			virtual ResourceDescriptorInfo GetDescriptorInfo() const override { return (ResourceDescriptorInfo)&imageDescInfo; }

			VkImage& GetImage() { return image; }

			VkImageView& GetImageView() { return imageView; }



			static void TransitionImageLayout(Device& device, VulkanImage& image, VkImageLayout srcLayout, VkImageLayout dstLayout);

			static void CopyBufferToImage(Device& device, VkBuffer buffer, VulkanImage& image);

			static VkFormat GetVkFormat(Image::Format format) {
				switch (format)
				{
				case Madam::Rendering::Image::Format::None:
					return VK_FORMAT_UNDEFINED;
					break;
				case Madam::Rendering::Image::Format::RGB:
					return VK_FORMAT_R8G8B8_UNORM;
					break;
				case Madam::Rendering::Image::Format::RGB_SRGB:
					return VK_FORMAT_R8G8B8_SRGB;
					break;
				case Madam::Rendering::Image::Format::RGB16F:
					return VK_FORMAT_R16G16B16_SFLOAT;
					break;
				case Madam::Rendering::Image::Format::RGB32F:
					return VK_FORMAT_R32G32B32_SFLOAT;
					break;
				case Madam::Rendering::Image::Format::RGBA:
					return VK_FORMAT_R8G8B8A8_UNORM;
					break;
				case Madam::Rendering::Image::Format::RGBA_SRGB:
					return VK_FORMAT_R8G8B8A8_SRGB;
					break;
				case Madam::Rendering::Image::Format::RGBA16F:
					return VK_FORMAT_R16G16B16A16_SFLOAT;
					break;
				case Madam::Rendering::Image::Format::RGBA32F:
					return VK_FORMAT_R32G32B32A32_SFLOAT;
					break;
				case Madam::Rendering::Image::Format::BGRA:
					return VK_FORMAT_B8G8R8A8_UNORM;
					break;
				case Madam::Rendering::Image::Format::BGRA_SRGB:
					return VK_FORMAT_B8G8R8A8_SRGB;
					break;
				case Madam::Rendering::Image::Format::D32F:
					return VK_FORMAT_D32_SFLOAT;
					break;
				case Madam::Rendering::Image::Format::D32FS8:
					return VK_FORMAT_D32_SFLOAT_S8_UINT;
					break;
				case Madam::Rendering::Image::Format::D24S8:
					return VK_FORMAT_D24_UNORM_S8_UINT;
					break;
				default:
					break;
				}
				return VK_FORMAT_UNDEFINED;
			}

			static Image::Format GetFormat(VkFormat format)
			{
				switch (format)
				{
				case VK_FORMAT_UNDEFINED:
					return Image::Format::None;
					break;
				case VK_FORMAT_R8G8B8_UNORM:
					return Image::Format::RGB;
					break;
				case VK_FORMAT_R8G8B8_SRGB:
					return Image::Format::RGB_SRGB;
					break;
				case VK_FORMAT_R16G16B16_SFLOAT:
					return Image::Format::RGB16F;
					break;
				case VK_FORMAT_R32G32B32_SFLOAT:
					return Image::Format::RGB32F;
					break;
				case VK_FORMAT_R8G8B8A8_UNORM:
					return Image::Format::RGBA;
					break;
				case VK_FORMAT_R8G8B8A8_SRGB:
					return Image::Format::RGBA_SRGB;
					break;
				case VK_FORMAT_R16G16B16A16_SFLOAT:
					return Image::Format::RGBA16F;
					break;
				case VK_FORMAT_R32G32B32A32_SFLOAT:
					return Image::Format::RGBA32F;
					break;
				case VK_FORMAT_B8G8R8A8_UNORM:
					return Image::Format::BGRA;
					break;
				case VK_FORMAT_B8G8R8A8_SRGB:
					return Image::Format::BGRA_SRGB;
				case VK_FORMAT_D32_SFLOAT:
					return Image::Format::D32F;
					break;
				case VK_FORMAT_D32_SFLOAT_S8_UINT:
					return Image::Format::D32FS8;
					break;
				case VK_FORMAT_D24_UNORM_S8_UINT:
					return Image::Format::D24S8;
					break;
				default:
					return Image::Format::None;
					break;
				}
				return Image::Format::None;
			}

			static VkSampleCountFlagBits GetVkSampleCount(Image::Multisample multisample)
			{
				switch (multisample)
				{
				case Madam::Rendering::Image::Multisample::None:
					return VK_SAMPLE_COUNT_1_BIT;
					break;
				case Madam::Rendering::Image::Multisample::MSAAx2:
					return VK_SAMPLE_COUNT_2_BIT;
					break;
				case Madam::Rendering::Image::Multisample::MSAAx4:
					return VK_SAMPLE_COUNT_4_BIT;
					break;
				case Madam::Rendering::Image::Multisample::MSAAx8:
					return VK_SAMPLE_COUNT_8_BIT;
					break;
				default:
					return VK_SAMPLE_COUNT_1_BIT;
					break;
				}

				return VK_SAMPLE_COUNT_1_BIT;
			}

			static Image::Multisample GetSampleCount(VkSampleCountFlagBits multisample)
			{
				switch (multisample)
				{
				case VK_SAMPLE_COUNT_1_BIT:
					return Madam::Rendering::Image::Multisample::None;
					break;
				case VK_SAMPLE_COUNT_2_BIT:
					return Madam::Rendering::Image::Multisample::MSAAx2;
					break;
				case VK_SAMPLE_COUNT_4_BIT:
					return Madam::Rendering::Image::Multisample::MSAAx4;
					break;
				case VK_SAMPLE_COUNT_8_BIT:
					return Madam::Rendering::Image::Multisample::MSAAx8;
					break;
				case VK_SAMPLE_COUNT_16_BIT:
					return Madam::Rendering::Image::Multisample::MSAAx8;
					break;
				case VK_SAMPLE_COUNT_32_BIT:
					return Madam::Rendering::Image::Multisample::MSAAx8;
					break;
				case VK_SAMPLE_COUNT_64_BIT:
					return Madam::Rendering::Image::Multisample::MSAAx8;
					break;
				case VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM:
					return Madam::Rendering::Image::Multisample::MSAAx8;
					break;
				default:
					return Madam::Rendering::Image::Multisample::None;
					break;
				}
				return Madam::Rendering::Image::Multisample::None;
			}

			static VkImageAspectFlags GetVkAspectMask(Image::Format format) {
				switch (format)
				{
				case Madam::Rendering::Image::Format::D32F:
					return VK_IMAGE_ASPECT_DEPTH_BIT;
					break;
				case Madam::Rendering::Image::Format::D32FS8:
					return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
					break;
				case Madam::Rendering::Image::Format::D24S8:
					return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
					break;
				default:
					return VK_IMAGE_ASPECT_COLOR_BIT;
					break;
				}
			}

		protected:
			VkImageView imageView;
			VkImage image;
			VkDeviceMemory imageMemory;
			VkDescriptorImageInfo imageDescInfo;
			//VkImageLayout imageLayout;
			bool isDestroyed = false;

			friend class SwapChain;
			friend class Renderer;
		};
	}
}