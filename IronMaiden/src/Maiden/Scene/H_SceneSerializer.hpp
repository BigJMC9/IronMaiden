#pragma once

#include "maidenpch.hpp"

#define INCLUDE_UTILS
#include "../Utils/H_Utils.h"

#include "../Rendering/Vulkan/H_VulkanDevice.hpp"

#include <filesystem>

namespace Madam {
	class Scene;
	class SceneSerializer {
	public:
		SceneSerializer(Ref<Scene> scene, Device& _device);

		void Serialize(const std::filesystem::path& filePath);
		void SerializeRuntime(const std::string& filePath);

		//static void SerializeEntity(YAML::Emitter& out, Entity entity);

		bool Deserialize(const std::filesystem::path& rawfilePath);
		bool DeserializeRuntime(const std::string& filePath);
	private:
		Ref<Scene> m_Scene;
		Device& device;
	};
}