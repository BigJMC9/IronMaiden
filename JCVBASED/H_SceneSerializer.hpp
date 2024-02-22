#pragma once

#include "H_Scene.hpp"
#include "H_JCVB_base.hpp"

namespace Digestion {
	
	class SceneSerializer {
	public:
		SceneSerializer(std::unique_ptr<Scene>& scene, Device& _device);

		void Serialize(const std::string& filePath);
		void SerializeRuntime(const std::string& filePath);

		//static void SerializeEntity(YAML::Emitter& out, Entity entity);

		bool Deserialize(const std::string& filePath);
		bool DeserializeRuntime(const std::string& filePath);
	private:
		std::unique_ptr<Scene>& m_Scene;
		Device& device;
	};
}