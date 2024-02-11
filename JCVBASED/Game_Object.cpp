#include "H_Game_Object.hpp"

namespace Digestion {
	Entity::Entity(entt::entity _handle, Scene* _scene) : entityHandle{ _handle }, scene{ _scene } {

	}

	Entity::Entity(entt::entity _handle, Scene* _scene, UUID _id) : entityHandle{ _handle }, scene{ _scene }, id{ _id } {

	}
}