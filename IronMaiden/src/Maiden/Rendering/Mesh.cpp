#include "maidenpch.hpp"
#include "../Core/H_Application.hpp"
#include "H_Mesh.h"
#include "../Project/H_Project.h"
#include "Vulkan/H_VulkanMesh.h"
#include "../Core/H_Utils.hpp"


namespace Madam {
	
	Ref<StaticMesh> StaticMesh::Create(const std::filesystem::path& filepath)
	{
		return CreateRef<VulkanStaticMesh>(filepath);
	}
}