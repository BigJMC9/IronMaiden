#include "maidenpch.hpp"
#include "H_Renderer.hpp"
#include "H_Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_include.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
namespace Madam {
	namespace Rendering {
		Renderer* Renderer::instance = nullptr;
		bool Renderer::instanceFlag = false;

		Renderer::Renderer(Window& window, Device& device) : window{ window }, device{ device } {

		}

		Renderer::~Renderer() 
		{
			if (isRunning) 
			{
				MADAM_CORE_WARN("Renderer prematurally shutdown");
				Deinit();
			}
		}

		void Renderer::Init() 
		{
			isRunning = true;
			instance = this;
			instanceFlag = true;
			RecreateSwapChain();
			frames.resize(swapChain->imageCount());
			CreateCommandBuffers();
			CreateMainRenderPass();
		}

		void Renderer::Deinit() 
		{
			FreeCommandBuffers();
			//freeImageBuffers(0);
			isRunning = false;
			instance = nullptr;
			instanceFlag = false;
		}

		//Recreate swapchain if window is resized
		void Renderer::RecreateSwapChain() 
		{
			hasSwapChainRecreated = true;
			MADAM_CORE_INFO("Recreating SwapChain");
			auto extent = window.getExtent();
			while (extent.width == 0 || extent.height == 0) 
			{
				auto extent = window.getExtent();
				glfwWaitEvents();
			}
			vkDeviceWaitIdle(device.device());

			if (swapChain == nullptr) 
			{
				swapChain = std::make_unique<SwapChain>(device, extent);
			}
			else 
			{
				Ref<SwapChain> oldSwapChain = std::move(swapChain);
				swapChain = std::make_unique<SwapChain>(device, extent, oldSwapChain);

				if (!oldSwapChain->compareSwapFormats(*swapChain.get())) 
				{
					MADAM_CORE_ERROR("Swap chain image(or depth) format has changed!");
				}

				WindowData windowData;
				windowData.width = window.getWidth();
				windowData.height = window.getHeight();
				windowData.windowName = window.getName();
				window.resetWindowResizedFlag();
				//temporary, update this
				bool isRecreate = false;
				for (size_t i = 0; i < frames.size(); i++)
				{
					if (frames[i].width != swapChain->getSwapChainExtent().width || frames[i].height != swapChain->getSwapChainExtent().height) 
					{
						isRecreate = true;
						break;
					}
				}
				if (isRecreate) 
				{
					for (size_t i = 0; i < frames.size(); i++)
					{ 
						vkDestroyFramebuffer(device.device(), frames[i].images[0].frameBuffer, nullptr);
						vkDestroyImageView(device.device(), frames[i].images[0].imageView, nullptr);
						vkDestroyImage(device.device(), frames[i].images[0].image, nullptr);
						vkFreeMemory(device.device(), frames[i].images[0].imageMemory, nullptr);
						vkDestroyImageView(device.device(), frames[i].images[1].imageView, nullptr);
						vkDestroyImage(device.device(), frames[i].images[1].image, nullptr);
						vkFreeMemory(device.device(), frames[i].images[1].imageMemory, nullptr);
					}
					frames.clear();
					frames.resize(swapChain->imageCount());
					CreateMainRenderPass();
				}
				Events::WindowResizeEvent e(windowData);
				Events::EventSystem::Get().PushEvent(&e, true);
			}
		}

		//Create command buffers from command pool for each frame and render pass
		//??
		void Renderer::CreateCommandBuffers() 
		{
			commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = device.getCommandPool();
			allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

			if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) 
			{
				MADAM_CORE_ERROR("Failed to allocate command buffers!");
			}

		}

		void Renderer::FreeCommandBuffers() 
		{
			vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
			commandBuffers.clear();
		}

		bool Renderer::BeginFrame() 
		{
			MADAM_CORE_ASSERT(!isFrameStarted, "Can't call beginFrame while already in progress");

			auto result = swapChain->acquireNextImage(&currentImageIndex);
			if (result == VK_ERROR_OUT_OF_DATE_KHR) 
			{
				MADAM_CORE_INFO("Recreating Swapchain on begin frame");
				RecreateSwapChain();
				return false;
			}

			if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
			{
				throw std::runtime_error("Failed to acquire swap chain image!");
			}

			isFrameStarted = true;
			return true;
		}

