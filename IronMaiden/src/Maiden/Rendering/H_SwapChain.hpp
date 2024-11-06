#pragma once

#include "maidenpch.hpp"
#include "Vulkan/H_VulkanDevice.hpp"
#include "../Core/H_Utils.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

namespace Madam {

	namespace Rendering {
		class VulkanImage;
		struct SwapChainImage
		{
			VkImage image;
			VkImageView view;
			VkDeviceMemory memory;
		};
		class SwapChain {
		public:
			static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

			SwapChain(Device& deviceRef, VkExtent2D windowExtent);
			SwapChain(Device& deviceRef, VkExtent2D windowExtent, Ref<SwapChain> previous);
			~SwapChain();

			SwapChain(const SwapChain&) = delete;
			SwapChain& operator=(SwapChain&&) = delete;
			Device& operator=(const SwapChain&) = delete;

			VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
			VkRenderPass getRenderPass();
			VkImageView getImageView(int index);
			size_t imageCount() { return swapChainImages.size(); }
			VkImage getSwapChainImage(int index);
			VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
			VkExtent2D getSwapChainExtent() { return swapChainExtent; } //dimension of the swap chain image
			uint32_t width() { return swapChainExtent.width; }
			uint32_t height() { return swapChainExtent.height; }

			float extentAspectRatio() {
				return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
			}
			VkFormat findDepthFormat();

			VkResult acquireNextImage(uint32_t* imageIndex);
			VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

			bool compareSwapFormats(const SwapChain& swapChain) const {
				return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat;
			}

		private:
			void StartUp();
			void CreateSwapChain();
			void CreateImageViews();
			void CreateRenderPass();
			void CreateFramebuffers();
			void CreateSyncObjects();

			int32_t viewportWidth;
			int32_t viewportHeight;

			// Helper functions
			VkSurfaceFormatKHR chooseSwapSurfaceFormat(
				const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR chooseSwapPresentMode(
				const std::vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

			VkFormat swapChainImageFormat;
			VkFormat swapChainDepthFormat;
			VkExtent2D swapChainExtent;

			std::vector<VkFramebuffer> swapChainFramebuffers;
			VkRenderPass renderpass = VK_NULL_HANDLE;

			std::vector<SwapChainImage> swapChainImages;

			Device& device;
			VkExtent2D windowExtent;

			VkSwapchainKHR swapChain;
			Ref<SwapChain> oldSwapChain;

			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;
			std::vector<VkFence> imagesInFlight;
			size_t currentFrame = 0;

		};

	}
}
