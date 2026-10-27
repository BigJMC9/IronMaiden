#include "maidenpch.hpp"
#include "H_ShadowSystem.hpp"
#include "../Project/H_Project.h"
#include "../Scene/H_Entity.hpp"
#include "H_Mesh.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <optional>

namespace Madam::Rendering
{
	namespace
	{
		glm::vec3 LightDirection(const glm::mat4& transform)
		{
			return glm::normalize(glm::vec3(transform * glm::vec4(0, 0, 1, 0)));
		}

		glm::vec3 LightUp(const glm::vec3& direction)
		{
			return std::abs(direction.y) > 0.98f ? glm::vec3(0, 0, 1) : glm::vec3(0, -1, 0);
		}

		glm::mat4 DirectionalProjection(float distance)
		{
			const float nearPlane = 0.1f;
			const float farPlane = distance * 2.0f;
			glm::mat4 projection(1.0f);
			projection[0][0] = 2.0f / distance;
			projection[1][1] = -2.0f / distance;
			projection[2][2] = 1.0f / (farPlane - nearPlane);
			projection[3][2] = -nearPlane / (farPlane - nearPlane);
			return projection;
		}

		glm::mat4 PerspectiveProjection(float outerAngleDegrees, float range)
		{
			const float nearPlane = 0.1f;
			const float farPlane = std::max(range, nearPlane + 0.1f);
			const float tangent = std::tan(glm::radians(glm::clamp(outerAngleDegrees, 1.0f, 89.0f)));
			glm::mat4 projection(0.0f);
			projection[0][0] = 1.0f / tangent;
			projection[1][1] = 1.0f / tangent;
			projection[2][2] = farPlane / (farPlane - nearPlane);
			projection[2][3] = 1.0f;
			projection[3][2] = -(farPlane * nearPlane) / (farPlane - nearPlane);
			return projection;
		}

		void ImageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout,
			VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags targetStage,
			VkAccessFlags sourceAccess, VkAccessFlags targetAccess)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = sourceAccess;
			barrier.dstAccessMask = targetAccess;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(commandBuffer, sourceStage, targetStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		uint64_t HashBytes(uint64_t hash, const void* data, size_t size)
		{
			const auto* bytes = static_cast<const uint8_t*>(data);
			for (size_t index = 0; index < size; ++index)
			{
				hash ^= bytes[index];
				hash *= 1099511628211ull;
			}
			return hash;
		}

		struct AtlasAllocator
		{
			static constexpr uint32_t Cells = ShadowSystem::AtlasSize / ShadowSystem::MinimumTile;
			uint32_t activeCells = Cells;
			explicit AtlasAllocator(uint32_t size = ShadowSystem::AtlasSize)
				: activeCells(size / ShadowSystem::MinimumTile) {}
			std::array<uint16_t, Cells> occupiedRows{};
			std::optional<std::pair<uint32_t, uint32_t>> Allocate(uint32_t resolution)
			{
				const uint32_t side = resolution / ShadowSystem::MinimumTile;
				for (uint32_t y = 0; y + side <= activeCells; ++y)
					for (uint32_t x = 0; x + side <= activeCells; ++x)
					{
						const auto mask = static_cast<uint16_t>(((1u << side) - 1u) << x);
						bool free = true;
						for (uint32_t row = 0; row < side && free; ++row)
							if (occupiedRows[y + row] & mask) free = false;
						if (!free) continue;
						for (uint32_t row = 0; row < side; ++row)
							occupiedRows[y + row] |= mask;
						return std::make_pair(x * ShadowSystem::MinimumTile, y * ShadowSystem::MinimumTile);
					}
				return std::nullopt;
			}
		};
	}

