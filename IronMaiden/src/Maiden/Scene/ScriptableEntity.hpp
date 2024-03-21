#pragma once

#include "maidenpch.hpp"
#include "H_Entity.hpp"

namespace Madam {
	class ScriptableEntity {
	public:
		template<typename T>
		T& GetComponent() {
			return entity.GetComponent<T>();
		}
	private:
		Entity entity;
		friend class Scene;
	};
}