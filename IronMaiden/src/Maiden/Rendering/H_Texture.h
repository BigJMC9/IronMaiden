#pragma once

#include "maidenpch.hpp"
#include "Vulkan/H_VulkanDevice.hpp"
#include "../Asset/Asset.h"
#include "H_RIResource.h"
#include "H_RenderUtils.hpp"
#include "H_Image.hpp"

//libs
#include <vulkan/vulkan.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <filesystem>


namespace Madam {

	enum TextureFlags : uint32_t
	{
		NONE = 0,
		Error = 1 << 1,
		Loaded = 1 << 2,
	};

	/*inline TextureFlags operator|(TextureFlags lhs, TextureFlags rhs)
	{
		return static_cast<TextureFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}*/
	inline TextureFlags operator|(TextureFlags lhs, uint32_t rhs)
	{
		return static_cast<TextureFlags>(static_cast<uint32_t>(lhs) | rhs);
	}

	inline TextureFlags operator&(TextureFlags lhs, uint32_t rhs)
	{
		return static_cast<TextureFlags>(static_cast<uint32_t>(lhs) & rhs);
	}

	inline TextureFlags operator|=(TextureFlags& lhs, uint32_t rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	inline TextureFlags operator&=(TextureFlags& lhs, TextureFlags rhs)
	{
		lhs = lhs & rhs;
		return lhs;
	}

	inline TextureFlags operator&=(TextureFlags& lhs, uint32_t rhs)
	{
		lhs = lhs & rhs;
		return lhs;
	}

	/*inline TextureFlags operator|=(TextureFlags& lhs, TextureFlags rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}*/

	struct TextureData
	{
		Rendering::Image::Format format = Rendering::Image::Format::RGBA_SRGB;
		uint32_t width = 0, height = 0, depth = 1;
		uint32_t mipLevels = 1, arrayLayers = 1;
		Rendering::TextureWrap samplerWrap = Rendering::TextureWrap::Repeat;
		Rendering::TextureFilter samplerFilter = Rendering::TextureFilter::Linear;
		Rendering::Image::Multisample multisample = Rendering::Image::Multisample::None;

		bool SRGB = false;
		bool generateMips = true;
	};

	class Texture : public RenderResource
	{
	public:
		virtual ~Texture() {}

		static Ref<Texture> Create(const TextureData& textureData);
		static Ref<Texture> Create(const TextureData& textureData, const std::filesystem::path& filepath);

		virtual void Bind() const = 0;

		virtual Rendering::Image::Format GetFormat() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetDepth() const = 0;
		virtual glm::uvec2 GetSize() const = 0;

		virtual uint32_t GetMipLevelCount() const = 0;
			
		virtual void Resize(const glm::uvec2& size) = 0;
		virtual void Resize(const uint32_t width, const uint32_t height) = 0;

		virtual void Load(Device& device) = 0;

		virtual const std::filesystem::path& GetFilepath() const = 0;

		virtual uint32_t GetFlags() const { return (uint32_t)_flags; }
		virtual void SetFlags(uint32_t flags) { _flags |= flags; }

		static AssetType GetStaticType() { return AssetType::TEXTURE; }
		AssetType GetAssetType() const override { return GetStaticType(); }

	protected:
		TextureFlags _flags = TextureFlags::NONE;
	};
}