#include "H_Render_systems.hpp"

//std
#include <map>

//May need overhaul since a pipeline may need to be created for each shader
//Each shader needs a pipeline. Shader variants can use the same pipeline but ubershader has to be used which can cause a performance hit to the GPU.
//More research required
//Maybe have vkCmdSequences built onto class with builder and then have it execute in order for rendersystems?
//Or could have CmdSequences handled in pipeline via a method by default but can override with custom sequence?
namespace Digestion {
	namespace Rendering {
		/*
		------------------Render System------------------
		*/

		RenderSystem::RenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: jcvbDevice{ device }, name{ std::move(_name) } {

			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		RenderSystem::~RenderSystem() {
			vkDestroyPipelineLayout(jcvbDevice.device(), pipelineLayout, nullptr);
		}

        void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
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
			if (vkCreatePipelineLayout(jcvbDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create pipeline layout!");
			}
        }

        void RenderSystem::createPipeline(VkRenderPass renderPass) {
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

			PipelineConfigInfo pipelineConfig{};
			Pipeline::defaultPipelineConfigInfo(pipelineConfig);
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;
			jcvbPipeline = std::make_unique<Pipeline>(jcvbDevice, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);
        }

		void RenderSystem::preRender(FrameInfo& frameInfo) {
			//std::cout << "PreRendering \n";
		}

		void RenderSystem::render(FrameInfo& frameInfo) {
			jcvbPipeline->bind(frameInfo.commandBuffer);

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

		GridRenderSystem::GridRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderSystem(device, renderPass, globalSetLayout, _name) {
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		GridRenderSystem::~GridRenderSystem() {
			vkDestroyPipelineLayout(jcvbDevice.device(), pipelineLayout, nullptr);
		}

		void GridRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
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
			if (vkCreatePipelineLayout(jcvbDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create pipeline layout!");
			}
		}

		void GridRenderSystem::createPipeline(VkRenderPass renderPass) {
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");


			PipelineConfigInfo pipelineConfig{};
			Pipeline::defaultPipelineConfigInfo(pipelineConfig);
			Pipeline::enableAlphaBlending(pipelineConfig);

			//pipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			pipelineConfig.attributeDescriptions.clear();
			pipelineConfig.bindingDescriptions.clear();
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;
			pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
			pipelineConfig.depthStencilInfo.depthWriteEnable = VK_TRUE;
			pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			pipelineConfig.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
			pipelineConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;
			jcvbPipeline = std::make_unique<Pipeline>(jcvbDevice, "shaders/grid_shader.vert.spv", "shaders/grid_shader.frag.spv", pipelineConfig);
		}

		void GridRenderSystem::render(FrameInfo& frameInfo) {
			jcvbPipeline->bind(frameInfo.commandBuffer);

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

		TextureRenderSystem::TextureRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderSystem(device, renderPass, globalSetLayout, _name) {
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		TextureRenderSystem::~TextureRenderSystem() {
			vkDestroyPipelineLayout(jcvbDevice.device(), pipelineLayout, nullptr);
		}

		void TextureRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = sizeof(TexturePushConstantData);

			renderSystemLayout =
				JcvbDescriptorSetLayout::Builder(jcvbDevice)
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
			if (vkCreatePipelineLayout(jcvbDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
				VK_SUCCESS) {
				throw std::runtime_error("failed to create pipeline layout!");
			}
		}

		void TextureRenderSystem::createPipeline(VkRenderPass renderPass) {
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

			PipelineConfigInfo pipelineConfig{};
			Pipeline::defaultPipelineConfigInfo(pipelineConfig);
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;
			jcvbPipeline = std::make_unique<Pipeline>(
				jcvbDevice,
				"shaders/texture_shader.vert.spv",
				"shaders/texture_shader.frag.spv",
				pipelineConfig);
		}

		void TextureRenderSystem::render(FrameInfo& frameInfo) {
			jcvbPipeline->bind(frameInfo.commandBuffer);

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
			//std::cout << "---Texture---" << std::endl;
			//auto group = entities.group<Transform>(entt::get<MeshRenderer>);
			for (auto entity : group) {
				//auto& obj = kv.second;
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
				JcvbDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
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

		PointLightRenderSystem::PointLightRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name)
			: RenderSystem(device, renderPass, globalSetLayout, _name) {
			// Ensure to initialize jcvbDevice in the member initialization list
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		PointLightRenderSystem::~PointLightRenderSystem() {
			vkDestroyPipelineLayout(jcvbDevice.device(), pipelineLayout, nullptr);
		}

		void PointLightRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
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
			if (vkCreatePipelineLayout(jcvbDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create pipeline layout!");
			}
		}

		void PointLightRenderSystem::createPipeline(VkRenderPass renderPass) {
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

			PipelineConfigInfo pipelineConfig{};
			Pipeline::defaultPipelineConfigInfo(pipelineConfig);
			Pipeline::enableAlphaBlending(pipelineConfig);

			// default color blending for this isn't physically accurate, but i like how it looks
			// in this case better, and are current rendering for this isn't physically accurate anyway
			// pipelineConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;

			pipelineConfig.attributeDescriptions.clear();
			pipelineConfig.bindingDescriptions.clear();
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;
			jcvbPipeline = std::make_unique<Pipeline>(jcvbDevice, "shaders/point_light.vert.spv", "shaders/point_light.frag.spv", pipelineConfig);
		}

		void PointLightRenderSystem::preRender(FrameInfo& frameInfo) {
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

		void PointLightRenderSystem::render(FrameInfo& frameInfo) {

			std::map<float, entt::entity> sorted;
			entt::registry& entities = frameInfo.scene->Reg();

			//This could absolutely be speed up if put on another thread and done before the rendering call
			auto group = entities.view<Transform, PointLight>();
			for (auto entity : group) {
				auto [transform, pointLight] = group.get<Transform, PointLight>(entity);
				auto offset = frameInfo.camera.getPosition() - transform.translation;
				float disSquared = glm::dot(offset, offset);
				sorted[disSquared] = entity;
			}

			jcvbPipeline->bind(frameInfo.commandBuffer);

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
		------------------Master Render System------------------
		*/

		MasterRenderSystem::~MasterRenderSystem() { 
			renderSystems.clear(); 
		}

		void MasterRenderSystem::initialize(std::unique_ptr<JcvbDescriptorSetLayout>& globalSetLayout) {

			renderSystems.push_back(std::make_unique<TextureRenderSystem>
				(
					jcvbDevice,
					jcvbRenderer.getSwapChainRenderPass(),
					globalSetLayout->getDescriptorSetLayout(),
					"Texture Render System"
				));

			renderSystems.push_back(std::make_unique<RenderSystem>
				(
					jcvbDevice,
					jcvbRenderer.getSwapChainRenderPass(),
					globalSetLayout->getDescriptorSetLayout(),
					"Render System"
				));

			renderSystems.push_back(std::make_unique<GridRenderSystem>
				(
					jcvbDevice,
					jcvbRenderer.getSwapChainRenderPass(),
					globalSetLayout->getDescriptorSetLayout(),
					"Grid Render System"
				));

			renderSystems.push_back(std::make_unique<PointLightRenderSystem>
				(
					jcvbDevice,
					jcvbRenderer.getSwapChainRenderPass(),
					globalSetLayout->getDescriptorSetLayout(),
					"Point Light Render System"
				));
			
		}

		void MasterRenderSystem::preRender(FrameInfo& frameInfo) {
			for (auto& system : renderSystems) {
				system->preRender(frameInfo);
			}
		}

		void MasterRenderSystem::render(FrameInfo& frameInfo) {
			for (auto& system : renderSystems) {
				system->render(frameInfo);
			}
		}

		bool MasterRenderSystem::switchRenderSystems(int first, int second) {
			std::swap(renderSystems[first], renderSystems[second]);
			return true;
		}
	}
}