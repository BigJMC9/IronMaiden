#pragma once

//#include "H_JCVB_camera.hpp"
#include "H_JCVB_descriptors.hpp"
#include "H_JCVB_device.hpp"
#include "H_JCVB_frame_info.hpp"
#include "H_components.hpp"
#include "H_Game_Object.hpp"
#include "H_JCVB_pipeline.hpp"
#include "H_JCVB_renderer.hpp"
#include "H_JCVB_base.hpp"

// std
#include <memory>
#include <functional>
#include <vector>
#include <iostream>

namespace Digestion {
    namespace Rendering {

        struct DefaultPushConstantData {
            glm::mat4 modelMatrix{ 1.f };
            glm::mat4 normalMatrix{ 1.f };
        };

        struct TexturePushConstantData {
            glm::mat4 modelMatrix{ 1.f };
            glm::mat4 normalMatrix{ 1.f };
        };

        struct PointLightPushConstants {
            glm::vec4 position{};
            glm::vec4 color{};
            float radius;
        };

		class RenderSystem {

		public:
            RenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "Default");
            //RenderSystem(JcvbDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name);
			~RenderSystem();

			//RenderSystem(const RenderSystem&) = delete;
			//RenderSystem& operator=(const RenderSystem&) = delete;

			std::string name = "Default";

            virtual void preRender(FrameInfo& frameInfo);
            virtual void render(FrameInfo& frameInfo);

		protected:
            virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            virtual void createPipeline(VkRenderPass renderPass);

			bool isFirstFrame = true;

            Device& jcvbDevice;

            std::unique_ptr<Pipeline> jcvbPipeline;
            VkPipelineLayout pipelineLayout;
		};

        class TextureRenderSystem : public RenderSystem {

        public:
            TextureRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "Default");
            //TextureRenderSystem(JcvbDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name);
            ~TextureRenderSystem();

            //TextureRenderSystem(const TextureRenderSystem&) = delete;
            //TextureRenderSystem& operator=(const TextureRenderSystem&) = delete;

            //void preRender(FrameInfo& frameInfo) override;
            void render(FrameInfo& frameInfo) override;

        protected:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
            void createPipeline(VkRenderPass renderPass) override;

            std::unique_ptr<JcvbDescriptorSetLayout> renderSystemLayout;
        };

        class PointLightRenderSystem : public RenderSystem {

        public:
            PointLightRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "Default");
            //PointLightRenderSystem(JcvbDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name);
            ~PointLightRenderSystem();

            //PointLightRenderSystem(const PointLightRenderSystem&) = delete;
            //PointLightRenderSystem& operator=(const PointLightRenderSystem&) = delete;

            void preRender(FrameInfo& frameInfo) override;
            void render(FrameInfo& frameInfo) override;
            

        protected:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
            void createPipeline(VkRenderPass renderPass) override;
        };

        class MasterRenderSystem {

        public:
            MasterRenderSystem(Device& device, Renderer& renderer) : jcvbDevice{ device }, jcvbRenderer{ renderer } {}
            ~MasterRenderSystem();

            

            void initialize(std::unique_ptr<JcvbDescriptorSetLayout>& globalSetLayout);
            void preRender(FrameInfo& frameInfo);
            void render(FrameInfo& frameInfo);

            /*void addSystem(const RenderSystem& renderSystem) {
                renderSystems.push_back(std::make_unique<RenderSystem>(std::move(renderSystem)));
            }*/

            Renderer& jcvbRenderer;

        private:
            Device& jcvbDevice;
            //Should be Shared
            std::vector<std::unique_ptr<RenderSystem>> renderSystems;
            //std::vector<std::function<void()>> orderOfExecution;
        };
    }
}