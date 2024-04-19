#include "maidenpch.hpp"
#include "H_RenderSystems.hpp"

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
			pipeline = std::make_unique<Pipeline>(device, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);
        }

		void RenderLayer::preRender(FrameInfo& frameInfo) {
			//std::cout << "PreRendering \n";
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
			auto group = entities.view<Transform, MeshRenderer>();
			for (auto entity : group)
			{
				auto [transform, meshRenderer] = group.get<Transform, MeshRenderer>(entity);

				if (!entities.valid(entity)) {
					std::cerr << "Error, entity is not valid" << std::endl;
					continue;
				}
				if (meshRenderer.getModel() == nullptr) continue;

				std::shared_ptr<Material> material = meshRenderer.getMaterial();
				if (material != nullptr) continue;

				if (isFirstFrame) {
					std::cout << "Standard Rendering " << std::endl;
				}

				DefaultPushConstantData push{};
				push.modelMatrix = transform.m_transform();
				push.normalMatrix = transform.normalMatrix();

				vkCmdPushConstants(
					frameInfo.commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(DefaultPushConstantData),
					&push);
				meshRenderer.getModel()->bind(frameInfo.commandBuffer);
				meshRenderer.getModel()->draw(frameInfo.commandBuffer);
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
			/*VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = 0;*/

			//Change for different layout
			std::vector<VkDescriptorSetLayout> descriptorSetLayout{ globalSetLayout };

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

			if (isFirstFrame) {
				std::cout << "Rendering Grid" << std::endl;
			}

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				0,
				1,
				&frameInfo.globalDescriptorSet,
				0,
				nullptr);

			/*vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				0,
				nullptr);*/
			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
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
			pushConstantRange.size = sizeof(TexturePushConstantData);

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
				"shaders/texture_shader.vert.spv",
				"shaders/texture_shader.frag.spv",
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
			auto group = entities.view<Transform, MeshRenderer>();

			for (auto entity : group) {

				auto [transform, meshRenderer] = group.get<Transform, MeshRenderer>(entity);

				if (!entities.valid(entity)) {
					std::cerr << "Error, entity is not valid" << std::endl;
					continue;
				}
				// skip objects that don't have both a model and texture
				//JcvbMeshRenderer* meshRenderer = obj.getComponent<JcvbMeshRenderer>();
				//if (meshRenderer == nullptr) continue;
				std::shared_ptr<Material> material = meshRenderer.getMaterial();
				if (material == nullptr) continue;
				if (material->diffuseMap == nullptr) continue;

				if (isFirstFrame) {
					std::cout << "Texture Rendering: " << std::endl;
				}

				// writing descriptor set each frame can slow performance
				// would be more efficient to implement some sort of caching
				// Edit implement descriptor set pool
				auto imageInfo = meshRenderer.getMaterial()->diffuseMap->getImageInfo();
				auto normalInfo = meshRenderer.getMaterial()->normalMap->getImageInfo();
				auto ambientOcclusionInfo = meshRenderer.getMaterial()->ambientOcclusionMap->getImageInfo();
				auto glossInfo = meshRenderer.getMaterial()->glossMap->getImageInfo();
				VkDescriptorSet descriptorSet1;
				DescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
					.writeImage(0, &imageInfo)
					.writeImage(1, &normalInfo)
					.writeImage(2, &ambientOcclusionInfo)
					.writeImage(3, &glossInfo)
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

				TexturePushConstantData push{};
				push.modelMatrix = transform.m_transform();
				push.normalMatrix = transform.normalMatrix();

				vkCmdPushConstants(
					frameInfo.commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(TexturePushConstantData),
					&push);

				meshRenderer.getModel()->bind(frameInfo.commandBuffer);
				meshRenderer.getModel()->draw(frameInfo.commandBuffer);
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

		void PointLightRenderLayer::preRender(FrameInfo& frameInfo) {
			auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime, { 0.f, -1.f, 0.f });
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
		}

		void PointLightRenderLayer::render(FrameInfo& frameInfo) {

			std::map<float, entt::entity> sorted;
			entt::registry& entities = frameInfo.scene->Reg();

			//This could absolutely be speed up if put on another thread and done before the rendering call
			auto group = entities.view<Transform, PointLight>();
			for (auto entity : group) {
				auto [transform, pointLight] = group.get<Transform, PointLight>(entity);
				auto offset = Rendering::CameraHandle::getMain().getPosition() - transform.translation;
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
				auto [transform, pointLight] = entities.get<Transform, PointLight>(entity);
				PointLightPushConstants push{};
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
		------------------GUI Render System------------------
		*/

		GUILayer::GUILayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout guiSetLayout, std::string _name)
			: device{ device }, name{ std::move(_name) } {

			createPipelineLayout(guiSetLayout);
			createPipeline(renderPass);
		}

		GUILayer::~GUILayer() {
			vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
		}

		void GUILayer::createPipelineLayout(VkDescriptorSetLayout guiSetLayout) {
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = sizeof(DefaultPushConstantData);

			std::vector<VkDescriptorSetLayout> descriptorSetLayout{ guiSetLayout };

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

		void GUILayer::createPipeline(VkRenderPass renderPass) {
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

			PipelineConfigInfo pipelineConfig{};
			Pipeline::setDescriptions(pipelineConfig);
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;
			pipeline = std::make_unique<Pipeline>(device, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);
		}

		void GUILayer::preRender(FrameInfo& frameInfo) {
			//std::cout << "PreRendering \n";
		}

		void GUILayer::render(FrameInfo& frameInfo) {
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
			auto group = entities.view<Transform, MeshRenderer>();
			for (auto entity : group)
			{
				auto [transform, meshRenderer] = group.get<Transform, MeshRenderer>(entity);

				if (!entities.valid(entity)) {
					std::cerr << "Error, entity is not valid" << std::endl;
					continue;
				}
				if (meshRenderer.getModel() == nullptr) continue;

				std::shared_ptr<Material> material = meshRenderer.getMaterial();
				if (material != nullptr) continue;

				if (isFirstFrame) {
					std::cout << "Standard Rendering " << std::endl;
				}

				DefaultPushConstantData push{};
				push.modelMatrix = transform.m_transform();
				push.normalMatrix = transform.normalMatrix();

				vkCmdPushConstants(
					frameInfo.commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(DefaultPushConstantData),
					&push);
				meshRenderer.getModel()->bind(frameInfo.commandBuffer);
				meshRenderer.getModel()->draw(frameInfo.commandBuffer);
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
				ShutDown();
			}
		}

		void RenderStack::StartUp() {
			isRunning = true;
		}

		void RenderStack::ShutDown() {
			renderSystems.clear();
			isRunning = false;
		}

		void RenderStack::initialize(std::unique_ptr<DescriptorSetLayout>& globalSetLayout) {

			renderSystems.push_back(std::make_unique<TextureRenderLayer>
				(
					device,
					renderer.getSwapChainRenderPass(),
					globalSetLayout->getDescriptorSetLayout(),
					"Texture Render System"
				));

			renderSystems.push_back(std::make_unique<RenderLayer>
				(
					device,
					renderer.getSwapChainRenderPass(),
					globalSetLayout->getDescriptorSetLayout(),
					"Render System"
				));

			renderSystems.push_back(std::make_unique<GridRenderLayer>
				(
					device,
					renderer.getSwapChainRenderPass(),
					globalSetLayout->getDescriptorSetLayout(),
					"Grid Render System"
				));

			renderSystems.push_back(std::make_unique<PointLightRenderLayer>
				(
					device,
					renderer.getSwapChainRenderPass(),
					globalSetLayout->getDescriptorSetLayout(),
					"Point Light Render System"
				));
			
		}

		void RenderStack::preRender(FrameInfo& frameInfo) {
			for (auto& system : renderSystems) {
				system->preRender(frameInfo);
			}
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