#include "maidenpch.hpp"
#include "H_RenderSystems.hpp"
#include "../Core/H_Application.hpp"
#include "Vulkan/ShaderCompilation/H_VulkanShaderCompiler.h"
#include "../Asset/H_ShaderDefinition.h"
#include "../Asset/H_AssetSystem.h"
#include "../Project/H_Project.h"

//May need overhaul since a pipeline may need to be created for each shader
//Each shader needs a pipeline. Shader variants can use the same pipeline but ubershader has to be used which can cause a performance hit to the GPU.
//More research required
//Maybe have vkCmdSequences built onto class with builder and then have it execute in order for rendersystems?
//Or could have CmdSequences handled in pipeline via a method by default but can override with custom sequence?

//UI rendersystem is billboard quad with texture. see (pg 49)

//Refactor to remove Device
namespace Madam {
	namespace Rendering {
		static bool MaterialTexturesReady(const Ref<CMaterial>& material)
		{
			if (!material)
				return false;
			if (!material->shaderDriven)
				return material->diffuse_map && material->normal_map &&
					material->ambient_occlusion_map && material->gloss_map;
			if (!material->shaderValid)
				return false;
			if (material->textureBindings.size() != material->expectedTextureCount)
				return false;
			for (const auto& [binding, texture] : material->textureBindings)
				if (!texture) return false;
			return true;
		}
		/*
		------------------Render System------------------
		*/

		RenderLayer::RenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderLayer(device, renderPass, globalSetLayout, std::move(_name), true) {}

		RenderLayer::RenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout,
			std::string _name, bool initializeBasePipeline)
			: device{ device }, name{ std::move(_name) } {
			if (initializeBasePipeline)
			{
				createPipelineLayout(globalSetLayout);
				createPipeline(renderPass);
			}
		}

		RenderLayer::~RenderLayer() {
			oldPipeline.reset();
			if (oldPipelineLayout != VK_NULL_HANDLE)
				vkDestroyPipelineLayout(device.device(), oldPipelineLayout, nullptr);
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
			pipelineLayout = VK_NULL_HANDLE;
		}

        void RenderLayer::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = sizeof(DefaultPushConstantData);

