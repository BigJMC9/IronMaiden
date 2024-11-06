#include "maidenpch.hpp"
#include "H_VulkanMaterial.h"

namespace Madam
{
	VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader)
	{
		_shader = std::static_pointer_cast<VulkanShader>(shader);
	}

	VulkanMaterial::~VulkanMaterial()
	{

	}

	Ref<Shader> VulkanMaterial::GetShader()
	{
		return _shader;
	}
}