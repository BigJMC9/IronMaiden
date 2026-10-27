#include "maidenpch.hpp"
#include "H_ShadowSystem.hpp"
#include "../Project/H_Project.h"
#include "../Scene/H_Entity.hpp"
#include "H_Mesh.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>

namespace Madam::Rendering
{
	namespace
	{
		glm::vec3 LightDirection(const glm::mat4& transform)
		{
			return glm::normalize(glm::vec3(transform * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
		}

		glm::vec3 LightUp(const glm::vec3& direction)
		{
			return std::abs(direction.y) > 0.98f ? glm::vec3(0.0f, 0.0f, 1.0f)
				: glm::vec3(0.0f, -1.0f, 0.0f);
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
			vkCmdPipelineBarrier(commandBuffer, sourceStage, targetStage, 0, 0, nullptr, 0,
				nullptr, 1, &barrier);
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

		glm::mat4 SpotProjection(float outerAngleDegrees, float range)
		{
			const float nearPlane = 0.1f;
			const float farPlane = std::max(range, nearPlane + 0.1f);
			const float tangent = std::tan(glm::radians(glm::clamp(outerAngleDegrees, 1.0f, 85.0f)));
			glm::mat4 projection(0.0f);
			projection[0][0] = 1.0f / tangent;
			projection[1][1] = 1.0f / tangent;
			projection[2][2] = farPlane / (farPlane - nearPlane);
			projection[2][3] = 1.0f;
			projection[3][2] = -(farPlane * nearPlane) / (farPlane - nearPlane);
			return projection;
		}

		uint64_t HashBytes(uint64_t hash, const void* data, size_t size)
		{
			const auto* bytes = static_cast<const uint8_t*>(data);
			for (size_t index = 0; index < size; ++index) {
				hash ^= bytes[index];
				hash *= 1099511628211ull;
			}
			return hash;
		}
	}

	void ShadowSystem::Init()
	{
		if (initialized) return;
		depthFormat = device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT }, VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

		VkAttachmentDescription attachment{};
		attachment.format = depthFormat;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
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
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &attachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();
		if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
			throw std::runtime_error("Could not create shadow render pass");

		VkPushConstantRange pushRange{};
		pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushRange.size = sizeof(ShadowPush);
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushRange;
		if (vkCreatePipelineLayout(device.device(), &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Could not create shadow pipeline layout");

		PipelineConfigInfo config{};
		Pipeline::setDescriptions(config);
		config.attributeDescriptions.resize(1); // Depth pass only reads vertex position.
		config.renderPass = renderPass;
		config.pipelineLayout = pipelineLayout;
		config.colorBlendInfo.attachmentCount = 0;
		config.colorBlendInfo.pAttachments = nullptr;
		config.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		config.rasterizationInfo.depthBiasEnable = VK_TRUE;
		config.rasterizationInfo.depthBiasConstantFactor = 1.25f;
		config.rasterizationInfo.depthBiasSlopeFactor = 1.75f;
		ShaderStageConfigInfo stages;
		stages.vertexModule = ShaderModuleConfigInfo("resources\\shaders\\shadow_map.vert.spv");
		pipeline = CreateScope<Pipeline>(device, config, stages);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		VkFilter shadowFilter = VK_FILTER_NEAREST;
		try {
			device.findSupportedFormat({ depthFormat }, VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
			shadowFilter = VK_FILTER_LINEAR;
		}
		catch (const std::runtime_error&) {
			MADAM_CORE_WARN("Linear depth filtering is unavailable; shadow edges will be sharp.");
		}
		samplerInfo.magFilter = shadowFilter;
		samplerInfo.minFilter = shadowFilter;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.compareEnable = VK_TRUE;
		samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		samplerInfo.maxLod = 1.0f;
		if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			throw std::runtime_error("Could not create shadow sampler");

		for (auto& map : realtimeMaps) CreateMap(map);
		CreateMap(bakedMap);
		bakeReadback = CreateScope<Buffer>(device, Resolution * Resolution * sizeof(float), 1,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (bakeReadback->Map() != VK_SUCCESS)
			throw std::runtime_error("Could not map shadow bake readback buffer");
		initialized = true;
	}

	void ShadowSystem::Shutdown()
	{
		bakeReadback.reset();
		for (auto& map : realtimeMaps) DestroyMap(map);
		DestroyMap(bakedMap);
		if (sampler) vkDestroySampler(device.device(), sampler, nullptr);
		sampler = VK_NULL_HANDLE;
		pipeline.reset();
		if (pipelineLayout) vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
		pipelineLayout = VK_NULL_HANDLE;
		if (renderPass) vkDestroyRenderPass(device.device(), renderPass, nullptr);
		renderPass = VK_NULL_HANDLE;
		realtimeValid.fill(false);
		drawList.clear();
		initialized = false;
	}

	void ShadowSystem::CreateMap(Map& map)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent = { Resolution, Resolution, 1 };
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
			throw std::runtime_error("Could not create shadow image view");
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &map.view;
		framebufferInfo.width = Resolution;
		framebufferInfo.height = Resolution;
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &map.framebuffer) != VK_SUCCESS)
			throw std::runtime_error("Could not create shadow framebuffer");
		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
		ImageBarrier(commandBuffer, map.image, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, VK_ACCESS_SHADER_READ_BIT);
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
		return { sampler, realtimeMaps.at(frameIndex).view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	}

	VkDescriptorImageInfo ShadowSystem::BakedDescriptor() const
	{
		return { sampler, bakedMap.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	}

	void ShadowSystem::RenderMap(Map& map, const glm::mat4& lightViewProjection,
		VkCommandBuffer commandBuffer)
	{
		VkClearValue clear{};
		clear.depthStencil = { 1.0f, 0 };
		VkRenderPassBeginInfo begin{};
		begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		begin.renderPass = renderPass;
		begin.framebuffer = map.framebuffer;
		begin.renderArea.extent = { Resolution, Resolution };
		begin.clearValueCount = 1;
		begin.pClearValues = &clear;
		vkCmdBeginRenderPass(commandBuffer, &begin, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport{ 0.0f, 0.0f, static_cast<float>(Resolution),
			static_cast<float>(Resolution), 0.0f, 1.0f };
		VkRect2D scissor{ { 0, 0 }, { Resolution, Resolution } };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		pipeline->bind(commandBuffer);
		for (const auto& draw : drawList)
		{
			ShadowPush push{};
			push.lightViewProjection = lightViewProjection;
			push.model = draw.model;
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
				0, sizeof(push), &push);
			draw.mesh->Bind(commandBuffer);
			draw.mesh->Draw(commandBuffer);
		}
		vkCmdEndRenderPass(commandBuffer);
	}

	void ShadowSystem::CopyBakeToBuffer(VkCommandBuffer commandBuffer)
	{
		ImageBarrier(commandBuffer, bakedMap.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT);
		VkBufferImageCopy copy{};
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		copy.imageSubresource.layerCount = 1;
		copy.imageExtent = { Resolution, Resolution, 1 };
		vkCmdCopyImageToBuffer(commandBuffer, bakedMap.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			bakeReadback->GetBuffer(), 1, &copy);
		ImageBarrier(commandBuffer, bakedMap.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
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
		if (!file || std::memcmp(header.magic, "IMSHADOW", 8) != 0 ||
			header.version != 2 || header.resolution != Resolution) return false;
		const size_t byteCount = Resolution * Resolution * sizeof(float);
		std::vector<char> pixels(byteCount);
		file.read(pixels.data(), byteCount);
		if (!file || file.peek() != EOF) return false;
		Buffer upload(device, byteCount, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (upload.Map() != VK_SUCCESS) return false;
		upload.WriteToBuffer(pixels.data(), byteCount);
		upload.Flush();
		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
		ImageBarrier(commandBuffer, bakedMap.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
		VkBufferImageCopy copy{};
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		copy.imageSubresource.layerCount = 1;
		copy.imageExtent = { Resolution, Resolution, 1 };
		vkCmdCopyBufferToImage(commandBuffer, upload.GetBuffer(), bakedMap.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
		ImageBarrier(commandBuffer, bakedMap.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		device.endSingleTimeCommands(commandBuffer);
		bakedMatrix = header.lightViewProjection;
		bakedCaster.assign(header.caster, std::find(header.caster, header.caster + sizeof(header.caster), '\0'));
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
			header.lightViewProjection = bakedMatrix;
			std::memcpy(header.caster, bakedCaster.data(),
				std::min(bakedCaster.size(), sizeof(header.caster) - 1));
			file.write(reinterpret_cast<const char*>(&header), sizeof(header));
			file.write(static_cast<const char*>(bakeReadback->GetMappedMemory()),
				Resolution * Resolution * sizeof(float));
			if (file) {
				MADAM_CORE_INFO("Shadow bake saved: {0}", pendingBakePath.string());
				if (ResolveBakePath(scene) == pendingBakePath)
					scene.GetLightingSettings().bakeStatus = "Shadow bake saved";
			}
			else error = std::make_error_code(std::errc::io_error);
		}
		if (error) {
			MADAM_CORE_ERROR("Could not save shadow bake: {0}", error.message());
			if (ResolveBakePath(scene) == pendingBakePath)
				scene.GetLightingSettings().bakeStatus = "Could not save shadow bake";
		}
		pendingReadback = false;
		pendingBakePath.clear();
	}

	void ShadowSystem::SyncBake(Scene& scene)
	{
		if (pendingReadback) FinalizeBake(scene);
		auto& lighting = scene.GetLightingSettings();
		if (lighting.shadowMode != ShadowMode::Baked || lighting.bakeRequested) return;
		const auto path = ResolveBakePath(scene);
		if (path == loadedBakePath) return;
		bakedValid = !path.empty() && LoadBake(path);
		loadedBakePath = path;
		if (!bakedValid && !path.empty()) {
			if (std::filesystem::exists(path)) {
				lighting.bakeRequested = true;
				lighting.bakeStatus = "Updating shadow bake...";
				MADAM_CORE_INFO("Updating outdated shadow bake: {0}", path.string());
			}
			else MADAM_CORE_WARN("No usable shadow bake found: {0}", path.string());
		}
	}

	ShadowSystem::FrameResult ShadowSystem::Prepare(Scene& scene, const RenderScene& renderScene, GlobalUbo& ubo,
		uint32_t frameIndex, VkCommandBuffer commandBuffer)
	{
		FrameResult result{};
		const auto& lighting = scene.GetLightingSettings();
		ubo.ambientLightColor = glm::vec4(lighting.ambientColor, lighting.ambientIntensity);
		ubo.shadowParams.x = std::max(0.00001f, lighting.shadowBias);
		const auto selectedCaster = static_cast<std::string>(lighting.shadowCaster);
		int selectedType = 0;
		int selectedIndex = 0;
		glm::vec3 selectedPosition(0.0f);
		glm::vec3 selectedDirection(0.0f, 1.0f, 0.0f);
		float selectedRange = 15.0f;
		float selectedAngle = 30.0f;
		std::string selectedUuid;
		auto& entities = *renderScene.registry;
		for (const auto entity : renderScene.pointLightEntities)
		{
			if (ubo.numLights >= MAX_LIGHTS) break;
			const auto& light = entities.get<CPointLight>(entity);
			if (!light.enabled) continue;
			const auto uuid = entities.get<CUniqueIdentifier>(entity).uuid;
			const auto world = scene.GetWorldTransform(uuid);
			auto& data = ubo.pointLights[ubo.numLights++];
			data.position = glm::vec4(glm::vec3(world[3]), std::max(0.1f, light.range));
			data.color = glm::vec4(light.color, std::max(0.0f, light.intensity));
		}
		for (const auto entity : renderScene.directionalLightEntities)
		{
			if (ubo.numDirectionalLights >= MAX_DIRECTIONAL_LIGHTS) break;
			const auto& light = entities.get<CDirectionalLight>(entity);
			if (!light.enabled) continue;
			const auto uuid = entities.get<CUniqueIdentifier>(entity).uuid;
			const auto world = scene.GetWorldTransform(uuid);
			const auto direction = LightDirection(world);
			const int index = ubo.numDirectionalLights++;
			auto& data = ubo.directionalLights[index];
			data.direction = glm::vec4(direction, 0.0f);
			data.color = glm::vec4(light.color, std::max(0.0f, light.intensity));
			if ((selectedCaster.empty() && selectedType == 0) || selectedCaster == static_cast<std::string>(uuid)) {
				selectedType = 1;
				selectedIndex = index;
				selectedDirection = direction;
				selectedUuid = static_cast<std::string>(uuid);
			}
		}
		for (const auto entity : renderScene.spotLightEntities)
		{
			if (ubo.numSpotLights >= MAX_SPOT_LIGHTS) break;
			const auto& light = entities.get<CSpotLight>(entity);
			if (!light.enabled) continue;
			const auto uuid = entities.get<CUniqueIdentifier>(entity).uuid;
			const auto world = scene.GetWorldTransform(uuid);
			const auto direction = LightDirection(world);
			const int index = ubo.numSpotLights++;
			auto& data = ubo.spotLights[index];
			data.position = glm::vec4(glm::vec3(world[3]), 1.0f);
			data.direction = glm::vec4(direction, 0.0f);
			data.color = glm::vec4(light.color, std::max(0.0f, light.intensity));
			const float outer = glm::clamp(light.outerAngle, 1.0f, 85.0f);
			const float inner = glm::clamp(light.innerAngle, 0.0f, outer - 0.1f);
			data.cone = glm::vec4(std::cos(glm::radians(inner)),
				std::cos(glm::radians(outer)), std::max(0.2f, light.range), 0.0f);
			if ((selectedCaster.empty() && selectedType == 0) || selectedCaster == static_cast<std::string>(uuid)) {
				selectedType = 2;
				selectedIndex = index;
				selectedPosition = glm::vec3(world[3]);
				selectedDirection = direction;
				selectedRange = std::max(0.2f, light.range);
				selectedAngle = outer;
				selectedUuid = static_cast<std::string>(uuid);
			}
		}
		if (selectedType == 0) {
			if (scene.GetLightingSettings().bakeRequested) {
				scene.GetLightingSettings().bakeRequested = false;
				scene.GetLightingSettings().bakeStatus = "Add or select an enabled directional or spot light";
			}
			return result;
		}
		if (lighting.shadowMode == ShadowMode::Disabled) return result;

		drawList.clear();
		drawList.reserve(renderScene.meshEntities.size());
		glm::vec3 minPoint(std::numeric_limits<float>::max());
		glm::vec3 maxPoint(std::numeric_limits<float>::lowest());
		uint64_t signature = 14695981039346656037ull;
		const auto sceneAddress = reinterpret_cast<uintptr_t>(&scene);
		signature = HashBytes(signature, &sceneAddress, sizeof(sceneAddress));
		signature = HashBytes(signature, selectedUuid.data(), selectedUuid.size());
		for (const auto entity : renderScene.meshEntities) {
			if (!entities.valid(entity)) continue;
			const auto& renderer = entities.get<CMeshRenderer>(entity);
			if (!renderer.GetMesh()) continue;
			const auto uuid = entities.get<CUniqueIdentifier>(entity).uuid;
			ShadowDraw draw{ renderer.GetMesh().get(), scene.GetWorldTransform(uuid) };
			minPoint = glm::min(minPoint, glm::vec3(draw.model[3]));
			maxPoint = glm::max(maxPoint, glm::vec3(draw.model[3]));
			drawList.push_back(draw);
			const auto entityId = static_cast<uint32_t>(entity);
			const auto meshAddress = reinterpret_cast<uintptr_t>(draw.mesh);
			signature = HashBytes(signature, &entityId, sizeof(entityId));
			signature = HashBytes(signature, &meshAddress, sizeof(meshAddress));
			signature = HashBytes(signature, glm::value_ptr(draw.model), sizeof(glm::mat4));
		}

		glm::mat4 lightMatrix(1.0f);
		if (selectedType == 1)
		{
			const glm::vec3 center = drawList.empty()
				? glm::vec3(0.0f) : (minPoint + maxPoint) * 0.5f;
			const float distance = std::max(5.0f, lighting.shadowDistance);
			lightMatrix = DirectionalProjection(distance) *
				glm::lookAtLH(center - selectedDirection * distance, center, LightUp(selectedDirection));
		}
		else
			lightMatrix = SpotProjection(selectedAngle, selectedRange) *
				glm::lookAtLH(selectedPosition, selectedPosition + selectedDirection, LightUp(selectedDirection));
		signature = HashBytes(signature, glm::value_ptr(lightMatrix), sizeof(glm::mat4));

		if (lighting.shadowMode == ShadowMode::Realtime)
		{
			if (!realtimeValid.at(frameIndex) || realtimeSignatures.at(frameIndex) != signature) {
				RenderMap(realtimeMaps.at(frameIndex), lightMatrix, commandBuffer);
				realtimeSignatures.at(frameIndex) = signature;
				realtimeValid.at(frameIndex) = true;
				result.drawCalls = static_cast<uint32_t>(drawList.size());
				result.rendered = true;
			}
			else result.cached = true;
			ubo.shadowViewProjection = lightMatrix;
			ubo.shadowInfo = glm::ivec4(1, selectedType, selectedIndex, 0);
		}
		else if (lighting.shadowMode == ShadowMode::Baked)
		{
			if (scene.GetLightingSettings().bakeRequested)
			{
				scene.GetLightingSettings().bakeRequested = false;
				pendingBakePath = ResolveBakePath(scene);
				if (pendingBakePath.empty()) {
					scene.GetLightingSettings().bakeStatus = "Save the scene before baking shadows";
				}
				else {
					RenderMap(bakedMap, lightMatrix, commandBuffer);
					CopyBakeToBuffer(commandBuffer);
					result.drawCalls = static_cast<uint32_t>(drawList.size());
					result.rendered = true;
					bakedMatrix = lightMatrix;
					bakedCaster = selectedUuid;
					bakedValid = true;
					loadedBakePath = pendingBakePath;
					pendingReadback = true;
					scene.GetLightingSettings().bakeStatus = "Baking shadows...";
				}
			}
			if (bakedValid && bakedCaster == selectedUuid &&
				loadedBakePath == ResolveBakePath(scene)) {
				ubo.shadowViewProjection = bakedMatrix;
				ubo.shadowInfo = glm::ivec4(2, selectedType, selectedIndex, 0);
				if (!result.rendered) result.cached = true;
			}
		}
		return result;
	}
}
