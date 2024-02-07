#pragma once

#include "H_JCVB_camera.hpp"
#include "H_JCVB_device.hpp"
#include "H_JCVB_frame_info.hpp"
#include "H_Game_Object.hpp"
#include "H_JCVB_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace Digestion {
	class SimpleRenderSystem {

	public:

		SimpleRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo &frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
		
		bool isFirstFrame = true;

		Device &jcvbDevice;

		std::unique_ptr<Pipeline> jcvbPipeline; //smart pointer
		VkPipelineLayout pipelineLayout;
	};
}