	void ShadowSystem::Init()
	{
		if (initialized) return;
		depthFormat = device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT }, VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
		VkAttachmentDescription attachment{};
		attachment.format = depthFormat;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkAttachmentReference depthReference{ 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pDepthStencilAttachment = &depthReference;
		std::array<VkSubpassDependency, 2> dependencies{};
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &attachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();
		if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
			throw std::runtime_error("Could not create shadow atlas render pass");

		VkPushConstantRange pushRange{};
		pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushRange.size = sizeof(ShadowPush);
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushRange;
		if (vkCreatePipelineLayout(device.device(), &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Could not create shadow atlas pipeline layout");
		PipelineConfigInfo config{};
		Pipeline::setDescriptions(config);
		config.attributeDescriptions.resize(1);
		config.renderPass = renderPass;
		config.pipelineLayout = pipelineLayout;
		config.colorBlendInfo.attachmentCount = 0;
		config.colorBlendInfo.pAttachments = nullptr;
		config.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		config.rasterizationInfo.depthBiasEnable = VK_TRUE;
		config.dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		config.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(config.dynamicStateEnables.size());
		config.dynamicStateInfo.pDynamicStates = config.dynamicStateEnables.data();
		ShaderStageConfigInfo stages;
		stages.vertexModule = ShaderModuleConfigInfo("resources\\shaders\\shadow_map.vert.spv");
		pipeline = CreateScope<Pipeline>(device, config, stages);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.compareEnable = VK_TRUE;
		samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		samplerInfo.maxLod = 1.0f;
		if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &compareSampler) != VK_SUCCESS)
			throw std::runtime_error("Could not create shadow comparison sampler");
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &depthSampler) != VK_SUCCESS)
			throw std::runtime_error("Could not create shadow depth sampler");
		for (auto& atlas : realtimeAtlases) CreateMap(atlas, AtlasSize);
		for (auto& atlas : directionalAtlases) CreateMap(atlas, DirectionalAtlasSize);
		CreateMap(bakedAtlas, AtlasSize);
		bakeReadback = CreateScope<Buffer>(device, static_cast<VkDeviceSize>(AtlasSize) * AtlasSize * sizeof(float), 1,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (bakeReadback->Map() != VK_SUCCESS)
			throw std::runtime_error("Could not map shadow atlas readback buffer");
		initialized = true;
	}

	void ShadowSystem::Shutdown()
	{
		bakeReadback.reset();
		for (auto& atlas : realtimeAtlases) DestroyMap(atlas);
		for (auto& atlas : directionalAtlases) DestroyMap(atlas);
		DestroyMap(bakedAtlas);
		if (compareSampler) vkDestroySampler(device.device(), compareSampler, nullptr);
		if (depthSampler) vkDestroySampler(device.device(), depthSampler, nullptr);
		compareSampler = depthSampler = VK_NULL_HANDLE;
		pipeline.reset();
		if (pipelineLayout) vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
		pipelineLayout = VK_NULL_HANDLE;
		if (renderPass) vkDestroyRenderPass(device.device(), renderPass, nullptr);
		renderPass = VK_NULL_HANDLE;
		initialized = false;
	}

	void ShadowSystem::CreateMap(Map& map, uint32_t size)
	{
		map.size = size;
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent = { size, size, 1 };
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, map.image, map.memory);
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = map.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(device.device(), &viewInfo, nullptr, &map.view) != VK_SUCCESS)
			throw std::runtime_error("Could not create shadow atlas image view");
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &map.view;
		framebufferInfo.width = size;
		framebufferInfo.height = size;
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &map.framebuffer) != VK_SUCCESS)
			throw std::runtime_error("Could not create shadow atlas framebuffer");
		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
		ImageBarrier(commandBuffer, map.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
		VkClearDepthStencilValue clear{ 1.0f, 0 };
		VkImageSubresourceRange range{};
		range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		range.levelCount = 1;
		range.layerCount = 1;
		vkCmdClearDepthStencilImage(commandBuffer, map.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1, &range);
		ImageBarrier(commandBuffer, map.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		device.endSingleTimeCommands(commandBuffer);
	}

	void ShadowSystem::DestroyMap(Map& map)
	{
		if (map.framebuffer) vkDestroyFramebuffer(device.device(), map.framebuffer, nullptr);
		if (map.view) vkDestroyImageView(device.device(), map.view, nullptr);
		if (map.image) vkDestroyImage(device.device(), map.image, nullptr);
		if (map.memory) vkFreeMemory(device.device(), map.memory, nullptr);
		map = {};
	}

	VkDescriptorImageInfo ShadowSystem::RealtimeDescriptor(uint32_t frameIndex) const
	{
		return { compareSampler, realtimeAtlases.at(frameIndex).view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	}
	VkDescriptorImageInfo ShadowSystem::BakedDescriptor() const
	{
		return { compareSampler, bakedAtlas.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	}
	VkDescriptorImageInfo ShadowSystem::RealtimeDepthDescriptor(uint32_t frameIndex) const
	{
		return { depthSampler, realtimeAtlases.at(frameIndex).view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	}
	VkDescriptorImageInfo ShadowSystem::BakedDepthDescriptor() const
	{
		return { depthSampler, bakedAtlas.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	}
	VkDescriptorImageInfo ShadowSystem::DirectionalDescriptor(uint32_t frameIndex) const
	{
		return { compareSampler, directionalAtlases.at(frameIndex).view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	}
	VkDescriptorImageInfo ShadowSystem::DirectionalDepthDescriptor(uint32_t frameIndex) const
	{
		return { depthSampler, directionalAtlases.at(frameIndex).view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	}

	void ShadowSystem::RenderTile(Map& map, Tile tile, const glm::mat4& lightViewProjection,
		const LightShadowSettings& settings, VkCommandBuffer commandBuffer)
	{
		VkRenderPassBeginInfo begin{};
		begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		begin.renderPass = renderPass;
		begin.framebuffer = map.framebuffer;
		begin.renderArea = { { static_cast<int32_t>(tile.x), static_cast<int32_t>(tile.y) },
			{ tile.size, tile.size } };
		vkCmdBeginRenderPass(commandBuffer, &begin, VK_SUBPASS_CONTENTS_INLINE);
		VkClearAttachment attachment{};
		attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		attachment.clearValue.depthStencil = { 1.0f, 0 };
		VkClearRect clearRect{};
		clearRect.rect = begin.renderArea;
		clearRect.layerCount = 1;
		vkCmdClearAttachments(commandBuffer, 1, &attachment, 1, &clearRect);
		VkViewport viewport{ static_cast<float>(tile.x), static_cast<float>(tile.y),
			static_cast<float>(tile.size), static_cast<float>(tile.size), 0.0f, 1.0f };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &begin.renderArea);
		pipeline->bind(commandBuffer);
		vkCmdSetDepthBias(commandBuffer, settings.advancedBias ? settings.depthBias * 1000.0f : 1.25f,
			0.0f, settings.advancedBias ? settings.slopeBias : 1.75f);
		for (const auto& draw : drawList)
		{
			ShadowPush push{};
			push.lightViewProjection = lightViewProjection;
			push.model = draw.model;
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push), &push);
			draw.mesh->Bind(commandBuffer);
			draw.mesh->Draw(commandBuffer);
		}
		vkCmdEndRenderPass(commandBuffer);
	}

	void ShadowSystem::CopyBakeToBuffer(VkCommandBuffer commandBuffer)
	{
		ImageBarrier(commandBuffer, bakedAtlas.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT);
		VkBufferImageCopy copy{};
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		copy.imageSubresource.layerCount = 1;
		copy.imageExtent = { AtlasSize, AtlasSize, 1 };
		vkCmdCopyImageToBuffer(commandBuffer, bakedAtlas.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			bakeReadback->GetBuffer(), 1, &copy);
		ImageBarrier(commandBuffer, bakedAtlas.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT);
	}

	std::filesystem::path ShadowSystem::ResolveBakePath(const Scene& scene)
	{
		if (!Project::HasActive()) return {};
		const auto& relative = scene.GetLightingSettings().bakeFile;
		if (relative.empty() || relative.is_absolute()) return {};
		for (const auto& part : relative)
			if (part == "..") return {};
		return Project::Get().getProjectDirectory() / relative;
	}

	bool ShadowSystem::LoadBake(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file) return false;
		BakeHeader header{};
		file.read(reinterpret_cast<char*>(&header), sizeof(header));
		if (!file || std::memcmp(header.magic, "IMSHADOW", 8) != 0 || header.version != 4 ||
			header.atlasSize != AtlasSize || header.recordCount > MAX_SHADOW_RECORDS) return false;
		std::vector<BakeRecord> records(header.recordCount);
		file.read(reinterpret_cast<char*>(records.data()), records.size() * sizeof(BakeRecord));
		const size_t byteCount = static_cast<size_t>(AtlasSize) * AtlasSize * sizeof(float);
		std::vector<char> pixels(byteCount);
		file.read(pixels.data(), byteCount);
		if (!file || file.peek() != EOF) return false;
		Buffer upload(device, byteCount, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (upload.Map() != VK_SUCCESS) return false;
		upload.WriteToBuffer(pixels.data(), byteCount);
		upload.Flush();
		vkDeviceWaitIdle(device.device());
		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
		ImageBarrier(commandBuffer, bakedAtlas.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
		VkBufferImageCopy copy{};
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		copy.imageSubresource.layerCount = 1;
		copy.imageExtent = { AtlasSize, AtlasSize, 1 };
		vkCmdCopyBufferToImage(commandBuffer, upload.GetBuffer(), bakedAtlas.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
		ImageBarrier(commandBuffer, bakedAtlas.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		device.endSingleTimeCommands(commandBuffer);
		bakedAtlas.cached.clear();
		for (const auto& record : records)
		{
			if (record.tile.size < MinimumTile || record.tile.x + record.tile.size > AtlasSize ||
				record.tile.y + record.tile.size > AtlasSize) return false;
			const std::string key(record.key, std::find(record.key, record.key + sizeof(record.key), '\0'));
			bakedAtlas.cached[key] = { record.tile, 0, record.matrix };
		}
		return true;
	}

	void ShadowSystem::FinalizeBake(Scene& scene)
	{
		vkDeviceWaitIdle(device.device());
		bakeReadback->Invalidate();
		std::error_code error;
		std::filesystem::create_directories(pendingBakePath.parent_path(), error);
		if (!error)
		{
			std::ofstream file(pendingBakePath, std::ios::binary | std::ios::trunc);
			BakeHeader header{};
			header.recordCount = static_cast<uint32_t>(bakedAtlas.cached.size());
			file.write(reinterpret_cast<const char*>(&header), sizeof(header));
			for (const auto& [key, cached] : bakedAtlas.cached)
			{
				BakeRecord record{};
				std::memcpy(record.key, key.data(), std::min(key.size(), sizeof(record.key) - 1));
				record.tile = cached.tile;
				record.matrix = cached.matrix;
				file.write(reinterpret_cast<const char*>(&record), sizeof(record));
			}
			file.write(static_cast<const char*>(bakeReadback->GetMappedMemory()),
				static_cast<size_t>(AtlasSize) * AtlasSize * sizeof(float));
			if (file)
			{
				MADAM_CORE_INFO("Shadow atlas bake saved: {0}", pendingBakePath.string());
				if (ResolveBakePath(scene) == pendingBakePath)
					scene.GetLightingSettings().bakeStatus = "Shadow atlas bake saved";
			}
			else error = std::make_error_code(std::errc::io_error);
		}
		if (error)
		{
			MADAM_CORE_ERROR("Could not save shadow atlas bake: {0}", error.message());
			if (ResolveBakePath(scene) == pendingBakePath)
				scene.GetLightingSettings().bakeStatus = "Could not save shadow atlas bake";
		}
		pendingReadback = false;
		pendingBakePath.clear();
	}

	void ShadowSystem::SyncBake(Scene& scene)
	{
		if (pendingReadback) FinalizeBake(scene);
		auto& lighting = scene.GetLightingSettings();
		if (lighting.bakeRequested) return;
		const auto path = ResolveBakePath(scene);
		if (path == loadedBakePath) return;
		bakedAtlas.cached.clear();
		const bool loaded = !path.empty() && LoadBake(path);
		loadedBakePath = path;
		if (!loaded && !path.empty() && std::filesystem::exists(path))
		{
			lighting.bakeRequested = true;
			lighting.bakeStatus = "Updating shadow atlas bake...";
		}
	}

	ShadowSystem::FrameResult ShadowSystem::Prepare(Scene& scene, const RenderScene& renderScene,
		GlobalUbo& ubo, uint32_t frameIndex, VkCommandBuffer commandBuffer)
	{
		FrameResult result{};
		auto& lighting = scene.GetLightingSettings();
		ubo.ambientLightColor = glm::vec4(lighting.ambientColor, lighting.ambientIntensity);
		struct Candidate
		{
			int type = 0; // directional 1, spot 2, point 3
			int index = 0;
			std::string uuid;
			LightShadowSettings settings{};
			glm::vec3 position{ 0.0f };
			glm::vec3 direction{ 0.0f, 1.0f, 0.0f };
			float range = 15.0f;
			float angle = 30.0f;
			float radius = 0.5f;
			float intensity = 1.0f;
			float score = 0.0f;
		};
		std::vector<Candidate> candidates;
		candidates.reserve(MAX_LIGHTS + MAX_DIRECTIONAL_LIGHTS + MAX_SPOT_LIGHTS);
		const glm::vec3 cameraPosition = glm::vec3(ubo.inverseView[3]);
		const glm::vec3 cameraForward = glm::normalize(glm::vec3(ubo.inverseView[2]));
		const auto selectedCaster = static_cast<std::string>(lighting.shadowCaster);
		const auto addCandidate = [&](Candidate candidate)
		{
			candidate.score = candidate.settings.priority == ShadowPriority::Automatic
				? 220.0f + 10.0f * candidate.intensity + 0.1f * candidate.range
				: 100.0f * static_cast<int>(candidate.settings.priority);
			if (candidate.type == 1) candidate.score += 20.0f;
			if (candidate.uuid == selectedCaster) candidate.score += 1000.0f;
			candidates.push_back(std::move(candidate));
		};
		auto& entities = *renderScene.registry;
		for (const auto entity : renderScene.pointLightEntities)
		{
			if (ubo.numLights >= MAX_LIGHTS) break;
			const auto& light = entities.get<CPointLight>(entity);
			if (!light.enabled) continue;
			const auto uuid = entities.get<CUniqueIdentifier>(entity).uuid;
			const auto position = glm::vec3(scene.GetWorldTransform(uuid)[3]);
			const int index = ubo.numLights++;
			ubo.pointLights[index].position = glm::vec4(position, std::max(0.1f, light.range));
			ubo.pointLights[index].color = glm::vec4(light.color, std::max(0.0f, light.intensity));
			addCandidate({ 3, index, static_cast<std::string>(uuid), light.shadows,
				position, {}, std::max(0.2f, light.range), 45.0f, light.radius, light.intensity });
		}
		for (const auto entity : renderScene.directionalLightEntities)
		{
			if (ubo.numDirectionalLights >= MAX_DIRECTIONAL_LIGHTS) break;
			const auto& light = entities.get<CDirectionalLight>(entity);
			if (!light.enabled) continue;
			const auto uuid = entities.get<CUniqueIdentifier>(entity).uuid;
			const auto direction = LightDirection(scene.GetWorldTransform(uuid));
			const int index = ubo.numDirectionalLights++;
			ubo.directionalLights[index].direction = glm::vec4(direction, 0.0f);
			ubo.directionalLights[index].color = glm::vec4(light.color, std::max(0.0f, light.intensity));
			addCandidate({ 1, index, static_cast<std::string>(uuid), light.shadows,
				{}, direction, 0.0f, 0.0f, 0.0f, light.intensity });
		}
		for (const auto entity : renderScene.spotLightEntities)
		{
			if (ubo.numSpotLights >= MAX_SPOT_LIGHTS) break;
			const auto& light = entities.get<CSpotLight>(entity);
			if (!light.enabled) continue;
			const auto uuid = entities.get<CUniqueIdentifier>(entity).uuid;
			const auto world = scene.GetWorldTransform(uuid);
			const auto position = glm::vec3(world[3]);
			const auto direction = LightDirection(world);
			const int index = ubo.numSpotLights++;
			auto& data = ubo.spotLights[index];
			data.position = glm::vec4(position, 1.0f);
			data.direction = glm::vec4(direction, 0.0f);
			data.color = glm::vec4(light.color, std::max(0.0f, light.intensity));
			const float outer = glm::clamp(light.outerAngle, 1.0f, 85.0f);
			const float inner = glm::clamp(light.innerAngle, 0.0f, outer - 0.1f);
			data.cone = glm::vec4(std::cos(glm::radians(inner)), std::cos(glm::radians(outer)),
				std::max(0.2f, light.range), 0.0f);
			addCandidate({ 2, index, static_cast<std::string>(uuid), light.shadows,
				position, direction, std::max(0.2f, light.range), outer, 0.0f, light.intensity });
		}
		if (lighting.shadowMode == ShadowMode::Disabled)
		{
			if (lighting.bakeRequested)
			{
				lighting.bakeRequested = false;
				lighting.bakeStatus = "Enable shadows before baking";
			}
			return result;
		}
		std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b)
		{
			if (a.score != b.score) return a.score > b.score;
			return a.uuid < b.uuid;
		});
		struct MeshInput
		{
			ShadowDraw draw;
			uint32_t entityId = 0;
			bool isStatic = false;
			bool castRealtime = false;
			bool contributeBaked = false;
		};
		std::vector<MeshInput> meshes;
		meshes.reserve(renderScene.meshEntities.size());
		for (const auto entity : renderScene.meshEntities)
		{
			if (!entities.valid(entity)) continue;
			const auto& renderer = entities.get<CMeshRenderer>(entity);
			if (!renderer.mesh || !renderer.castShadows) continue;
			const auto uuid = entities.get<CUniqueIdentifier>(entity).uuid;
			meshes.push_back({ { renderer.mesh.get(), scene.GetWorldTransform(uuid) },
				static_cast<uint32_t>(entity), renderer.mobility == CMeshRenderer::Mobility::Static,
				renderer.castRealtimeShadows,
				renderer.contributeToBakedLighting });
		}

		bool bakeNow = lighting.bakeRequested;
		if (bakeNow)
		{
			lighting.bakeRequested = false;
			pendingBakePath = ResolveBakePath(scene);
			if (pendingBakePath.empty())
			{
				bakeNow = false;
				lighting.bakeStatus = "Save the scene before baking shadows";
			}
			else
			{
				bakedAtlas.cached.clear();
				lighting.bakeStatus = "Baking shadow atlas...";
			}
		}
		AtlasAllocator realtimeAllocation, bakedAllocation;
		AtlasAllocator directionalAllocation(DirectionalAtlasSize);
		struct TileGroup
		{
			std::array<Tile, 6> slots{};
			bool valid = false;
			bool empty() const { return !valid; }
			const Tile& operator[](size_t index) const { return slots[index]; }
		};
		const auto allocateGroup = [&](AtlasAllocator& allocator, uint32_t requested, uint32_t count,
			bool cascades) -> TileGroup
		{
			uint32_t baseSize = AtlasSize;
			while (baseSize > requested && baseSize > MinimumTile) baseSize >>= 1;
			for (; baseSize >= MinimumTile; baseSize >>= 1)
			{
				auto trial = allocator;
				TileGroup tiles;
				uint32_t allocated = 0;
				for (uint32_t face = 0; face < count; ++face)
				{
					const uint32_t size = cascades && face > 0 ? std::max(MinimumTile, baseSize / 2) : baseSize;
					auto position = trial.Allocate(size);
					if (!position) break;
					tiles.slots[face] = { position->first, position->second, size };
					++allocated;
				}
				if (allocated == count)
				{
					allocator = trial;
					tiles.valid = true;
					return tiles;
				}
			}
			return {};
		};
		const auto gather = [&](const Candidate& candidate, bool baked, bool dynamicOnly)
		{
			drawList.clear();
			uint64_t signature = 14695981039346656037ull;
			const auto sceneAddress = reinterpret_cast<uintptr_t>(&scene);
			signature = HashBytes(signature, &sceneAddress, sizeof(sceneAddress));
			signature = HashBytes(signature, candidate.uuid.data(), candidate.uuid.size());
			for (const auto& mesh : meshes)
			{
				if (baked && (!mesh.isStatic || !mesh.contributeBaked)) continue;
				if (!baked && mesh.isStatic && (!mesh.castRealtime || dynamicOnly)) continue;
				drawList.push_back(mesh.draw);
				const auto meshAddress = reinterpret_cast<uintptr_t>(mesh.draw.mesh);
				signature = HashBytes(signature, &mesh.entityId, sizeof(mesh.entityId));
				signature = HashBytes(signature, &meshAddress, sizeof(meshAddress));
				signature = HashBytes(signature, glm::value_ptr(mesh.draw.model), sizeof(glm::mat4));
			}
			return signature;
		};
		const auto matrixFor = [&](const Candidate& candidate, uint32_t face, Tile tile, bool baked)
		{
			if (candidate.type == 3)
			{
				static const glm::vec3 directions[6] = {
					{ 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 },
					{ 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
				static const glm::vec3 ups[6] = {
					{ 0, -1, 0 }, { 0, -1, 0 }, { 0, 0, -1 },
					{ 0, 0, 1 }, { 0, -1, 0 }, { 0, -1, 0 } };
				return PerspectiveProjection(45.0f, candidate.range) * glm::lookAtLH(
					candidate.position, candidate.position + directions[face], ups[face]);
			}
			if (candidate.type == 2)
				return PerspectiveProjection(candidate.angle, candidate.range) * glm::lookAtLH(
					candidate.position, candidate.position + candidate.direction, LightUp(candidate.direction));
			const float distance = std::max(5.0f, lighting.shadowDistance);
			const float split = baked ? distance : distance * (face == 0 ? 0.18f : face == 1 ? 0.45f : 1.0f);
			const float previous = baked || face == 0 ? 0.0f : distance * (face == 1 ? 0.18f : 0.45f);
			glm::vec3 center = cameraPosition + cameraForward * ((split + previous) * 0.5f);
			if (baked)
			{
				glm::vec3 minimum(std::numeric_limits<float>::max());
				glm::vec3 maximum(std::numeric_limits<float>::lowest());
				for (const auto& draw : drawList)
				{
					minimum = glm::min(minimum, glm::vec3(draw.model[3]));
					maximum = glm::max(maximum, glm::vec3(draw.model[3]));
				}
				if (!drawList.empty()) center = (minimum + maximum) * 0.5f;
			}
			const glm::vec3 right = glm::normalize(glm::cross(LightUp(candidate.direction), candidate.direction));
			const glm::vec3 up = glm::normalize(glm::cross(candidate.direction, right));
			const float worldTexel = std::max(split, 5.0f) / tile.size;
			center += right * (std::round(glm::dot(center, right) / worldTexel) * worldTexel - glm::dot(center, right));
			center += up * (std::round(glm::dot(center, up) / worldTexel) * worldTexel - glm::dot(center, up));
			return DirectionalProjection(std::max(split, 5.0f)) * glm::lookAtLH(
				center - candidate.direction * distance, center, LightUp(candidate.direction));
		};
		const auto indicesFor = [&](const Candidate& candidate) -> glm::ivec4&
		{
			if (candidate.type == 1) return ubo.directionalShadowIndices[candidate.index];
			if (candidate.type == 2) return ubo.spotShadowIndices[candidate.index];
			return ubo.pointShadowIndices[candidate.index];
		};

		bool bakedAny = false;
		bool waitedForBakedWrite = false;
		for (const auto& candidate : candidates)
		{
			const bool stationary = candidate.settings.mobility == LightMobility::Stationary;
			// Stationary lights always update dynamic casters in the realtime atlas.
			const bool wantsRealtime = candidate.settings.mode != LightShadowMode::Baked || stationary;
			const bool wantsBaked = candidate.settings.mode != LightShadowMode::Realtime || stationary;
			for (int kind = 0; kind < 2; ++kind)
			{
				const bool baked = kind == 1;
				if (baked ? !wantsBaked : !wantsRealtime) continue;
				const uint32_t count = candidate.type == 3 ? 6 :
					candidate.type == 1 && !baked ? CascadeCount : 1;
				if (ubo.shadowCounts.x + static_cast<int>(count) > MAX_SHADOW_RECORDS)
				{
					++result.droppedLights;
					continue;
				}
				const bool directionalRealtime = candidate.type == 1 && !baked;
				AtlasAllocator& allocation = baked ? bakedAllocation
					: directionalRealtime ? directionalAllocation : realtimeAllocation;
				auto tiles = allocateGroup(allocation,
					candidate.settings.resolution, count, candidate.type == 1);
				if (tiles.empty())
				{
					++result.droppedLights;
					continue;
				}
				const bool dynamicOnly = !baked && stationary;
				auto signature = gather(candidate, baked, dynamicOnly);
				signature = HashBytes(signature, glm::value_ptr(glm::vec4(candidate.position, 1.0f)), sizeof(glm::vec4));
				signature = HashBytes(signature, glm::value_ptr(glm::vec4(candidate.direction, 0.0f)), sizeof(glm::vec4));
				signature = HashBytes(signature, &candidate.range, sizeof(candidate.range));
				signature = HashBytes(signature, &candidate.angle, sizeof(candidate.angle));
				signature = HashBytes(signature, &candidate.settings.bias, sizeof(candidate.settings.bias));
				signature = HashBytes(signature, &candidate.settings.depthBias, sizeof(candidate.settings.depthBias));
				signature = HashBytes(signature, &candidate.settings.slopeBias, sizeof(candidate.settings.slopeBias));
				signature = HashBytes(signature, &candidate.settings.advancedBias, sizeof(candidate.settings.advancedBias));
				if (candidate.type == 1)
					signature = HashBytes(signature, &lighting.shadowDistance, sizeof(lighting.shadowDistance));
				if (candidate.type == 1 && !baked)
				{
					signature = HashBytes(signature, glm::value_ptr(glm::vec4(cameraPosition, 1.0f)), sizeof(glm::vec4));
					signature = HashBytes(signature, glm::value_ptr(glm::vec4(cameraForward, 0.0f)), sizeof(glm::vec4));
				}
				Map& atlas = baked ? bakedAtlas : directionalRealtime
					? directionalAtlases.at(frameIndex) : realtimeAtlases.at(frameIndex);
				const bool runtimeStaticCache = baked && stationary;
				std::array<glm::mat4, 6> matrices{};
				bool usable = true;
				for (uint32_t face = 0; face < count; ++face)
				{
					const auto key = candidate.uuid + ":" + std::to_string(face);
					const auto found = atlas.cached.find(key);
					const bool validTile = found != atlas.cached.end() && found->second.tile == tiles[face];
					if (baked && !bakeNow && !runtimeStaticCache && !validTile)
					{
						lighting.bakeStatus = "Shadow atlas layout changed; bake again";
						usable = false;
						break;
					}
					auto drawSignature = HashBytes(signature, &face, sizeof(face));
					drawSignature = HashBytes(drawSignature, &tiles[face], sizeof(Tile));
					const bool mustRender = baked
						? bakeNow || (runtimeStaticCache && (!validTile || found->second.signature != drawSignature))
						: !validTile || found->second.signature != drawSignature ||
							(candidate.settings.mobility == LightMobility::Static && !candidate.settings.cacheRuntimeShadows);
					const auto matrix = mustRender ? matrixFor(candidate, face, tiles[face], baked)
						: found->second.matrix;
					matrices[face] = matrix;
					if (mustRender)
					{
						if (baked && !waitedForBakedWrite)
						{
							vkDeviceWaitIdle(device.device());
							waitedForBakedWrite = true;
						}
						RenderTile(atlas, tiles[face], matrix, candidate.settings, commandBuffer);
						atlas.cached[key] = { tiles[face], drawSignature, matrix };
						result.drawCalls += static_cast<uint32_t>(drawList.size());
						result.rendered = true;
					}
					else result.cached = true;
				}
				if (!usable) continue;
				glm::ivec4& indices = indicesFor(candidate);
				const int base = ubo.shadowCounts.x;
				if (baked) { indices.y = base + 1; indices.w = static_cast<int>(count); bakedAny = true; }
				else { indices.x = base + 1; indices.z = static_cast<int>(count); }
				for (uint32_t face = 0; face < count; ++face)
				{
					auto& record = ubo.shadows[ubo.shadowCounts.x++];
					record.viewProjection = matrices[face];
					record.atlasRect = glm::vec4(tiles[face].x, tiles[face].y,
							tiles[face].size, tiles[face].size) / static_cast<float>(atlas.size);
					record.biasFilter = glm::vec4(candidate.settings.advancedBias
						? candidate.settings.depthBias : candidate.settings.bias,
						candidate.settings.advancedBias ? candidate.settings.slopeBias : 1.75f,
						candidate.settings.advancedBias ? candidate.settings.normalBias : 0.0f, 1.0f);
					const float cascadeFar = candidate.type == 1 && !baked
						? std::max(5.0f, lighting.shadowDistance) * (face == 0 ? 0.18f : face == 1 ? 0.45f : 1.0f)
						: std::max(5.0f, lighting.shadowDistance);
					record.options = glm::vec4(cascadeFar, candidate.radius > 0.0f ? candidate.radius : 1.5f,
						1.0f / atlas.size, 0.0f);
					record.meta = glm::ivec4(directionalRealtime ? 2 : kind,
						static_cast<int>(candidate.settings.filtering),
						static_cast<int>(face), static_cast<int>(candidate.settings.mobility));
				}
				result.activeTiles += count;
			}
		}
		if (bakeNow)
		{
			if (bakedAny)
			{
				CopyBakeToBuffer(commandBuffer);
				pendingReadback = true;
			}
			else
			{
				pendingBakePath.clear();
				lighting.bakeStatus = "No baked shadow lights fit in the atlas";
			}
		}
		return result;
	}
}
