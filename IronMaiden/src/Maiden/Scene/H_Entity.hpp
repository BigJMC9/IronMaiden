#pragma once
#ifndef H_ENTITY
#define H_ENTITY

#include "maidenpch.hpp"
#include "../Core/H_CoreUtils.hpp"
#include "H_Scene.hpp"

#define MADAM_ENTT_HEADER_FLAG

// std
#include <cassert>

namespace Madam {

	//Wrapper class of entt::entity
	class MADAM_API Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity _handle, Scene* _scene);
		//Entity(entt::entity _handle, Scene* _scene, UUID _id);
		Entity(const Entity& other) = default;

		~Entity() = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			MADAM_CORE_ASSERT(!HasComponent<T>(), "GameObject already has component!");
			//assert(!HasComponent<T>(), "GameObject already has component!");
			return scene->registry.emplace<T>(entityHandle, std::forward<Args>(args)...);
			//scene->OnComponentAdded<T>(*this, component);
			//return component; //forwards args
		}

		template<typename T>
		T& GetComponent() {
			MADAM_CORE_ASSERT(HasComponent<T>(), "GameObject does not have that component!");
			//assert(HasComponent<T>(), "GameObject does not have that component!");
			return scene->registry.get<T>(entityHandle);
		}

		template<typename T>
		bool HasComponent() {
			return scene->registry.any_of<T>(entityHandle);
		}

		template<typename T>
		void RemoveComponent() {
			MADAM_CORE_ASSERT(HasComponent<T>(), "GameObject does not have that component!");
			//assert(HasComponent<T>(), "GameObject does not have that component!");
			//scene->OnComponentRemoved<T>(*this, component);
			scene->registry.remove<T>(entityHandle);
		}

		operator bool() const { return entityHandle != entt::null; }
		operator entt::entity() const { return entityHandle; }
		operator uint32_t() const { return (uint32_t)entityHandle; }

		entt::entity GetHandle() {
			return entityHandle;
		}

		entt::entity& GetHandleAsRef() {
			return entityHandle;
		}

		std::string GetHandleMemoryLocation() {
			std::stringstream ss;
			ss << static_cast<void*>(&entityHandle);
			return ss.str();
		}

		bool isNull()
		{
			if (entityHandle == entt::null)
			{

			}
		}

		bool operator==(const Entity& other) const
		{
			return entityHandle == other.entityHandle && scene == other.scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

		bool operator==(const null_t& other) const
		{
			if (entityHandle == entt::null)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		bool operator!=(const null_t& other) const
		{
			if (entityHandle != entt::null)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		//Transform transform;
	private:
		entt::entity entityHandle{ entt::null };
		Scene* scene = nullptr;
	};
}
#endif