#pragma once

#include "maidenpch.hpp"
#include "../Core/H_Device.hpp"
#include "H_SwapChain.hpp"
#include "../Core/H_Window.hpp"

//Low level rendering should be completely agnostic. see (pg 47)
namespace Madam {
	namespace Rendering {
		class Renderer {

		public:

			Renderer(Window& window, Device& device);
			~Renderer();

			Renderer(const Renderer&) = delete;
			Renderer& operator=(const Renderer&) = delete;

			void StartUp();
			void ShutDown();

			VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
			float getAspectRatio() const {
				return swapChain->extentAspectRatio();
			}
			bool isFrameInProgress() const { return isFrameStarted; }

			VkCommandBuffer getCurrentCommandBuffer() const {
				MADAM_CORE_ASSERT(isFrameStarted, "Cannot get command buffer when frame not in progress");
				return commandBuffers[currentFrameIndex];
			}

			int getFrameIndex() const {
				MADAM_CORE_ASSERT(isFrameStarted, "Cannot get command buffer when frame not in progress");
				return currentFrameIndex;
			}

			ImGui_ImplVulkan_InitInfo* getImGuiInitInfo() {
				ImGui_ImplVulkan_InitInfo* init_info = &ImGui_ImplVulkan_InitInfo();
				init_info->Device = device.device();
				init_info->PipelineCache = VK_NULL_HANDLE;
				init_info->Allocator = VK_NULL_HANDLE;
				init_info->MinImageCount = swapChain->imageCount();
				init_info->ImageCount = swapChain->imageCount();
				init_info->CheckVkResultFn = nullptr;
				device.getImGuiInitInfo(*init_info);
				return init_info;
			};

			VkCommandBuffer beginFrame();
			void endFrame();
			void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
			void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		private:
			void createCommandBuffers();
			void freeCommandBuffers();
			void recreateSwapChain();

			Window& window;
			Device& device;
			std::unique_ptr<SwapChain> swapChain;
			std::vector<VkCommandBuffer> commandBuffers;

			uint32_t currentImageIndex;
			int currentFrameIndex = 0;
			bool isFrameStarted = false;
			bool isRunning = false;
		};
	}
}