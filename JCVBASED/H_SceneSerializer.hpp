#pragma once

#include "H_Scene.hpp"
#include "H_JCVB_base.hpp"

namespace Digestion {
	
	class SceneSerializer {
	public:
		SceneSerializer(Scene* scene);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		//static void SerializeEntity(YAML::Emitter& out, Entity entity);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);
	private:
		Scene* m_Scene;
	};
}