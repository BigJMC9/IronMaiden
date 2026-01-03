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
#include "../Core/H_CoreUtils.hpp"

namespace Madam {
    //Create solution that don't involve using virtual functions.
    namespace Rendering {

        struct DefaultPushConstantData {
            glm::mat4 modelMatrix{ 1.f };
            glm::mat4 normalMatrix{ 1.f };
        };

        struct Q12Constants{
            glm::mat4 modelMatrix{ 1.f };
            glm::mat4 normalMatrix{ 1.f };
            float dx0;
            float dx1;
        };

        struct Q13Constants
        {
            glm::mat4 modelMatrix{ 1.f };
            glm::mat4 normalMatrix{ 1.f };
            float dx0;
            float dx1;
            float dx2;
            float dx3;
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
            virtual void recreatePipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            virtual void updatePipelineState();

			bool isFirstFrame = true;

            Device& device;

            Scope<Pipeline> pipeline = nullptr;
            Scope<Pipeline> oldPipeline = nullptr;
            int tick = 0;
            VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
            VkPipelineLayout oldPipelineLayout = VK_NULL_HANDLE;
            friend class RenderStack;
		};

        class MADAM_API CustomLayer : public RenderLayer {

        public:
            CustomLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "CustomLayer");
            ~CustomLayer();

            void render(FrameInfo& frameInfo) override;
        protected:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
            void createPipeline(VkRenderPass renderPass) override;

        };

        class MADAM_API GridRenderLayer : public RenderLayer {

        public:
            GridRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "GridLayer");
            ~GridRenderLayer();

            void render(FrameInfo& frameInfo) override;

        protected:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
            void createPipeline(VkRenderPass renderPass) override;

        };

        class MADAM_API SkyboxRenderLayer : public RenderLayer {

        public:
            SkyboxRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "SkyboxLayer");
            ~SkyboxRenderLayer();

            void render(FrameInfo& frameInfo) override;

        protected:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
            void createPipeline(VkRenderPass renderPass) override;

            Scope<DescriptorSetLayout> skyboxRenderSystemLayout;
            Scope<Buffer> skyboxBuffer;
            Ref<Texture> noiseTexture;
            Ref<StaticMesh> skybox = StaticMesh::Create("resources\\models\\skybox.obj");
        };

        class MADAM_API TextureRenderLayer : public RenderLayer {

        public:
            TextureRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "TextureLayer");
            ~TextureRenderLayer();

            void render(FrameInfo& frameInfo) override;

        protected:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
            void createPipeline(VkRenderPass renderPass) override;

            Scope<DescriptorSetLayout> renderSystemLayout;
        };

        class MADAM_API PointLightRenderLayer : public RenderLayer {

        public:
            PointLightRenderLayer(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string _name = "PointLightLayer");
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
            void reloadPipeline(int renderLayer, VkRenderPass renderPass, Scope<DescriptorSetLayout>& globalDescriptorSetLayout);
            const std::vector<Ref<RenderLayer>>& getRenderLayers() const {
                return renderSystems;
            }

            Renderer& renderer;

        private:
            bool isRunning = false;
            Device& device;
            std::vector<Ref<RenderLayer>> renderSystems;
        };
    }
}