			std::vector<VkDescriptorSetLayout> descriptorSetLayout{ globalSetLayout };

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
			pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
			if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create pipeline layout!");
			}
        }

        void RenderLayer::createPipeline(VkRenderPass renderPass) {
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

			PipelineConfigInfo pipelineConfig{};
			Pipeline::setDescriptions(pipelineConfig);
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;

			ShaderStageConfigInfo shaderStageConfigInfo;
			shaderStageConfigInfo.vertexModule = ShaderModuleConfigInfo("resources\\shaders\\simple_shader.vert.spv");
			shaderStageConfigInfo.fragmentModule = ShaderModuleConfigInfo("resources\\shaders\\simple_shader.frag.spv");

			pipeline = CreateScope<Pipeline>(device, pipelineConfig, shaderStageConfigInfo);
        }

		void RenderLayer::updatePipelineState()
		{
			if (oldPipeline != nullptr)
			{
				if (tick >= 5)
				{
					vkDestroyPipelineLayout(device.device(), oldPipelineLayout, nullptr);
					oldPipelineLayout = VK_NULL_HANDLE;
					oldPipeline = nullptr;
					tick = 0;
				}
				else
				{
					tick++;
				}
			}
		}

		void RenderLayer::recreatePipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		{
			oldPipeline = std::move(pipeline);
			oldPipelineLayout = pipelineLayout;
			pipeline = nullptr;
			pipelineLayout = VK_NULL_HANDLE;
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		void RenderLayer::render(FrameInfo& frameInfo) {
			pipeline->bind(frameInfo.commandBuffer);

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				0,
				1,
				&frameInfo.globalDescriptorSet,
				0,
				nullptr);
			if (!frameInfo.renderScene.registry)
			{
				isFirstFrame = false;
				return;
			}

			entt::registry& entities = *frameInfo.renderScene.registry;
			for (auto entity : frameInfo.renderScene.meshEntities)
			{
				if (!entities.valid(entity)) {
					std::cerr << "Error, entity is not valid" << std::endl;
					continue;
				}

				auto& meshRenderer = entities.get<CMeshRenderer>(entity);
				if (meshRenderer.GetMesh() == nullptr) continue;
				const auto material = meshRenderer.GetMaterial();
				if (meshRenderer.materialAsset == null)
				{
					if (entities.any_of<CMaterial>(entity) || material) continue;
				}
				else if (MaterialTexturesReady(material))
					continue;

				DefaultPushConstantData push{};
				UUID uuid = entities.get<CUniqueIdentifier>(entity).uuid;
				push.modelMatrix = frameInfo.scene->GetWorldTransform(uuid);
				//push.modelMatrix[3][1] = -push.modelMatrix[3][1];
				push.normalMatrix = glm::transpose(glm::inverse(push.modelMatrix));
				push.normalMatrix[3][0] = meshRenderer.mobility == CMeshRenderer::Mobility::Static ? 1.0f : 0.0f;
				push.normalMatrix[3][1] = meshRenderer.receiveRealtimeShadows ? 1.0f : 0.0f;

				vkCmdPushConstants(
					frameInfo.commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(DefaultPushConstantData),
					&push);
				meshRenderer.GetMesh()->Bind(frameInfo.commandBuffer);
				meshRenderer.GetMesh()->Draw(frameInfo.commandBuffer);
			}
			isFirstFrame = false;
		}

		/*
		------------------Custom Render Layer-----------------
		*/

		CustomLayer::CustomLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderLayer(device, renderPass, globalSetLayout, _name, false) {
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		CustomLayer::~CustomLayer() {
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
			pipelineLayout = VK_NULL_HANDLE;
		}

		void CustomLayer::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = sizeof(Q13Constants);

			//Change for different layout
			std::vector<VkDescriptorSetLayout> descriptorSetLayout
			{
				globalSetLayout
			};

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
			pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
			if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create pipeline layout!");
			}
		}

		void CustomLayer::createPipeline(VkRenderPass renderPass) {
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

			PipelineConfigInfo pipelineConfig{};
			Pipeline::setDescriptions(pipelineConfig);

			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;

			ShaderStageConfigInfo shaderStageConfigInfo;
			shaderStageConfigInfo.vertexModule = ShaderModuleConfigInfo("resources\\shaders\\Q13.vert.spv");
			shaderStageConfigInfo.fragmentModule = ShaderModuleConfigInfo("resources\\shaders\\Q13.frag.spv");

			pipeline = CreateScope<Pipeline>(device, pipelineConfig, shaderStageConfigInfo);
		}

		void CustomLayer::render(FrameInfo& frameInfo) {
			pipeline->bind(frameInfo.commandBuffer);

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				0,
				1,
				&frameInfo.globalDescriptorSet,
				0,
				nullptr);

			if (!frameInfo.renderScene.registry)
			{
				return;
			}

			entt::registry& entities = *frameInfo.renderScene.registry;
			for (auto entity : frameInfo.renderScene.meshEntities) {

				if (!entities.valid(entity)) {
					std::cerr << "Error, entity is not valid" << std::endl;
					continue;
				}

				auto& meshRenderer = entities.get<CMeshRenderer>(entity);
				CMaterial material;
				if (entities.any_of<CMaterial>(entity))
				{
					material = entities.get<CMaterial>(entity);
				}
				else
				{
					continue;
				}
				if (material.is_custom == false) continue;

				Q13Constants push{};
				UUID uuid = entities.get<CUniqueIdentifier>(entity).uuid;
				push.modelMatrix = frameInfo.scene->GetWorldTransform(uuid);
				push.normalMatrix = glm::transpose(glm::inverse(push.modelMatrix));
				//MADAM_CORE_INFO("ax: {0}, dx0: {1}, dx1: {2}", material.ax, material.dx0, material.dx1);
				push.dx0 = material.dx0;
				push.dx1 = material.dx1;
				push.dx2 = material.dx2;
				push.dx3 = material.dx3;
				vkCmdPushConstants(
					frameInfo.commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(Q13Constants),
					&push);
				meshRenderer.GetMesh()->Bind(frameInfo.commandBuffer);
				meshRenderer.GetMesh()->Draw(frameInfo.commandBuffer);
			}
			isFirstFrame = false;
		}

		/*
		------------------Grid Render System------------------
		*/

		GridRenderLayer::GridRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderLayer(device, renderPass, globalSetLayout, _name, false) {
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		GridRenderLayer::~GridRenderLayer() {
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
			pipelineLayout = VK_NULL_HANDLE;
		}

		void GridRenderLayer::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = sizeof(GridPushConstants);

			//Change for different layout
			std::vector<VkDescriptorSetLayout> descriptorSetLayout
			{
				globalSetLayout
			};

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
			pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
			if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create pipeline layout!");
			}
		}

		void GridRenderLayer::createPipeline(VkRenderPass renderPass) {
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");


			PipelineConfigInfo pipelineConfig{};
			Pipeline::setDescriptions(pipelineConfig);
			pipelineConfig.enableAlphaBlending();
			// The scene target is opaque. Preserve its alpha while blending grid lines.
			pipelineConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			pipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			pipelineConfig.attributeDescriptions.clear();
			pipelineConfig.bindingDescriptions.clear();
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;
			pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
			pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
			pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			pipelineConfig.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
			pipelineConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;
			//pipelineConfig.rasterizationInfo.depthClampEnable = VK_TRUE;

			ShaderStageConfigInfo shaderStageConfigInfo;
			shaderStageConfigInfo.vertexModule = ShaderModuleConfigInfo("resources\\shaders\\grid_shader.vert.spv");
			shaderStageConfigInfo.fragmentModule = ShaderModuleConfigInfo("resources\\shaders\\grid_shader.frag.spv");

			pipeline = CreateScope<Pipeline>(device, pipelineConfig, shaderStageConfigInfo);
		}

		void GridRenderLayer::render(FrameInfo& frameInfo) {
			pipeline->bind(frameInfo.commandBuffer);

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				0,
				1,
				&frameInfo.globalDescriptorSet,
				0,
				nullptr);

			GridPushConstants push{};
			push.nearPlane = CameraHandle::GetMain().GetCameraData().perspective.near;
			push.farPlane = CameraHandle::GetMain().GetCameraData().perspective.far;
			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(GridPushConstants),
				&push);
			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
			isFirstFrame = false;
		}

		/*
		------------------Skybox Render System------------------
		*/

		SkyboxRenderLayer::SkyboxRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderLayer(device, renderPass, globalSetLayout, _name, false) {
			TextureData textureData;
			noiseTexture = Texture::Create(textureData, std::filesystem::u8path("resources\\textures\\skybox.png"));
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		SkyboxRenderLayer::~SkyboxRenderLayer() {
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
			pipelineLayout = VK_NULL_HANDLE;
		}

		void SkyboxRenderLayer::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

			skyboxRenderSystemLayout =
				DescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

			//Change for different layout
			std::vector<VkDescriptorSetLayout> descriptorSetLayout
			{
				globalSetLayout,
				skyboxRenderSystemLayout->getDescriptorSetLayout()
			};

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
			pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;
			if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create pipeline layout!");
			}

			skyboxDescriptorSet = VK_NULL_HANDLE;
		}

		void SkyboxRenderLayer::createPipeline(VkRenderPass renderPass) {
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");


			PipelineConfigInfo pipelineConfig{};
			Pipeline::setDescriptions(pipelineConfig);
			//Pipeline::enableAlphaBlending(pipelineConfig);

			//pipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;
			pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
			pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
			pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			pipelineConfig.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
			pipelineConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;
			//pipelineConfig.rasterizationInfo.depthClampEnable = VK_TRUE;

			ShaderStageConfigInfo shaderStageConfigInfo;
			shaderStageConfigInfo.vertexModule = ShaderModuleConfigInfo("resources\\shaders\\skybox_shader_1.2.vert.spv");
			shaderStageConfigInfo.fragmentModule = ShaderModuleConfigInfo("resources\\shaders\\skybox_shader_1.2.frag.spv");

			pipeline = CreateScope<Pipeline>(device, pipelineConfig, shaderStageConfigInfo);
		}

		void SkyboxRenderLayer::render(FrameInfo& frameInfo) {
			pipeline->bind(frameInfo.commandBuffer);

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				0,
				1,
				&frameInfo.globalDescriptorSet,
				0,
				nullptr);

			if (skyboxDescriptorSet == VK_NULL_HANDLE) {
				auto imageInfo = (VkDescriptorImageInfo*)std::static_pointer_cast<VulkanTexture>(noiseTexture)->GetDescriptorInfo();
				if (!DescriptorWriter(*skyboxRenderSystemLayout, Application::Get().GetMaterialDescriptorPool())
					.WriteImage(0, imageInfo)
					.Build(skyboxDescriptorSet)) {
					MADAM_CORE_ERROR("Failed to allocate skybox descriptor set");
					return;
				}
			}

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				1,  // first set
				1,  // set count
				&skyboxDescriptorSet,
				0,
				nullptr);

			skybox->Bind(frameInfo.commandBuffer);
			skybox->Draw(frameInfo.commandBuffer);
			isFirstFrame = false;
		}

		/*
		------------------Texture Render System------------------
		*/

		TextureRenderLayer::TextureRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderLayer(device, renderPass, globalSetLayout, _name, false) {
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		TextureRenderLayer::~TextureRenderLayer() {
			ClearShaderPipelines();
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
			pipelineLayout = VK_NULL_HANDLE;
		}

		void TextureRenderLayer::ClearShaderPipelines()
		{
			for (auto& [path, state] : shaderPipelines)
			{
				state.pipeline.reset();
				if (state.layout != VK_NULL_HANDLE)
					vkDestroyPipelineLayout(device.device(), state.layout, nullptr);
			}
			shaderPipelines.clear();
			failedShaders.clear();
		}

		void TextureRenderLayer::recreatePipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		{
			vkDeviceWaitIdle(device.device());
			ClearShaderPipelines();
			RenderLayer::recreatePipeline(renderPass, globalSetLayout);
		}

		void TextureRenderLayer::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
			globalLayout = globalSetLayout;
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = sizeof(DefaultPushConstantData);

			renderSystemLayout =
				DescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

			std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
				globalSetLayout,
				renderSystemLayout->getDescriptorSetLayout() };

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
			pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
			if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
				VK_SUCCESS) {
				throw std::runtime_error("failed to create pipeline layout!");
			}
		}

		void TextureRenderLayer::createPipeline(VkRenderPass renderPass) {
			activeRenderPass = renderPass;
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

			PipelineConfigInfo pipelineConfig{};
			Pipeline::setDescriptions(pipelineConfig);
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;

			ShaderStageConfigInfo shaderStageConfigInfo;
			shaderStageConfigInfo.vertexModule = ShaderModuleConfigInfo("resources\\shaders\\texture_shader.vert.spv");
			shaderStageConfigInfo.fragmentModule = ShaderModuleConfigInfo("resources\\shaders\\texture_shader.frag.spv");

			pipeline = CreateScope<Pipeline>(device, pipelineConfig, shaderStageConfigInfo);
		}

		TextureRenderLayer::ShaderPipeline* TextureRenderLayer::GetShaderPipeline(const std::string& fragmentPath)
		{
			if (failedShaders.count(fragmentPath))
				return nullptr;
			auto cached = shaderPipelines.find(fragmentPath);
			if (cached != shaderPipelines.end())
				return &cached->second;

			const auto definition = ReadMaterialShader(fragmentPath,
				std::filesystem::absolute("resources/shaders/texture_shader.vert"));
			if (!definition.IsValid())
			{
				MADAM_CORE_ERROR("Material shader {0}: {1}", fragmentPath, definition.error);
				failedShaders.insert(fragmentPath);
				return nullptr;
			}
			std::vector<uint32_t> vertexCode;
			std::vector<uint32_t> fragmentCode;
			if (Project::HasActive())
			{
				auto& assets = Project::Get().getAssetManager();
				const auto& metadata = assets.GetMetadata(definition.fragmentPath);
				if (metadata.IsValid() && metadata.assetType == AssetType::SHADER)
				{
					std::string diagnostic;
					if (!assets.CompileShader(metadata.uuid, &diagnostic))
					{
						MADAM_CORE_ERROR("Material shader {0}: {1}", fragmentPath, diagnostic);
						failedShaders.insert(fragmentPath);
						return nullptr;
					}
					const auto binaryDirectory = Project::Get().getProjectDirectory() / "Library" / "Shaders";
					const auto name = (std::string)metadata.uuid;
					const auto readBinary = [](const std::filesystem::path& path)
					{
						std::ifstream file(path, std::ios::binary | std::ios::ate);
						const std::streamsize size = file ? static_cast<std::streamsize>(file.tellg()) : 0;
						if (size <= 0 || size % sizeof(uint32_t) != 0)
							return std::vector<uint32_t>{};
						std::vector<uint32_t> code(static_cast<size_t>(size) / sizeof(uint32_t));
						file.seekg(0);
						file.read(reinterpret_cast<char*>(code.data()), static_cast<std::streamsize>(code.size() * sizeof(uint32_t)));
						return file.good() ? code : std::vector<uint32_t>{};
					};
					vertexCode = readBinary(binaryDirectory / (name + ".vert.spv"));
					fragmentCode = readBinary(binaryDirectory / (name + ".frag.spv"));
				}
			}
			if (vertexCode.empty() && fragmentCode.empty())
			{
				vertexCode = VulkanShaderCompiler::CompileStage(definition.vertexPath, VK_SHADER_STAGE_VERTEX_BIT);
				fragmentCode = VulkanShaderCompiler::CompileStage(definition.fragmentPath, VK_SHADER_STAGE_FRAGMENT_BIT);
			}
			if (vertexCode.empty() || fragmentCode.empty())
			{
				failedShaders.insert(fragmentPath);
				return nullptr;
			}

			ShaderPipeline state;
			try
			{
				DescriptorSetLayout::Builder builder(device);
				for (const auto& input : definition.textures)
					builder.addBinding(input.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
				state.descriptorLayout = builder.build();

				VkPushConstantRange push{};
				push.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
				push.size = sizeof(DefaultPushConstantData);
				VkDescriptorSetLayout layouts[] = { globalLayout, state.descriptorLayout->getDescriptorSetLayout() };
				VkPipelineLayoutCreateInfo layoutInfo{};
				layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				layoutInfo.setLayoutCount = 2;
				layoutInfo.pSetLayouts = layouts;
				layoutInfo.pushConstantRangeCount = 1;
				layoutInfo.pPushConstantRanges = &push;
				if (vkCreatePipelineLayout(device.device(), &layoutInfo, nullptr, &state.layout) != VK_SUCCESS)
					throw std::runtime_error("Could not create shader pipeline layout");

				PipelineConfigInfo config;
				Pipeline::setDescriptions(config);
				config.renderPass = activeRenderPass;
				config.pipelineLayout = state.layout;
				ShaderStageConfigInfo stages;
				stages.vertexModule = ShaderModuleConfigInfo(vertexCode);
				stages.fragmentModule = ShaderModuleConfigInfo(fragmentCode);
				state.pipeline = CreateScope<Pipeline>(device, config, stages);
			}
			catch (const std::exception& e)
			{
				MADAM_CORE_ERROR("Could not create material shader pipeline {0}: {1}", fragmentPath, e.what());
				if (state.layout != VK_NULL_HANDLE)
					vkDestroyPipelineLayout(device.device(), state.layout, nullptr);
				failedShaders.insert(fragmentPath);
				return nullptr;
			}
			return &shaderPipelines.emplace(fragmentPath, std::move(state)).first->second;
		}

		void TextureRenderLayer::render(FrameInfo& frameInfo) {
			if (!frameInfo.renderScene.registry)
				return;

			entt::registry& entities = *frameInfo.renderScene.registry;
			for (auto entity : frameInfo.renderScene.meshEntities)
			{
				if (!entities.valid(entity))
					continue;
				auto& meshRenderer = entities.get<CMeshRenderer>(entity);
				if (!meshRenderer.GetMesh())
					continue;
				const Ref<CMaterial> material = meshRenderer.GetMaterial();
				if (!MaterialTexturesReady(material))
					continue;

				std::map<uint32_t, Ref<Texture>> bindings;
				if (material->shaderFragmentPath.empty())
					bindings = { {0, material->diffuse_map}, {1, material->normal_map},
						{2, material->ambient_occlusion_map}, {3, material->gloss_map} };
				else
					bindings = material->textureBindings;

				Pipeline* selectedPipeline = pipeline.get();
				VkPipelineLayout selectedLayout = pipelineLayout;
				DescriptorSetLayout* selectedDescriptorLayout = renderSystemLayout.get();
			static const std::string standardShaderPath =
				std::filesystem::absolute("resources/shaders/texture_shader.frag").lexically_normal().string();
			if (!material->shaderFragmentPath.empty() &&
				material->shaderFragmentPath != standardShaderPath)
				{
					auto custom = GetShaderPipeline(material->shaderFragmentPath);
					if (!custom)
					{
						material->shaderValid = false;
						continue;
					}
					selectedPipeline = custom->pipeline.get();
					selectedLayout = custom->layout;
					selectedDescriptorLayout = custom->descriptorLayout.get();
				}

				selectedPipeline->bind(frameInfo.commandBuffer);
				vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					selectedLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
				if (!bindings.empty())
				{
				if (material->descriptorFrames.size() < SwapChain::MAX_FRAMES_IN_FLIGHT)
					material->descriptorFrames.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
				auto& descriptor = material->descriptorFrames[frameInfo.frameIndex];
				const bool layoutChanged = descriptor.layoutVersion != material->descriptorLayoutVersion;
				if (descriptor.set == VK_NULL_HANDLE || layoutChanged || descriptor.textures != bindings)
					{
						DescriptorWriter writer(*selectedDescriptorLayout, Application::Get().GetMaterialDescriptorPool());
						for (const auto& [binding, texture] : bindings)
							writer.WriteImage(binding, static_cast<VkDescriptorImageInfo*>(texture->GetDescriptorInfo()));
					if (descriptor.set == VK_NULL_HANDLE || layoutChanged)
						{
						VkDescriptorSet newSet = VK_NULL_HANDLE;
						if (!writer.Build(newSet))
							{
								MADAM_CORE_ERROR("Failed to allocate material descriptor set");
								continue;
							}
						descriptor.set = newSet;
						}
						else
						writer.Overwrite(descriptor.set);
					descriptor.layoutVersion = material->descriptorLayoutVersion;
					descriptor.textures = bindings;
					}
					vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					selectedLayout, 1, 1, &descriptor.set, 0, nullptr);
				}

				DefaultPushConstantData push{};
				UUID uuid = entities.get<CUniqueIdentifier>(entity).uuid;
				push.modelMatrix = frameInfo.scene->GetWorldTransform(uuid);
				//push.modelMatrix[3][1] = -push.modelMatrix[3][1];
				push.normalMatrix = glm::transpose(glm::inverse(push.modelMatrix));
				push.normalMatrix[3][0] = meshRenderer.mobility == CMeshRenderer::Mobility::Static ? 1.0f : 0.0f;
				push.normalMatrix[3][1] = meshRenderer.receiveRealtimeShadows ? 1.0f : 0.0f;

				vkCmdPushConstants(
					frameInfo.commandBuffer,
					selectedLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(DefaultPushConstantData),
					&push);

				meshRenderer.GetMesh()->Bind(frameInfo.commandBuffer);
				meshRenderer.GetMesh()->Draw(frameInfo.commandBuffer);
			}

			isFirstFrame = false;
		}

		/*
		------------------Point Light Render System------------------
		*/

		PointLightRenderLayer::PointLightRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderLayer(device, renderPass, globalSetLayout, _name, false) {
			// Ensure to initialize device in the member initialization list
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		PointLightRenderLayer::~PointLightRenderLayer() {
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
			pipelineLayout = VK_NULL_HANDLE;
		}

		void PointLightRenderLayer::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = sizeof(PointLightPushConstants);

			std::vector<VkDescriptorSetLayout> descriptorSetLayout{ globalSetLayout };

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
			pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
			if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create pipeline layout!");
			}
		}

		void PointLightRenderLayer::createPipeline(VkRenderPass renderPass) {
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

			PipelineConfigInfo pipelineConfig{};
			//Pipeline::setDescriptions(pipelineConfig);
			pipelineConfig.enableAlphaBlending();
			pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
			// Keep the offscreen scene opaque so ImGui does not blend the light twice.
			pipelineConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			pipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;

			pipelineConfig.attributeDescriptions.clear();
			pipelineConfig.bindingDescriptions.clear();
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;

			ShaderStageConfigInfo shaderStageConfigInfo;
			shaderStageConfigInfo.vertexModule = ShaderModuleConfigInfo("resources\\shaders\\point_light.vert.spv");
			shaderStageConfigInfo.fragmentModule = ShaderModuleConfigInfo("resources\\shaders\\point_light.frag.spv");

			pipeline = CreateScope<Pipeline>(device, pipelineConfig, shaderStageConfigInfo);
		}

		/*void PointLightRenderLayer::preRender(FrameInfo& frameInfo) {
			auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime/ 5.0f, { 0.f, -1.f, 0.f });
			int lightIndex = 0;

			entt::registry& entities = frameInfo.scene->GetRegistry();
			
			auto group = entities.view<Transform, PointLight>();
			for (auto entity : group)
			{
				auto [transform, pointLight] = group.get<Transform, PointLight>(entity);

				assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

				//update light position
				transform.translation = glm::vec3(rotateLight * glm::vec4(transform.translation, 1.f));

				//copy light to ubo
				frameInfo.ubo.pointLights[lightIndex].position = glm::vec4(transform.translation, 1.f);
				frameInfo.ubo.pointLights[lightIndex].color = glm::vec4(pointLight.color, pointLight.intensity);

				lightIndex += 1;
			}
			frameInfo.ubo.numLights = lightIndex;
		}*/

		void PointLightRenderLayer::render(FrameInfo& frameInfo) {

			if (!frameInfo.renderScene.registry)
			{
				return;
			}

			std::map<float, entt::entity> sorted;
			entt::registry& entities = *frameInfo.renderScene.registry;

			//This could absolutely be speed up if put on another thread and done before the rendering call
			for (entt::entity entity : frameInfo.renderScene.pointLightEntities) {
				auto& pointLight = entities.get<CPointLight>(entity);
				if (!pointLight.enabled) continue;

				UUID uuid = entities.get<CUniqueIdentifier>(entity).uuid;
				glm::mat4 worldTransform = frameInfo.scene->GetWorldTransform(uuid);
				glm::vec3 worldTranslation = glm::vec3(worldTransform[3][0], worldTransform[3][1], worldTransform[3][2]);

				auto offset = glm::vec3(frameInfo.ubo.inverseView[3]) - worldTranslation;
				float disSquared = glm::dot(offset, offset);
				sorted[disSquared] = entity;
			}

			pipeline->bind(frameInfo.commandBuffer);

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				0,
				1,
				&frameInfo.globalDescriptorSet,
				0,
				nullptr);

			for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
			{
				entt::entity entity = it->second;
				auto& pointLight = entities.get<CPointLight>(entity);
				PointLightPushConstants push{};
				//glm::vec3 adjustedTranslation = transform.translation;
				//adjustedTranslation.y = -transform.translation.y;
				UUID uuid = entities.get<CUniqueIdentifier>(entity).uuid;
				glm::mat4 worldTransform = frameInfo.scene->GetWorldTransform(uuid);
				glm::vec3 worldTranslation = glm::vec3(worldTransform[3][0], worldTransform[3][1], worldTransform[3][2]);
				push.position = glm::vec4(worldTranslation, 1.f);
				push.color = glm::vec4(pointLight.color, pointLight.intensity);
				push.radius = pointLight.radius;

				vkCmdPushConstants(
					frameInfo.commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(PointLightPushConstants),
					&push
				);
				vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
			}
			isFirstFrame = false;
		}
		/*
		------------------Master Render System------------------
		*/

		RenderStack::~RenderStack() { 
			//renderSystems.clear(); 
			if (isRunning) {
				MADAM_CORE_WARN("RenderStack prematurally shutdown");
				deinit();
			}
		}

		void RenderStack::init() {
			isRunning = true;
		}

		void RenderStack::deinit() {
			renderSystems.clear();
			isRunning = false;
		}

		void RenderStack::initialize(Scope<DescriptorSetLayout>& globalSetLayout) {
			MADAM_CORE_INFO("Pushing Render Systems in vector");
			try {
				renderSystems.push_back(std::make_unique<SkyboxRenderLayer>
					(
						device,
						renderer.GetMainRenderPass(),
						globalSetLayout->getDescriptorSetLayout(),
						"Skybox Render System"
					));
				renderSystems.push_back(std::make_unique<TextureRenderLayer>
					(
						device,
						renderer.GetMainRenderPass(),
						globalSetLayout->getDescriptorSetLayout(),
						"Texture Render System"
					));
				MADAM_CORE_INFO("Texture Render Layer Complete");
				renderSystems.push_back(std::make_unique<RenderLayer>
					(
						device,
						renderer.GetMainRenderPass(),
						globalSetLayout->getDescriptorSetLayout(),
						"Render System"
					));
				MADAM_CORE_INFO("Default Render Layer Complete");
				/*renderSystems.push_back(std::make_unique<CustomLayer>
					(
						device,
						renderer.GetMainRenderPass(),
						globalSetLayout->getDescriptorSetLayout(),
						"Custom Render System"
					));
				MADAM_CORE_INFO("Custom Render Layer Complete");*/
				renderSystems.push_back(std::make_unique<GridRenderLayer>
					(
						device,
						renderer.GetMainRenderPass(),
						globalSetLayout->getDescriptorSetLayout(),
						"Grid Render System"
					));
				MADAM_CORE_INFO("Grid Render Layer Complete");
				renderSystems.push_back(std::make_unique<PointLightRenderLayer>
					(
						device,
						renderer.GetMainRenderPass(),
						globalSetLayout->getDescriptorSetLayout(),
						"Point Light Render System"
					));
				MADAM_CORE_INFO("Point Light Render Layer Complete");
			}
			catch (const std::exception& e) {
				MADAM_CORE_ERROR("Error: {0}", e.what());
			}
			MADAM_CORE_INFO("Pushing Render Systems in vector is completed");
		}

		void RenderStack::render(FrameInfo& frameInfo, bool includeEditorGrid) {
			for (auto& system : renderSystems) {
				if (!includeEditorGrid && dynamic_cast<GridRenderLayer*>(system.get())) continue;
				system->updatePipelineState();
				system->render(frameInfo);
			}
		}

		bool RenderStack::switchRenderSystems(int first, int second) {
			std::swap(renderSystems[first], renderSystems[second]);
			return true;
		}

		void RenderStack::reloadPipeline(int renderLayer, VkRenderPass renderPass, Scope<DescriptorSetLayout>& globalDescriptorSetLayout)
		{
			if (renderLayer >= 0 && 0 < renderSystems.size())
			{
				renderSystems[renderLayer]->recreatePipeline(renderPass, globalDescriptorSetLayout->getDescriptorSetLayout());
			}
		}
	}
}
