#pragma once

#include "maidenpch.hpp"
#include "H_Scene.hpp"
#include "../Core/Base.hpp"

namespace Madam {
	
	class SceneSerializer {
	public:
		SceneSerializer(std::unique_ptr<Scene>& scene, Device& _device);

		void Serialize(const std::string& rawfilePath);
		void SerializeRuntime(const std::string& filePath);

		//static void SerializeEntity(YAML::Emitter& out, Entity entity);

		bool Deserialize(const std::string& rawfilePath);
		bool DeserializeRuntime(const std::string& filePath);
	private:
		std::unique_ptr<Scene>& m_Scene;
		Device& device;
	};
}