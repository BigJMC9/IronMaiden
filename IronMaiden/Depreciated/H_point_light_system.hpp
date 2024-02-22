#pragma once

#include "H_JCVB_camera.hpp"
#include "H_JCVB_device.hpp"
#include "H_JCVB_frame_info.hpp"
#include "H_JCVB_game_object.hpp"
#include "H_JCVB_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace Digestion {
	class PointLightSystem {

	public:

		PointLightSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem &operator=(const PointLightSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo &frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
		
		Device &jcvbDevice;

		std::unique_ptr<Pipeline> jcvbPipeline; //smart pointer
		VkPipelineLayout pipelineLayout;
	};
}