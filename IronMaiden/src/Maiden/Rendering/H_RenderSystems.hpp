#pragma once

#include "maidenpch.hpp"
//#include "H_JCVB_camera.hpp"
#include "H_DescriptorSetLayout.hpp"
#include "Vulkan/H_VulkanDevice.hpp"
#include "FrameInfo.hpp"
#include "../Scene/Components.hpp"
#include "../Scene/H_Entity.hpp"
#include "H_Pipeline.hpp"
#include "H_Renderer.hpp"

#define INCLUDE_UTILS
#include "../Utils/H_Utils.h"

namespace Madam {
    //Create solution that don't involve using virtual functions.
    namespace Rendering {

        struct DefaultPushConstantData {
            glm::mat4 modelMatrix{ 1.f };
            glm::mat4 normalMatrix{ 1.f };
        };

        struct PointLightPushConstants {
            glm::vec4 position{};
            glm::vec4 color{};
            float radius;
        };

        struct GridPushConstants {
            float nearPlane;
            float farPlane;
        };

        struct SkyboxBuffer {
            glm::vec3 resolution;
            float time;
            glm::vec3 textureResolution;
        };


        //Update this!!
		class MADAM_API RenderLayer {

		public:
            RenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "Default");
			~RenderLayer();

			std::string name = "Default";

            virtual void render(FrameInfo& frameInfo);

		protected:
            virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            virtual void createPipeline(VkRenderPass renderPass);

			bool isFirstFrame = true;

            Device& device;

            Scope<Pipeline> pipeline;
            VkPipelineLayout pipelineLayout;

		};

        class MADAM_API GridRenderLayer : public RenderLayer {

        public:
            GridRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "Default");
            ~GridRenderLayer();

            void render(FrameInfo& frameInfo) override;

        protected:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
            void createPipeline(VkRenderPass renderPass) override;

        };

        class MADAM_API SkyboxRenderLayer : public RenderLayer {

        public:
            SkyboxRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "Default");
            ~SkyboxRenderLayer();

            void render(FrameInfo& frameInfo) override;

        protected:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
            void createPipeline(VkRenderPass renderPass) override;

            Scope<DescriptorSetLayout> skyboxRenderSystemLayout;
            Scope<Buffer> skyboxBuffer;
            Ref<Texture> noiseTexture;
            SkyboxBuffer skyboxBufferData;
        };

        class MADAM_API TextureRenderLayer : public RenderLayer {

        public:
            TextureRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "Default");
            ~TextureRenderLayer();

            void render(FrameInfo& frameInfo) override;

        protected:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
            void createPipeline(VkRenderPass renderPass) override;

            Scope<DescriptorSetLayout> renderSystemLayout;
        };

        class MADAM_API PointLightRenderLayer : public RenderLayer {

        public:
            PointLightRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "Default");
            ~PointLightRenderLayer();

            void render(FrameInfo& frameInfo) override;
            

        protected:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
            void createPipeline(VkRenderPass renderPass) override;
        };

        class MADAM_API RenderStack {

        public:
            RenderStack(Device& device, Renderer& renderer) : device{ device }, renderer{ renderer } {}
            ~RenderStack();

            void init();
            void deinit();

            void initialize(Scope<DescriptorSetLayout>& globalSetLayout);
            void render(FrameInfo& frameInfo);
            bool switchRenderSystems(int first, int second);
            const std::vector<Ref<RenderLayer>>& getRenderLayers() const {
                return renderSystems;
            }

            /*void addSystem(const RenderLayer& renderSystem) {
                renderSystems.push_back(std::make_unique<RenderLayer>(std::move(renderSystem)));
            }*/

            Renderer& renderer;

        private:
            bool isRunning = false;
            Device& device;
            std::vector<Ref<RenderLayer>> renderSystems;
        };
    }
}