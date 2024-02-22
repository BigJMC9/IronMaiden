#pragma once

#include "H_JCVB_base.hpp"
#include "H_Scene.hpp"
#include "H_components.hpp"

// std
#include <cassert>

namespace Madam {

	//Wrapper class of entt::entity
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity _handle, Scene* _scene);
		Entity(entt::entity _handle, Scene* _scene, UUID _id);
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
			return scene->registry.any_of<T>(entityHandle);
		}

		template<typename T>
		void RemoveComponent() {
			//assert(HasComponent<T>(), "GameObject does not have that component!");
			scene->registry.remove<T>(entityHandle);
		}

		operator bool() const { return entityHandle != entt::null; }
		operator entt::entity() const { return entityHandle; }
		operator uint32_t() const { return (uint32_t)entityHandle; }

		UUID GetUUID() { return id; }

		std::string SetName(std::string _name) {
			name = _name;
			return name; 
		}

		std::string GetName() {
			return name;
		}

		entt::entity GetHandle() {
			return entityHandle;
		}

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
		UUID id;
		std::string name = "NULL";
	};
}