#pragma once
#include "H_VulkanDevice.hpp"
namespace Madam
{
	namespace ShaderResource
	{



		struct PushConstantRange
		{
			VkShaderStageFlagBits shaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			uint32_t size = 0;
			uint32_t offset = 0;
		};
	}
}