		void Renderer::EndFrame() 
		{
			MADAM_CORE_ASSERT(isFrameStarted, "Can't call endFrame while frame is not in progress");
			//MADAM_CORE_INFO("Ending Frame");
			auto commandBuffer = GetCurrentCommandBuffer();

			if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
			{
				MADAM_CORE_ERROR("Failed to record command buffer!");
			}

			auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentCommandBufferIndex);
			//mapImageBuffer(currentFrameIndex);
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) 
			{
				MADAM_CORE_INFO("Recreating Swapchain on end frame");
				RecreateSwapChain();
			}
			else if (result != VK_SUCCESS) 
			{
				MADAM_CORE_ERROR("Failed to present swap chain image!");
			}

			isFrameStarted = false;
			currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
		}

		VkCommandBuffer Renderer::BeginCommandBuffer() const 
		{
			MADAM_CORE_ASSERT(isFrameStarted, "Can't call beginCommandBuffer if frame is not in progress");

			auto commandBuffer = GetCurrentCommandBuffer();

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to begin recording command buffer!");
			}
			return commandBuffer;
		}

		void Renderer::BeginRenderPass(VkCommandBuffer commandBuffer, int renderIndex) 
		{
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPasses[renderIndex];
			renderPassInfo.framebuffer = frames[currentFrameIndex].images[renderIndex].frameBuffer;

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();
			
			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
			viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
			if (hasSwapChainRecreated)
			{
				MADAM_CORE_INFO("Viewport Width: {0}, Height: {1}", viewport.width, viewport.height);
				hasSwapChainRecreated = false;
			}
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			VkExtent2D extent = { viewportWidth, viewportHeight };
			VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent()};
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			renderpassIndex++;
			
			Events::NextRenderPassEvent e(renderpassIndex);
			Events::EventSystem::Get().PushEvent(&e, true);

		}

		void Renderer::EndRenderPass(VkCommandBuffer commandBuffer) const 
		{
			//saveAsImage(commandBuffer);
			MADAM_CORE_ASSERT(isFrameStarted, "Can't call endSwapChainRenderPass if frame is not in progress");
			MADAM_CORE_ASSERT(commandBuffer == GetCurrentCommandBuffer(), "Can't end render pass on command buffer from a different frame");
			vkCmdEndRenderPass(commandBuffer);
		}

		//Update this
		void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer) 
		{
			MADAM_CORE_ASSERT(isFrameStarted, "Can't call beginSwapChainRenderPass if frame is not in progress");
			MADAM_CORE_ASSERT(commandBuffer == GetCurrentCommandBuffer(), "Can't begin render pass on command buffer from a different frame");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = swapChain->getRenderPass(); //Gets VkRenderPass from swapchain
			renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex); //Gets VkFramebuffer from swapchain

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
			viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent() };
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			renderpassIndex++;

			Events::NextRenderPassEvent e(renderpassIndex);
			Events::EventSystem::Get().PushEvent(&e, true);
		}
		void Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer) 
		{
			MADAM_CORE_ASSERT(isFrameStarted, "Can't call endSwapChainRenderPass if frame is not in progress");
			MADAM_CORE_ASSERT(commandBuffer == GetCurrentCommandBuffer(), "Can't end render pass on command buffer from a different frame");
			vkCmdEndRenderPass(commandBuffer);
			renderpassIndex = -1;
		}

		VkRenderPass Renderer::CreateRenderPass(std::vector<VkAttachmentDescription> attachments, std::vector<VkSubpassDescription> subpass, std::vector<VkSubpassDependency> dependencies)
		{
			VkRenderPass renderPass;
			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = static_cast<uint32_t>(subpass.size());
			renderPassInfo.pSubpasses = subpass.data();
			renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
			renderPassInfo.pDependencies = dependencies.data();

			if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) 
			{
				MADAM_CORE_ERROR("Failed to create render pass!");
			}
			renderPasses.push_back(renderPass);
			return renderPass;
		}


		void Renderer::CreateMainRenderImages() 
		{
			for (size_t i = 0; i < frames.size(); i++)
			{
				VkImage image;
				VkImageCreateInfo imageInfo{};
				imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageInfo.imageType = VK_IMAGE_TYPE_2D;
				imageInfo.extent.width = swapChain->getSwapChainExtent().width;
				imageInfo.extent.height = swapChain->getSwapChainExtent().height;
				imageInfo.extent.depth = 1;
				imageInfo.mipLevels = 1;
				imageInfo.arrayLayers = 1;
				imageInfo.format = swapChain->getSwapChainImageFormat();
				imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				imageInfo.flags = 0;

				if (vkCreateImage(device.device(), &imageInfo, nullptr, &image) != VK_SUCCESS) 
				{
					throw std::runtime_error("Failed to create image!");
				}

				VkDeviceMemory imageMemory;
				device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

				VkImageViewCreateInfo viewInfo = {};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.image = image;
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = swapChain->getSwapChainImageFormat();
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewInfo.subresourceRange.baseMipLevel = 0;
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.baseArrayLayer = 0;
				viewInfo.subresourceRange.layerCount = 1;
				viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewInfo.components.a = VK_COMPONENT_SWIZZLE_ONE;

				ImageInfo imageData;
				imageData.image = image;
				imageData.imageMemory = imageMemory;

				if (vkCreateImageView(device.device(), &viewInfo, nullptr, &imageData.imageView) != VK_SUCCESS) 
				{
					throw std::runtime_error("failed to create texture image view!");
				}

				frames[i].images.push_back(imageData);
				frames[i].width = swapChain->getSwapChainExtent().width;
				frames[i].height = swapChain->getSwapChainExtent().height;
				//setImageBuffer(i);

				VkImage depthImage;
				VkImageCreateInfo depthImageInfo{};
				depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
				depthImageInfo.extent.width = swapChain->getSwapChainExtent().width;
				depthImageInfo.extent.height = swapChain->getSwapChainExtent().height;
				depthImageInfo.extent.depth = 1;
				depthImageInfo.mipLevels = 1;
				depthImageInfo.arrayLayers = 1;
				depthImageInfo.format = swapChain->findDepthFormat();
				depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				depthImageInfo.flags = 0;


				VkDeviceMemory depthImageMemory;
				device.createImageWithInfo(depthImageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);

				VkImageViewCreateInfo depthViewInfo = {};
				depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				depthViewInfo.image = depthImage;
				depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				depthViewInfo.format = swapChain->findDepthFormat();
				depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				depthViewInfo.subresourceRange.baseMipLevel = 0;
				depthViewInfo.subresourceRange.levelCount = 1;
				depthViewInfo.subresourceRange.baseArrayLayer = 0;
				depthViewInfo.subresourceRange.layerCount = 1;

				ImageInfo depthImageData;
				depthImageData.image = depthImage;
				depthImageData.imageMemory = depthImageMemory;

				if (vkCreateImageView(device.device(), &depthViewInfo, nullptr, &depthImageData.imageView) != VK_SUCCESS) 
				{
					throw std::runtime_error("failed to create texture image view!");
				}

				frames[i].images.push_back(depthImageData);
			}
			//std::vector<ImageView> renderImages;

			/*MultiSample multiSample = Rendering::MultiSample::None;

			ImageFormat _format;
			switch (swapChain->findDepthFormat())
			{
			case VK_FORMAT_D32_SFLOAT:
				_format = Rendering::ImageFormat::D32F;
				break;
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				_format = Rendering::ImageFormat::D32FS8;
				break;
			case VK_FORMAT_D24_UNORM_S8_UINT:
				_format = Rendering::ImageFormat::D24S8;
				break;
			default:
				MADAM_CORE_ERROR("Unsupported Depth Format!");
				break;
			}

			MADAM_CORE_INFO("Depth Format: {0}", static_cast<uint32_t>(_format));

			Rendering::ImageLayoutType layout = Rendering::ImageLayoutType::DepthStencil;
			Rendering::ImageUsageType usage = Rendering::ImageUsageType::Attachment;

			Rendering::ImageSpecification imageSpecs;
			imageSpecs.format = _format;
			imageSpecs.width = swapChain->getSwapChainExtent().width;
			imageSpecs.height = swapChain->getSwapChainExtent().height;
			imageSpecs.layout = layout;
			imageSpecs.usuage = usage;
			imageSpecs.multiSample = multiSample;
			imageSpecs.mipLevels = 1;
			imageSpecs.arrayLayers = 1;
			imageSpecs.depth = 1;


			Image depthImage = Image(imageSpecs);*/

		}

		void Renderer::CreateMainRenderPass() 
		{

			MADAM_CORE_INFO("Creating Main Render Pass");
			CreateMainRenderImages();

			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = swapChain->getSwapChainImageFormat();
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = swapChain->findDepthFormat();
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
			std::vector<VkSubpassDescription> subpasses = { subpass };
			std::vector<VkSubpassDependency> dependencies = { dependency };

			VkRenderPass renderPass = CreateRenderPass(attachments, subpasses, dependencies);

			for (size_t i = 0; i < frames.size(); i++)
			{
				std::array<VkImageView, 2> imageAttachments = { frames[i].images[0].imageView, frames[i].images[1].imageView };


				VkFramebufferCreateInfo framebufferInfo = {};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				framebufferInfo.pAttachments = imageAttachments.data();
				framebufferInfo.width = frames[i].width;
				framebufferInfo.height = frames[i].height;
				framebufferInfo.layers = 1;

				if (vkCreateFramebuffer
				(
					device.device(),
					&framebufferInfo,
					nullptr,
					&frames[i].images[0].frameBuffer
				) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to create framebuffer!");
				}
			}
		}

		void Renderer::PipelineBarrier(VkCommandBuffer commandBuffer, bool isSwapchain, bool isSwitch, int frameIndex, int renderIndex)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			if (isSwitch) 
			{
				barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			}
			else 
			{
				barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			if (isSwapchain) 
			{
				barrier.image = swapChain->getSwapChainImage(frameIndex);
			}
			else 
			{
				barrier.image = frames[frameIndex].images[renderIndex].image;
			}
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			if (isSwitch) 
			{
				barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT; //Error here
				barrier.dstAccessMask = 0;

				vkCmdPipelineBarrier
				(
					commandBuffer,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // The pipeline stage at which the transition begins
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // The pipeline stage at which the transition ends
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);
			}
			else 
			{
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier
				(
					commandBuffer,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // The pipeline stage at which the transition begins
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // The pipeline stage at which the transition ends
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);
			}
		}
	}
}