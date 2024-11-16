#pragma once

#include "maidenpch.hpp"
#include "H_RIResource.h"
#include "H_Buffer.hpp"
#include "Vulkan/H_VulkanDevice.hpp"

#include <filesystem>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define INCLUDE_GLM
#define INCLUDE_UTILS
#include "../Utils/H_Utils.h"

namespace Madam {

	enum class MeshPrimatives
	{
		Quad = 0,
		Cube = 1,
		Sphere = 2
	};

	static std::filesystem::path meshPrimativeFilepaths[3] =
	{
		"resources\\models\\plane.obj",
		"resources\\models\\cube.obj",
		"resources\\models\\sphere.obj"
	};

	class StaticMesh : public RenderResource 
	{
	public:
		enum MeshFlags
		{
			HasIndexBuffer = 1 << 0,
		};

		virtual ~StaticMesh() {}

		static Ref<StaticMesh> Create(const std::filesystem::path& filepath);
		static Ref<StaticMesh> Create(const MeshPrimatives primative);

		virtual void bind(void* commandBuffer) = 0;
		virtual void draw(void* commandBuffer) = 0;

		std::filesystem::path GetFilepath() const { return _filepath; }

		static AssetType GetStaticType() { return AssetType::MESH; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

	protected:

		MeshFlags meshFlags;
		std::filesystem::path _filepath;
	};
}