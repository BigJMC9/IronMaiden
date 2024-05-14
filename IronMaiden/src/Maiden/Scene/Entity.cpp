#include "maidenpch.hpp"
#include "H_Entity.hpp"

namespace Madam {
	Entity::Entity(entt::entity _handle, Scene* _scene) : entityHandle{ _handle }, scene{ _scene } {

	}
}