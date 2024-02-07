#include "H_JCVB_renderer.hpp"

// std
#include <stdexcept>
#include <array>
#include <cassert>

namespace Digestion {
	namespace Rendering {
		Renderer::Renderer(Window& window, Device& device) : jcvbWindow{ window }, jcvbDevice{ device } {
			recreateSwapChain();
			createCommandBuffers();
		}

		Renderer::~Renderer() {
			freeCommandBuffers();
		}

		void Renderer::recreateSwapChain() {
			auto extent = jcvbWindow.getExtent();
			while (extent.width == 0 || extent.height == 0) {
				extent = jcvbWindow.getExtent();
				glfwWaitEvents();
			}
			vkDeviceWaitIdle(jcvbDevice.device());

			if (jcvbSwapChain == nullptr) {
				jcvbSwapChain = nullptr;
				jcvbSwapChain = std::make_unique<SwapChain>(jcvbDevice, extent);
			}
			else {
				std::shared_ptr<SwapChain> oldSwapChain = std::move(jcvbSwapChain);
				jcvbSwapChain = std::make_unique<SwapChain>(jcvbDevice, extent, oldSwapChain);

				if (!oldSwapChain->compareSwapFormats(*jcvbSwapChain.get())) {
					throw std::runtime_error("Swap chain image(or depth) format has changed!");
				}
			}

		}

		void Renderer::createCommandBuffers() {
			commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = jcvbDevice.getCommandPool();
			allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

			if (vkAllocateCommandBuffers(jcvbDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
				throw std::runtime_error("Failed to allocate command buffers!");
			}

		}

		void Renderer::freeCommandBuffers() {
			vkFreeCommandBuffers(jcvbDevice.device(), jcvbDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
			commandBuffers.clear();
		}

		VkCommandBuffer Renderer::beginFrame() {
			assert(!isFrameStarted && "Can't call beginFrame while already in progress");

			auto result = jcvbSwapChain->acquireNextImage(&currentImageIndex);
			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				recreateSwapChain();
				return nullptr;
			}

			if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
				throw std::runtime_error("Failed to acquire swap chain image!");
			}

			isFrameStarted = true;

			auto commandBuffer = getCurrentCommandBuffer();

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("Failed to begin recording command buffer!");
			}

			return commandBuffer;
		}

		void Renderer::endFrame() {
			assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
			auto commandBuffer = getCurrentCommandBuffer();

			if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
				throw std::runtime_error("Failed to record command buffer!");
			}

			auto result = jcvbSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || jcvbWindow.wasWindowResized()) {
				jcvbWindow.resetWindowResizedFlag();
				recreateSwapChain();
			}
			else if (result != VK_SUCCESS) {
				throw std::runtime_error("Failed to present swap chain image!");
			}

			isFrameStarted = false;
			currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
		}
		void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
			assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
			assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = jcvbSwapChain->getRenderPass();
			renderPassInfo.framebuffer = jcvbSwapChain->getFrameBuffer(currentImageIndex);

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = jcvbSwapChain->getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(jcvbSwapChain->getSwapChainExtent().width);
			viewport.height = static_cast<float>(jcvbSwapChain->getSwapChainExtent().height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			VkRect2D scissor{ {0, 0}, jcvbSwapChain->getSwapChainExtent() };
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		}
		void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
			assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
			assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
			vkCmdEndRenderPass(commandBuffer);
		}
	}
}