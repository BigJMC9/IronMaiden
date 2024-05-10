#pragma once

#include "maidenpch.hpp"
#include "../Core/H_Utils.hpp"
#include "../Core/H_Device.hpp"

namespace Madam {
	class Scene;
	class SceneSerializer {
	public:
		SceneSerializer(Ref<Scene> scene, Device& _device);

		void Serialize(const std::string& rawfilePath);
		void SerializeRuntime(const std::string& filePath);

		//static void SerializeEntity(YAML::Emitter& out, Entity entity);

		bool Deserialize(const std::string& rawfilePath);
		bool DeserializeRuntime(const std::string& filePath);
	private:
		Ref<Scene> m_Scene;
		Device& device;
	};
}