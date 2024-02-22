#pragma once

#include "H_JCVB_base.hpp"

//std
#include <memory>
#include <unordered_map>
#include <iostream>
namespace Digestion {

	class JcvbComponent {
	public:
		
		JcvbComponent() = default;
		virtual ~JcvbComponent() = default;
		JcvbComponent(const JcvbComponent&) = delete;
		JcvbComponent& operator=(const JcvbComponent&) = delete;

		virtual void assignGameObject(id_t jcvbId);

		virtual void start() {}
		virtual void update() {}
		virtual void render() {}

		id_t gameObjectId;
		GameObject* gameObject = nullptr; // Reference to the assigned game object
	};
}