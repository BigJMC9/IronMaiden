#pragma once

#include "maidenpch.hpp"
#include "H_Entity.hpp"

namespace Madam {
	class ScriptableEntity {
	public:
		virtual ~ScriptableEntity() {}

		template<typename T>
		T& GetComponent() {
			return entity.GetComponent<T>();
		}

	protected:
		//virtual void OnBind() {}
	private:
		Entity entity;
		friend class Scene;
	};
}