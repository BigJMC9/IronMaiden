#pragma once

#include "H_JCVB_camera.hpp"
#include "H_JCVB_descriptors.hpp"
#include "H_JCVB_device.hpp"
#include "H_JCVB_frame_info.hpp"
#include "H_JCVB_game_object.hpp"
#include "H_JCVB_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace Digestion {
    class TextureRenderSystem {
    public:
        TextureRenderSystem(
            Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~TextureRenderSystem();

        TextureRenderSystem(const TextureRenderSystem&) = delete;
        TextureRenderSystem& operator=(const TextureRenderSystem&) = delete;

        void renderGameObjects(FrameInfo& frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        bool isFirstFrame = true;

        Device& lveDevice;

        std::unique_ptr<Pipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;

        std::unique_ptr<JcvbDescriptorSetLayout> renderSystemLayout;
    };
}  // namespace lve