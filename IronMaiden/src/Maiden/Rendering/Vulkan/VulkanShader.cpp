#include "maidenpch.hpp"
#include "H_VulkanShader.h"

namespace Madam
{
	VulkanShader::VulkanShader(const std::filesystem::path& filepath)
	{
		_filepath = filepath;
	}

	VulkanShader::VulkanShader(const std::vector<uint32_t> code)
	{
		_code = code;
		useFilepath = false;
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
		VkShaderModuleCreateInfo createInfo{};
		if (useFilepath)
		{
			std::vector<char> codeBuffer(LoadFile());
			const uint32_t* code = reinterpret_cast<const uint32_t*>(codeBuffer.data());
			size_t numElements = codeBuffer.size() / sizeof(uint32_t);
			_code = std::vector<uint32_t>(code, code + numElements);
		}

		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = _code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(_code.data());

		VkShaderModule shader;
		shaderModules.emplace(PipelineStage::Vert, shader);

		if (vkCreateShaderModule(_device->device(), &createInfo, nullptr, &shaderModules[PipelineStage::Vert]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create shader module");
		}
	}

	std::vector<char> VulkanShader::LoadFile()
	{
		std::ifstream file{ _filepath.string(), std::ios::ate | std::ios::binary};

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file: " + _filepath.string());
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}
}