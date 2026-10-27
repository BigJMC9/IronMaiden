#include "maidenpch.hpp"
#include "../Core/H_Application.hpp"
#include "H_Mesh.h"
#include "../Project/H_Project.h"
#include "Vulkan/H_VulkanMesh.h"
#include "../Core/H_CoreUtils.hpp"
#include <unordered_map>


namespace Madam {
	namespace {
		std::unordered_map<std::string, std::weak_ptr<StaticMesh>> residentMeshes;
	}
	
	Ref<StaticMesh> StaticMesh::Create(const std::filesystem::path& filepath)
	{
		const auto normalized = std::filesystem::absolute(filepath).lexically_normal();
		const auto key = std::string("file:") + normalized.u8string();
		if (auto cached = residentMeshes[key].lock()) return cached;
		auto mesh = CreateRef<VulkanStaticMesh>(normalized);
		residentMeshes[key] = mesh;
		return mesh;
	}

	Ref<StaticMesh> StaticMesh::Create(const MeshPrimatives primative)
	{
		const auto key = std::string("primitive:") + std::to_string(static_cast<int>(primative));
		if (auto cached = residentMeshes[key].lock()) return cached;
		auto mesh = CreateRef<VulkanStaticMesh>(primative);
		residentMeshes[key] = mesh;
		return mesh;
	}
}
