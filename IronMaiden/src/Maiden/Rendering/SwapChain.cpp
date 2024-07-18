#include "maidenpch.hpp"
#include "H_SwapChain.hpp"
#include "Vulkan/H_VulkanImage.h"

#ifdef max
	#undef max
#endif
#ifdef min
	#undef min
#endif
namespace Madam {
	namespace Rendering
	{

		SwapChain::SwapChain(Device& deviceRef, VkExtent2D extent)
			: device{ deviceRef }, windowExtent{ extent } {
			StartUp();
		}

		SwapChain::SwapChain(Device& deviceRef, VkExtent2D extent, Ref<SwapChain> previous)
			: device{ deviceRef }, windowExtent{ extent }, oldSwapChain{ previous } {
			StartUp();

			//clean up old swap chain
			oldSwapChain = nullptr;
		}

		void SwapChain::StartUp() {
			CreateSwapChain();
			CreateImageViews();
			CreateRenderPass();
			CreateFramebuffers();
			CreateSyncObjects();
			MADAM_CORE_INFO("SwapChain image count: {0}", imageCount());
		}

		SwapChain::~SwapChain() {
			MADAM_CORE_INFO("Cleaning up SwapChain");
			for (auto image : swapChainImages) {
				vkDestroyImageView(device.device(), image.view, nullptr);
			}
			swapChainImages.clear();

			if (swapChain != nullptr) {
				vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
				swapChain = nullptr;
			}

			for (auto framebuffer : swapChainFramebuffers) {
				vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
			}

			vkDestroyRenderPass(device.device(), renderpass, nullptr);

			// cleanup synchronization objects
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
				vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
				vkDestroyFence(device.device(), inFlightFences[i], nullptr);
			}
		}

		VkImageView SwapChain::getImageView(int index) {
			return swapChainImages[index].view;
		}

		VkRenderPass SwapChain::getRenderPass() {
			return renderpass;
		}

		VkResult SwapChain::acquireNextImage(uint32_t* imageIndex) {
			//MADAM_CORE_INFO("Current Image Index from acquireNextImage before func: {0}, Current Frame: {1}", *imageIndex, currentFrame);
			vkWaitForFences(
				device.device(),
				1,
				&inFlightFences[currentFrame],
				VK_TRUE,
				std::numeric_limits<uint64_t>::max());

			VkResult result = vkAcquireNextImageKHR( //vkAcquireNextImageKHR - Retrieve the index of the next available presentable image
				device.device(),
				swapChain,
				std::numeric_limits<uint64_t>::max(),
				imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
				VK_NULL_HANDLE,
				imageIndex); //is a pointer to a uint32_t in which the index of the next image to use (i.e. an index into the array of images returned by vkGetSwapchainImagesKHR) is returned.
			//MADAM_CORE_INFO("Current Image Index from acquireNextImage after func: {0}, Current Frame: {1}", *imageIndex, currentFrame);
			return result;
		}

