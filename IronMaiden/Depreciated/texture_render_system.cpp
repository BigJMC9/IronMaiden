#include "H_texture_render_system.hpp"
#include "H_JCVB_mesh_renderer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace Digestion {

    struct TexturePushConstantData {
        glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
    };

    TextureRenderSystem::TextureRenderSystem(
        Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : lveDevice{ device } {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    TextureRenderSystem::~TextureRenderSystem() {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void TextureRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(TexturePushConstantData);

        renderSystemLayout =
            JcvbDescriptorSetLayout::Builder(lveDevice)
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
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
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
        lvePipeline = std::make_unique<Pipeline>(
            lveDevice,
            "shaders/texture_shader.vert.spv",
            "shaders/texture_shader.frag.spv",
            pipelineConfig);
    }

    //Gameobject components should be doing this part
    void TextureRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
        lvePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        
        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;

            // skip objects that don't have both a model and texture
            JcvbMeshRenderer* meshRenderer = obj.getComponent<JcvbMeshRenderer>();
            if (meshRenderer == nullptr) continue;
            std::shared_ptr<Material> material = meshRenderer->getMaterial();
            if (material == nullptr || material->diffuseMap == nullptr) continue;

            if (isFirstFrame) {
                std::cout << "Texture Rendering: " << obj.getName() << std::endl;
            }
            
            // writing descriptor set each frame can slow performance
            // would be more efficient to implement some sort of caching
            // Edit implement descriptor set pool
            auto imageInfo = meshRenderer->getMaterial()->diffuseMap->getImageInfo();
            auto normalInfo = meshRenderer->getMaterial()->normalMap->getImageInfo();
            auto ambientOcclusionInfo = meshRenderer->getMaterial()->ambientOcclusionMap->getImageInfo();
            auto glossInfo = meshRenderer->getMaterial()->glossMap->getImageInfo();
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

            //meshRenderer->render();

            TexturePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(TexturePushConstantData),
                &push);

            meshRenderer->getModel()->bind(frameInfo.commandBuffer);
            meshRenderer->getModel()->draw(frameInfo.commandBuffer);
        }

        isFirstFrame = false;
    }
}