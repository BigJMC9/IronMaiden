#pragma once
#include "../H_Shader.h"
#include "H_VulkanDevice.hpp"
#include "H_DescriptorManager.h"
namespace Madam
{
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::filesystem::path& filepath);
		VulkanShader(const std::vector<uint32_t> code);

		~VulkanShader() override;

		void Init(Device* device);

		void Load() override;

	private:
		std::vector<char> LoadFile();

		std::unordered_map<PipelineStage, VkShaderModule> shaderModules;
		bool useFilepath = true;

		DescSet descriptorSet;

		Device* _device = nullptr;
	};
}