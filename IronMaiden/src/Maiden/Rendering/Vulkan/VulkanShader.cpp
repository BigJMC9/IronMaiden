#include "maidenpch.hpp"
#include "H_VulkanShader.h"
#include "ShaderCompilation/H_VulkanShaderCompiler.h"

namespace Madam
{
	VulkanShader::VulkanShader(const std::filesystem::path& filepath)
	{
		_filepath = filepath;
	}

	VulkanShader::VulkanShader(const std::vector<uint32_t> code)
	{
		//_code = code;
		//useFilepath = false;
	}

	VulkanShader::~VulkanShader()
	{
		for (std::unordered_map<PipelineStage, VkShaderModule>::iterator it = shaderModules.begin(); it != shaderModules.end(); it++)
		{
			vkDestroyShaderModule(_device->device(), it->second, nullptr);
		}
	}

	void VulkanShader::Init(Device* device)
	{
		_device = device;
	}

	void VulkanShader::Load()
	{
		std::map<VkShaderStageFlagBits, std::vector<uint32_t>> shaderStagesCode;
		shaderStagesCode = VulkanShaderCompiler::Compile(_filepath);


		for (auto pair : shaderStagesCode) {
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = pair.second.size() * sizeof(int32_t);
			createInfo.pCode = (uint32_t*)(pair.second.data());

			VkShaderModule shader;
			if (pair.first == VK_SHADER_STAGE_VERTEX_BIT) {
				shaderModules.emplace(PipelineStage::Vert, shader);
			}
			else if (pair.first == VK_SHADER_STAGE_FRAGMENT_BIT) {
				shaderModules.emplace(PipelineStage::Frag, shader);
			}
			else if (pair.first == VK_SHADER_STAGE_COMPUTE_BIT) {
				shaderModules.emplace(PipelineStage::Comp, shader);
			}

			if (vkCreateShaderModule(_device->device(), &createInfo, nullptr, &shader) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create shader module");
			}
		}
	}

	std::vector<uint32_t> VulkanShader::LoadFile()
	{
		/*::ifstream file{_filepath.string(), std::ios::ate | std::ios::binary};

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file: " + _filepath.string());
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;*/
		return std::vector<uint32_t>();
	}
}