#include "maidenpch.hpp"
#include "H_RenderSystems.hpp"
#include "../Core/H_Application.hpp"

//May need overhaul since a pipeline may need to be created for each shader
//Each shader needs a pipeline. Shader variants can use the same pipeline but ubershader has to be used which can cause a performance hit to the GPU.
//More research required
//Maybe have vkCmdSequences built onto class with builder and then have it execute in order for rendersystems?
//Or could have CmdSequences handled in pipeline via a method by default but can override with custom sequence?

//UI rendersystem is billboard quad with texture. see (pg 49)

//Refactor to remove Device
namespace Madam {
	namespace Rendering {
		/*
		------------------Render System------------------
		*/

		RenderLayer::RenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: device{ device }, name{ std::move(_name) } {

			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		RenderLayer::~RenderLayer() {
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
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
			pipeline = std::make_unique<Pipeline>(device, "shaders\\simple_shader.vert.spv", "shaders\\simple_shader.frag.spv", pipelineConfig);
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
			entt::registry& entities = frameInfo.scene->Reg();
			auto group = entities.view<CTransform, CMeshRenderer>();
			for (auto entity : group)
			{
				auto [transform, meshRenderer] = group.get<CTransform, CMeshRenderer>(entity);

				if (!entities.valid(entity)) {
					std::cerr << "Error, entity is not valid" << std::endl;
					continue;
				}
				if (meshRenderer.GetMesh() == nullptr) continue;

				Ref<CMaterial> material = meshRenderer.GetMaterial();
				if (material != nullptr) continue;

				DefaultPushConstantData push{};
				push.modelMatrix = frameInfo.scene->GetWorldTransform(frameInfo.scene->Reg().get<CUniqueIdentifier>(entity).uuid);
				//push.modelMatrix[3][1] = -push.modelMatrix[3][1];
				push.normalMatrix = glm::transpose(glm::inverse(push.modelMatrix));

				vkCmdPushConstants(
					frameInfo.commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(DefaultPushConstantData),
					&push);
				meshRenderer.GetMesh()->bind(frameInfo.commandBuffer);
				meshRenderer.GetMesh()->draw(frameInfo.commandBuffer);
			}
			isFirstFrame = false;
		}

		/*
		------------------Grid Render System------------------
		*/

		GridRenderLayer::GridRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderLayer(device, renderPass, globalSetLayout, _name) {
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		GridRenderLayer::~GridRenderLayer() {
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
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
			//Pipeline::enableAlphaBlending(pipelineConfig);

			//pipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
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
			pipeline = std::make_unique<Pipeline>(device, "shaders/grid_shader.vert.spv", "shaders/grid_shader.frag.spv", pipelineConfig);
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
			: RenderLayer(device, renderPass, globalSetLayout, _name) {
			TextureData textureData;
			noiseTexture = Texture::Create(textureData, std::filesystem::u8path("resources\\textures\\skybox.png"));
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		SkyboxRenderLayer::~SkyboxRenderLayer() {
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
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
			pipeline = std::make_unique<Pipeline>(device, "shaders/skybox_shader_1.2.vert.spv", "shaders/skybox_shader_1.2.frag.spv", pipelineConfig);
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

			VkDescriptorSet descriptorSet1;
			DescriptorWriter(*skyboxRenderSystemLayout, frameInfo.frameDescriptorPool)
				//.writeBuffer(0, &skyboxBuffer->descriptorInfo())
				.writeImage(0, (VkDescriptorImageInfo*)std::static_pointer_cast<VulkanTexture>(noiseTexture)->GetDescriptorInfo())
				.build(descriptorSet1);

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				1,  // first set
				1,  // set count
				&descriptorSet1,
				0,
				nullptr);

			skybox->bind(frameInfo.commandBuffer);
			skybox->draw(frameInfo.commandBuffer);
			isFirstFrame = false;
		}

		/*
		------------------Texture Render System------------------
		*/

		TextureRenderLayer::TextureRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderLayer(device, renderPass, globalSetLayout, _name) {
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		TextureRenderLayer::~TextureRenderLayer() {
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
		}

		void TextureRenderLayer::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
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
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

			PipelineConfigInfo pipelineConfig{};
			Pipeline::setDescriptions(pipelineConfig);
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;
			pipeline = std::make_unique<Pipeline>(
				device,
				"shaders\\texture_shader.vert.spv",
				"shaders\\texture_shader.frag.spv",
				pipelineConfig);
		}

		void TextureRenderLayer::render(FrameInfo& frameInfo) {
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

			entt::registry& entities = frameInfo.scene->Reg();
			auto group = entities.view<CTransform, CMeshRenderer>();

			for (auto entity : group) {

				auto [transform, meshRenderer] = group.get<CTransform, CMeshRenderer>(entity);

				if (!entities.valid(entity)) {
					std::cerr << "Error, entity is not valid" << std::endl;
					continue;
				}
				// skip objects that don't have both a model and texture
				//JcvbMeshRenderer* meshRenderer = obj.getComponent<JcvbMeshRenderer>();
				//if (meshRenderer == nullptr) continue;
				Ref<CMaterial> material = meshRenderer.GetMaterial();
				if (material == nullptr) continue;
				if (material->diffuseMap == nullptr) continue;

				// writing descriptor set each frame can slow performance
				// would be more efficient to implement some sort of caching
				// Edit implement descriptor set pool
				auto imageInfo = (VkDescriptorImageInfo*)meshRenderer.GetMaterial()->diffuseMap->GetDescriptorInfo();
				auto normalInfo = (VkDescriptorImageInfo*)meshRenderer.GetMaterial()->normalMap->GetDescriptorInfo();
				auto ambientOcclusionInfo = (VkDescriptorImageInfo*)meshRenderer.GetMaterial()->ambientOcclusionMap->GetDescriptorInfo();
				auto glossInfo = (VkDescriptorImageInfo*)meshRenderer.GetMaterial()->glossMap->GetDescriptorInfo();
				VkDescriptorSet descriptorSet1;
				DescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
					.writeImage(0, imageInfo)
					.writeImage(1, normalInfo)
					.writeImage(2, ambientOcclusionInfo)
					.writeImage(3, glossInfo)
					.build(descriptorSet1);

				vkCmdBindDescriptorSets(
					frameInfo.commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineLayout,
					1,  // first set
					1,  // set count
					&descriptorSet1,
					0,
					nullptr);

				DefaultPushConstantData push{};
				push.modelMatrix = frameInfo.scene->GetWorldTransform(frameInfo.scene->Reg().get<CUniqueIdentifier>(entity).uuid);
				//push.modelMatrix[3][1] = -push.modelMatrix[3][1];
				push.normalMatrix = glm::transpose(glm::inverse(push.modelMatrix));

				vkCmdPushConstants(
					frameInfo.commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(DefaultPushConstantData),
					&push);

				meshRenderer.GetMesh()->bind(frameInfo.commandBuffer);
				meshRenderer.GetMesh()->draw(frameInfo.commandBuffer);
			}

			isFirstFrame = false;
		}

		/*
		------------------Point Light Render System------------------
		*/

		PointLightRenderLayer::PointLightRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderLayer(device, renderPass, globalSetLayout, _name) {
			// Ensure to initialize device in the member initialization list
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		PointLightRenderLayer::~PointLightRenderLayer() {
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
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

			// default color blending for this isn't physically accurate, but i like how it looks
			// in this case better, and are current rendering for this isn't physically accurate anyway
			// pipelineConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;

			pipelineConfig.attributeDescriptions.clear();
			pipelineConfig.bindingDescriptions.clear();
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;
			pipeline = std::make_unique<Pipeline>(device, "shaders/point_light.vert.spv", "shaders/point_light.frag.spv", pipelineConfig);
		}

		/*void PointLightRenderLayer::preRender(FrameInfo& frameInfo) {
			auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime/ 5.0f, { 0.f, -1.f, 0.f });
			int lightIndex = 0;

			entt::registry& entities = frameInfo.scene->Reg();
			
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

			std::map<float, entt::entity> sorted;
			entt::registry& entities = frameInfo.scene->Reg();

			//This could absolutely be speed up if put on another thread and done before the rendering call
			auto group = entities.view<CTransform, CPointLight>();
			for (auto entity : group) {
				auto [transform, pointLight] = group.get<CTransform, CPointLight>(entity);
				auto offset = Rendering::CameraHandle::GetMain().GetPosition() - transform.translation;
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
				auto [transform, pointLight] = entities.get<CTransform, CPointLight>(entity);
				PointLightPushConstants push{};
				//glm::vec3 adjustedTranslation = transform.translation;
				//adjustedTranslation.y = -transform.translation.y;
				push.position = glm::vec4(transform.translation, 1.f);
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
						renderer.getMainRenderPass(),
						globalSetLayout->getDescriptorSetLayout(),
						"Skybox Render System"
					));
				renderSystems.push_back(std::make_unique<TextureRenderLayer>
					(
						device,
						renderer.getMainRenderPass(),
						globalSetLayout->getDescriptorSetLayout(),
						"Texture Render System"
					));
				MADAM_CORE_INFO("Texture Render Layer Complete");
				renderSystems.push_back(std::make_unique<RenderLayer>
					(
						device,
						renderer.getMainRenderPass(),
						globalSetLayout->getDescriptorSetLayout(),
						"Render System"
					));
				MADAM_CORE_INFO("Default Render Layer Complete");
				renderSystems.push_back(std::make_unique<GridRenderLayer>
					(
						device,
						renderer.getMainRenderPass(),
						globalSetLayout->getDescriptorSetLayout(),
						"Grid Render System"
					));
				MADAM_CORE_INFO("Grid Render Layer Complete");
				renderSystems.push_back(std::make_unique<PointLightRenderLayer>
					(
						device,
						renderer.getMainRenderPass(),
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

		void RenderStack::render(FrameInfo& frameInfo) {
			for (auto& system : renderSystems) {
				system->render(frameInfo);
			}
		}

		bool RenderStack::switchRenderSystems(int first, int second) {
			std::swap(renderSystems[first], renderSystems[second]);
			return true;
		}
	}
}