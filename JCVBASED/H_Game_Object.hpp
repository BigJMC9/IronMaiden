#pragma once

//#include "H_scene.hpp"

#include "H_JCVB_base.hpp"
#include "H_Scene.hpp"
#include "H_components.hpp"

//lib
#include "entt.hpp"

// std
#include <cassert>

namespace Digestion {

	//Wrapper class of entt::entity
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity _handle, Scene* _scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			//assert(!HasComponent<T>(), "GameObject already has component!");
			T& component = scene->registry.emplace<T>(entityHandle, std::forward<Args>(args)...);
			scene->OnComponentAdded<T>(*this, component);
			return component; //forwards args

		}

		template<typename T>
		T& GetComponent() {
			//assert(HasComponent<T>(), "GameObject does not have that component!");
			return scene->registry.get<T>(entityHandle);
		}

		template<typename T>
		bool HasComponent() {
			return scene->registry.orphan<T>(entityHandle);
		}

		template<typename T>
		void RemoveComponent() {
			//assert(HasComponent<T>(), "GameObject does not have that component!");
			scene->registry.remove<T>(entityHandle);
		}

		operator entt::entity() const { return entityHandle; }
		//UUID GetUUID() { return id; }

		bool operator==(const Entity& other) const
		{
			return entityHandle == other.entityHandle && scene == other.scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

		Transform transform;
	private:
		entt::entity entityHandle{ entt::null };
		Scene* scene = nullptr;
		//UUID id;
	};
}