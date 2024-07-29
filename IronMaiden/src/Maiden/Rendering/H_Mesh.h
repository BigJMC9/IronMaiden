#pragma once

#include "maidenpch.hpp"
#include "H_RIResource.h"
#include "H_Buffer.hpp"
#include "Vulkan/H_VulkanDevice.hpp"

#include <filesystem>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Madam {
	class StaticMesh : public RenderResource {
	public:
		enum MeshFlags
		{
			HasIndexBuffer = 1 << 0,
		};

		virtual ~StaticMesh() {}

		static Ref<StaticMesh> Create(const std::filesystem::path& filepath);

		virtual void bind(void* commandBuffer) = 0;
		virtual void draw(void* commandBuffer) = 0;

		std::filesystem::path GetFilepath() const {
			return _filepath;
		}

		static AssetType GetStaticType() { return AssetType::MESH; }
		virtual AssetType GetAssetType() const { return AssetType::MESH; }

	protected:

		MeshFlags meshFlags;
		std::filesystem::path _filepath;
	};
}