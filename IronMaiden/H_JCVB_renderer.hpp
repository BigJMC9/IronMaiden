#pragma once

#include "maidenpch.hpp"
#include "H_JCVB_device.hpp"
#include "H_JCVB_swap_chain.hpp"
#include "H_JCVB_window.hpp"

namespace Madam {
	namespace Rendering {
		class Renderer {

		public:

			Renderer(Window& window, Device& device);
			~Renderer();

			Renderer(const Renderer&) = delete;
			Renderer& operator=(const Renderer&) = delete;

			VkRenderPass getSwapChainRenderPass() const { return jcvbSwapChain->getRenderPass(); }
			float getAspectRatio() const {
				return jcvbSwapChain->extentAspectRatio();
			}
			bool isFrameInProgress() const { return isFrameStarted; }

			VkCommandBuffer getCurrentCommandBuffer() const {
				assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
				return commandBuffers[currentFrameIndex];
			}

			int getFrameIndex() const {
				assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
				return currentFrameIndex;
			}

			VkCommandBuffer beginFrame();
			void endFrame();
			void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
			void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		private:
			void createCommandBuffers();
			void freeCommandBuffers();
			void recreateSwapChain();

			Window& jcvbWindow;
			Device& jcvbDevice;
			std::unique_ptr<SwapChain> jcvbSwapChain;
			std::vector<VkCommandBuffer> commandBuffers;

			uint32_t currentImageIndex;
			int currentFrameIndex = 0;
			bool isFrameStarted = false;
		};
	}
}