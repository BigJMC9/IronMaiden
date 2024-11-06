#include "maidenpch.hpp"
#include "H_Material.h"
#include "Vulkan/H_VulkanMaterial.h"

namespace Madam
{
	Ref<Material> Material::Create(const Ref<Shader>& shader)
	{
		return CreateRef<VulkanMaterial>(shader);
	}
}