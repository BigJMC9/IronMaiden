#pragma once

#include "maidenpch.hpp"
#include "../Core/H_CoreUtils.hpp"
#include "../Rendering/Vulkan/H_VulkanDevice.hpp"

#include <filesystem>
#include <optional>

namespace Madam {
	class Scene;
	class SceneSerializer {
	public:
		SceneSerializer();

		static IrmResult Serialize(const std::filesystem::path& file_path, Ref<Scene> scene);
		static IrmResult Deserialize(const std::filesystem::path& file_path, Ref<Scene> scene);

		
	private:

	};
}