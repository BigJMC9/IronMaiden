#pragma once
#include "maidenpch.hpp"
#include "H_VulkanDevice.hpp"
#include "../H_Texture.h"
#include "H_VulkanImage.h"

namespace Madam
{
	class VulkanTexture : public Texture
	{
	public:
		VulkanTexture(const TextureData& textureData);
		VulkanTexture(const TextureData& textureData, const std::filesystem::path& filepath);
		~VulkanTexture() override;

		virtual void Bind() const override {}

		virtual void Resize(const glm::uvec2& size) override 
		{ 
			data.width = size.x;
			data.height = size.y;
		}
		virtual void Resize(uint32_t width, uint32_t height) override
		{
			data.width = width;
			data.height = height;
		}

		virtual Rendering::Image::Format GetFormat() const override { return data.format; }
		virtual uint32_t GetWidth() const override { return data.width; }
		virtual uint32_t GetHeight() const override { return data.height; }
		virtual uint32_t GetDepth() const override { return data.depth; }
		virtual glm::uvec2 GetSize() const override { return { data.width, data.height }; }
		virtual ResourceDescriptorInfo GetDescriptorInfo() const override { return (ResourceDescriptorInfo)&imageDescInfo; }

		const std::filesystem::path& GetFilepath() const override { return filepath; }
		uint32_t GetMipLevelCount() const override { return data.mipLevels; }
		//void GenerateMips();

		virtual void Load(Device& device) override;

		static VkSamplerAddressMode GetVkSamplerWrap(Rendering::TextureWrap wrap)
		{
			switch (wrap)
			{
			case Madam::Rendering::TextureWrap::Repeat:
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
				break;
			case Madam::Rendering::TextureWrap::Clamp:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				break;
			case Madam::Rendering::TextureWrap::Mirror:
				return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
				break;
			default:
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
				break;
			}
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}

		static VkFilter GetVkSamplerFilter(Rendering::TextureFilter filter)
		{
			switch (filter)
			{
			case Madam::Rendering::TextureFilter::Linear:
				return VK_FILTER_LINEAR;
				break;
			case Madam::Rendering::TextureFilter::Nearest:
				return VK_FILTER_NEAREST;
				break;
			case Madam::Rendering::TextureFilter::Cubic:
				return VK_FILTER_CUBIC_IMG;
				break;
			default:
				return VK_FILTER_LINEAR;
				break;
			}
			return VK_FILTER_LINEAR;
		}

	private:

		std::filesystem::path filepath;
		TextureData data;
		VkDescriptorImageInfo imageDescInfo;
		Ref<Rendering::VulkanImage> image = nullptr;
		VkSampler textureSampler = nullptr;
	};
}
