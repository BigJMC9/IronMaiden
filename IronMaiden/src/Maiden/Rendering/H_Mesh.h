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

	enum class MeshPrimatives : uint8_t
	{
		None = static_cast<uint8_t>(0),
		Quad = static_cast<uint8_t>(1),
		Cube = static_cast<uint8_t>(2),
		Sphere = static_cast<uint8_t>(3)
	};

	static std::map<std::string, uint8_t> meshPrimativesMap =
	{
		{"None", static_cast<uint8_t>(0)},
		{"Quad", static_cast<uint8_t>(1)},
		{"Cube", static_cast<uint8_t>(2)},
		{"Sphere", static_cast<uint8_t>(3)}
	};

	static std::filesystem::path meshPrimativeFilepaths[4] =
	{
		"none",
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
		bool IsPrimative() const { return meshPrimative != MeshPrimatives::None; }
		MeshPrimatives GetPrimative() const { return meshPrimative; }
		std::string GetPrimativeAsString() const 
		{
			switch (meshPrimative)
			{
			case Madam::MeshPrimatives::None:
				return "None";
				break;
			case Madam::MeshPrimatives::Quad:
				return "Quad";
				break;
			case Madam::MeshPrimatives::Cube:
				return "Cube";
				break;
			case Madam::MeshPrimatives::Sphere:
				return "Sphere";
				break;
			default:
				break;
			}
		}

		static AssetType GetStaticType() { return AssetType::MESH; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

	protected:

		bool isLoaded = false;
		MeshFlags meshFlags;
		std::filesystem::path _filepath;
		MeshPrimatives meshPrimative = MeshPrimatives::None;
	};
}