#pragma once

#include "maidenpch.hpp"
//#include "../Core/H_Device.hpp"
#include "H_SwapChain.hpp"
#include "../Interfaces/H_Interface.h"
#include "../Events/H_EventSystem.h"

//Low level rendering should be completely agnostic. see (pg 47)
namespace Madam {
	namespace Rendering {

		struct ImageInfo {
			std::string debugName;
			VkImageView imageView;
			VkImage image;
			VkDeviceMemory imageMemory;
			VkFramebuffer frameBuffer;
		};
		struct Frame {
			std::vector<ImageInfo> images;
			uint32_t width, height;
		};
		struct CommandBufferGroup {
			std::vector<VkCommandBuffer> commandBuffers;
		};

		class Renderer {
		public:

			Renderer(Window& window, Device& device);
			~Renderer();

			Renderer(const Renderer&) = delete;
			Renderer& operator=(const Renderer&) = delete;
			Renderer& operator=(Renderer&&) = delete;

			void init();
			void deinit();

			VkRenderPass getSwapChainRenderPass() const 
			{ 
				return swapChain->getRenderPass(); 
			}
			VkRenderPass getMainRenderPass() const 
			{ 
				return renderPasses[0]; 
			}
			float getAspectRatio() const 
			{
				return swapChain->extentAspectRatio();
			}
			bool isFrameInProgress() const 
			{ 
				return isFrameStarted; 
			}

			VkCommandBuffer getCurrentCommandBuffer() const 
			{
				MADAM_CORE_ASSERT(isFrameStarted, "Cannot get command buffer when frame not in progress");
				return commandBuffers[currentFrameIndex];
			}

			int getFrameIndex() const 
			{
				MADAM_CORE_ASSERT(isFrameStarted, "Cannot get command buffer when frame not in progress");
				return currentFrameIndex;
			}

			VkImageView getImageView(int index) const 
			{
				return frames[currentFrameIndex].images[index].imageView;
			}

			uint32_t getViewportWidth() const 
			{ 
				return viewportWidth; 
			}
			uint32_t getViewportHeight() const 
			{ 
				return viewportHeight; 
			}

			ImGui_ImplVulkan_InitInfo getImGuiInitInfo() 
			{
				ImGui_ImplVulkan_InitInfo init_info = ImGui_ImplVulkan_InitInfo();
				init_info.Device = device.device();
				init_info.PipelineCache = VK_NULL_HANDLE;
				init_info.Allocator = VK_NULL_HANDLE;
				init_info.MinImageCount = static_cast<uint32_t>(swapChain->imageCount());
				init_info.ImageCount = static_cast<uint32_t>(swapChain->imageCount());
				init_info.CheckVkResultFn = nullptr;
				device.getImGuiInitInfo(&init_info);
				return init_info;
			};

			static Renderer& Get() 
			{
				MADAM_CORE_ASSERT(instanceFlag, "Renderer instance not created");
				if (instance == nullptr) {
					MADAM_CORE_ERROR("Renderer instance is null pointer");
				}
				return *instance;
			}

			static VkDevice GetLogicDevice() {
				MADAM_CORE_ASSERT(instanceFlag, "Renderer instance not created");
				return Get().device.device_;
			}

			static Device& GetDevice() {
				MADAM_CORE_ASSERT(instanceFlag, "Renderer instance not created");
				return Get().device;
			}

			static SwapChain& GetSwapChain() {
				MADAM_CORE_ASSERT(instanceFlag, "Renderer instance not created");
				if (Get().swapChain == nullptr) {
					MADAM_CORE_ERROR("SwapChain is null pointer");
				}
				return *Get().swapChain;
			}

			//Needs to be updated to abstract away from the VkRenderPass, we don't want to accidentally change renderpass settings while the engine is running
			const std::vector<VkRenderPass> getRenderPasses() 
			{
				return renderPasses;
			}

			bool beginFrame();
			void endFrame();
			VkCommandBuffer beginCommandBuffer();
			void endCommandBuffer(VkCommandBuffer commandBuffer);
			void beginRenderPass(VkCommandBuffer commandBuffer, int renderpassIndex);
			void endRenderPass(VkCommandBuffer commandBuffer);
			void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
			void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

			VkRenderPass createRenderPass(std::vector<VkAttachmentDescription> attachments, std::vector<VkSubpassDescription> subpass, std::vector<VkSubpassDependency> dependencies);

			void PipelineBarrier(VkCommandBuffer commandBuffer, bool isSwapchain, bool isSwitch, int frameIndex, int renderIndex);

		private:
			void createCommandBuffers();
			void createMainRenderImages();
			void createMainRenderPass();
			void freeCommandBuffers();
			void recreateSwapChain();
			//VkRenderPass createRenderPass(std::vector<VkAttachmentDescription> attachments, std::vector<VkSubpassDescription> subpass, std::vector<VkSubpassDependency> dependencies, bool isSwapChain);

			static Renderer* instance;
			static bool instanceFlag;

			Window& window;
			Device& device;
			Scope<SwapChain> swapChain = nullptr;
			std::vector<VkCommandBuffer> commandBuffers;

			uint32_t currentImageIndex;
			int currentFrameIndex = 0;
			uint32_t currentCommandBufferIndex = 0;
			int renderpassIndex = -1;
			bool isFrameStarted = false;
			bool isRunning = false;
			std::vector<VkRenderPass> renderPasses;
			std::vector<Frame> frames;
			std::vector<CommandBufferGroup> commandBufferGroups;
			bool hasSwapChainRecreated = false;
			//std::vector<ImageView> imageViews;

			uint32_t viewportWidth = 800, viewportHeight = 450;
		};
	}
}