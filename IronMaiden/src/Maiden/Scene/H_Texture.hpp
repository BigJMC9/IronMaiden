#pragma once

#include "maidenpch.hpp"
#include "../Rendering/Vulkan/H_VulkanDevice.hpp"
#include "../Asset/Asset.h"

//libs
#include <vulkan/vulkan.h>

#include <filesystem>

namespace Madam {

	class Texture : public Asset {
	public:
		Texture(Device& device, const std::string& textureFilepath);
		Texture(Device& device, VkFormat format, VkExtent3D extent, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount);

		~Texture() override;

		//Texture(const Texture&) = delete;
		//Texture& operator=(const Texture&) = delete;

		VkImageView imageView() const { return mTextureImageView; }
		VkSampler sampler() const { return mTextureSampler; }
		VkImage getImage() const { return mTextureImage; }
		VkImageView getImageView() const { return mTextureImageView; }
		VkDescriptorImageInfo getImageInfo() const { return mDescriptor; }
		VkImageLayout getImageLayout() const { return mTextureLayout; }
		VkExtent3D getExtent() const { return mExtent; }
		VkFormat getFormat() const { return mFormat; }
		std::string getFile() const { return file; }

		bool loaded() { return true; }

		void updateDescriptor();
		void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

		static Scope<Texture> createTextureFromFile(
			Device& device,
			const std::string& rawFilePath
		);

		static Ref<Texture> createTexture(
			Device& device,
			const std::string& rawFilePath
		);

		static AssetType GetStaticType() { return AssetType::TEXTURE; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

	private:
		void createTextureImage(const std::string& filePath);
		void createTextureImageView(VkImageViewType viewType);
		void createTextureSampler();

		VkDescriptorImageInfo mDescriptor{};

		Device& mDevice;
		VkImage mTextureImage = nullptr;
		VkDeviceMemory mTextureImageMemory = nullptr;
		VkImageView mTextureImageView = nullptr;
		VkSampler mTextureSampler = nullptr;
		VkFormat mFormat;
		VkImageLayout mTextureLayout;
		uint32_t mMipLevels{ 1 };
		uint32_t mLayerCount{ 1 };
		VkExtent3D mExtent{};

		std::string file;
	};
}