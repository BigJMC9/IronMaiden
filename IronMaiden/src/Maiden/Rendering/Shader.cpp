#include "maidenpch.hpp"
#include "H_Shader.h"
#include "Vulkan/H_VulkanShader.h"

namespace Madam
{
	Ref<Shader> Shader::Create(const std::filesystem::path& filepath)
	{
		return CreateRef<VulkanShader>(filepath);
	}

	Ref<Shader> Shader::Create(const std::vector<uint32_t> _code)
	{
		return CreateRef<VulkanShader>(_code);
	}
}