		VkResult SwapChain::submitCommandBuffers(
			const VkCommandBuffer* buffers, uint32_t* imageIndex) {
			if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
				vkWaitForFences(device.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
			}
			imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = buffers;

			VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);
			if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) !=
				VK_SUCCESS) {
				throw std::runtime_error("failed to submit draw command buffer!");
			}

			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { swapChain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = imageIndex;

			auto result = vkQueuePresentKHR(device.presentQueue(), &presentInfo); //display to screen

			currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; //Increase Frame count

			return result;
		}

		void SwapChain::CreateSwapChain() {
			MADAM_CORE_INFO("Creating SwapChain");
			SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

			VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
			VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
			VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = device.surface();

			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
			uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

			if (indices.graphicsFamily != indices.presentFamily) {
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else {
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				createInfo.queueFamilyIndexCount = 0;      // Optional
				createInfo.pQueueFamilyIndices = nullptr;  // Optional
			}

			createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;

			createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

			//Validation Error, no renderpass provided (null renderpass).
			//Validation Error is normal! It is because we have not created a renderpass yet.
			MADAM_CORE_INFO("VkCreateSwapChainKHR");
			auto result = vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapChain);
			if (result != VK_SUCCESS) {
				throw std::runtime_error("failed to create swap chain!");
			}
			else if (result == VK_SUCCESS) {
				MADAM_CORE_INFO("VkCreateSwapChainKHR Done");
			}
			else {
				MADAM_CORE_ERROR("VkCreateSwapChainKHR threw unknown error, result code: {0}", result);
			}
			// we only specified a minimum number of images in the swap chain, so the implementation is
			// allowed to create a swap chain with more. That's why we'll first query the final number of
			// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
			// retrieve the handles.
			vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, nullptr);
			swapChainImages.resize(imageCount);
			std::vector<VkImage> images;
			images.resize(imageCount);
			vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, images.data());
			for (size_t i = 0; i < images.size(); i++)
			{
				swapChainImages[i].image = images[i];
			}
			swapChainImageFormat = surfaceFormat.format;
			swapChainExtent = extent;
			//VkFormat depthFormat = findDepthFormat();
			//swapChainDepthFormat = depthFormat;
		}

		void SwapChain::CreateImageViews() {
			for (size_t i = 0; i < swapChainImages.size(); i++) {
				VkImageViewCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				info.image = swapChainImages[i].image;
				info.viewType = VK_IMAGE_VIEW_TYPE_2D;
				info.format = swapChainImageFormat;
				info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				info.subresourceRange.baseMipLevel = 0;
				info.subresourceRange.levelCount = 1;
				info.subresourceRange.baseArrayLayer = 0;
				info.subresourceRange.layerCount = 1;

				if (vkCreateImageView(device.device(), &info, nullptr, &swapChainImages[i].view) !=
					VK_SUCCESS) {
					throw std::runtime_error("failed to create texture image view!");
				}
			}
		}

		//Look into this in more depth
		void SwapChain::CreateRenderPass() {

			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = getSwapChainImageFormat();
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.srcAccessMask = 0;
			dependency.srcStageMask =
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstSubpass = 0;
			dependency.dstStageMask =
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask =
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			std::vector<VkAttachmentDescription> attachments = { colorAttachment };

			//renderPass = CreateRef<Rendering::RenderPass>();

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderpass) != VK_SUCCESS) {
				MADAM_CORE_ERROR("Failed to create renderpass");
			}
			else {
				MADAM_CORE_INFO("Swapchain renderpass created");
			}
		}

		void SwapChain::CreateFramebuffers() {

			MADAM_CORE_INFO("Debug Framebuffer");
			swapChainFramebuffers.resize(imageCount());
			
			for (size_t i = 0; i < imageCount(); i++) {
				std::array<VkImageView, 1> attachments = { swapChainImages[i].view };

				VkFramebufferCreateInfo framebufferInfo;
				VkExtent2D swapChainExtent = getSwapChainExtent();
				framebufferInfo = {};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = swapChainExtent.width;
				framebufferInfo.height = swapChainExtent.height;
				framebufferInfo.layers = 1;
				framebufferInfo.renderPass = renderpass;

				if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
					MADAM_CORE_ERROR("failed to create framebuffer!");
				}
			}
		}

		void SwapChain::CreateSyncObjects() {
			imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
			imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				if (vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) !=
					VK_SUCCESS ||
					vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) !=
					VK_SUCCESS ||
					vkCreateFence(device.device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to create synchronization objects for a frame!");
				}
			}
		}

		//Update at some point maybe?
		VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
			for (const auto& availableFormat : availableFormats) {
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
					availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					return availableFormat;
				}
			}

			return availableFormats[0];
		}

		VkPresentModeKHR SwapChain::chooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& availablePresentModes) {
			for (const auto& availablePresentMode : availablePresentModes) {
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
					MADAM_CORE_INFO("Present mode: Mailbox");
					return availablePresentMode;
				}
			}
			MADAM_CORE_INFO("Present mode: V-Sync");
			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
				return capabilities.currentExtent;
			}
			else {
				VkExtent2D actualExtent = windowExtent;
				actualExtent.width = std::max(
					capabilities.minImageExtent.width,
					std::min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = std::max(
					capabilities.minImageExtent.height,
					std::min(capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
		}

		VkFormat SwapChain::findDepthFormat() {
			return device.findSupportedFormat(
				{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		}

		VkImage SwapChain::getSwapChainImage(int index)
		{
			return swapChainImages[index].image;
		}
	}
}
