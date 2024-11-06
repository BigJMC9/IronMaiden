#pragma once
#include "../Core/H_Logger.hpp"
#include "H_Events.hpp"
#include <typeindex>
#include <queue>

namespace Madam {
	class Application;
}

namespace Madam::Events {

	class EventSystem;

	class ListenerHandler {
	public:
		void Execute(Event* _event) {
			call(_event);
		}

	private:

		virtual void call(Event* _event) = 0;
	protected:
		virtual ~ListenerHandler() {};
		friend class EventSystem;
	};

	template<class T, class EventType>
	class MemberListenerHandler : public ListenerHandler {
	public:
		typedef void (T::* MemberFunction)(EventType*);
		MemberListenerHandler(T* instance, MemberFunction function) : _instance(instance), _function(function) {}

		void call(Event* _event) override {
			(_instance->*_function)(static_cast<EventType*>(_event));
		}

	private:
		T* _instance;
		MemberFunction _function;
		friend class EventSystem;
	};

	struct EventHandler {
		template<class EventType>
		EventHandler(EventType* _eventType) {
			eventType = typeid(EventType);
		}
		std::type_index eventType = typeid(Event);
		Event* _event;
	};

	typedef std::vector<ListenerHandler*> ListenerHandlers;
	class MADAM_API EventSystem {
	public:

		static EventSystem& Get() {
			MADAM_CORE_ASSERT(instanceFlag, "EventSystem instance not created");
			if (instance == nullptr) {
				MADAM_CORE_ERROR("EventSystem instance is null pointer");
			}
			return *instance;
		}

		template<class EventType>
		void PushEvent(EventType* _event, bool isImmediate) {
			if (isImmediate) {
				Dispatch(_event);
			}
			else {
				_eventQueue.push(EventHandler(_event));
			}
		}
		template<class EventType>
		void PushEvent(EventType* _event) {
			_eventQueue.push(EventHandler(_event));
		}

		template<class T, class EventType>
		void AddListener(T* instance, void (T::* function)(EventType*)) {
			ListenerHandlers* listeners = _listeners[typeid(EventType)];
			if (listeners == nullptr) {
				listeners = new ListenerHandlers();
				_listeners[typeid(EventType)] = listeners;
			}
			listeners->push_back(new MemberListenerHandler<T, EventType>(instance, function));
		}

		template<class T, class EventType>
		void RemoveListener(T* instance, void (T::* function)(EventType*)) {
			ListenerHandlers* listeners = _listeners[typeid(EventType)];
			for (ListenerHandler* listener : *listeners) {
				MemberListenerHandler<T, EventType>* memberListener = static_cast<MemberListenerHandler<T, EventType>*>(listener);
				if (memberListener != nullptr) {
					if (memberListener->_instance == instance && memberListener->_function == function) {
						listeners->erase(std::remove(listeners->begin(), listeners->end(), listener), listeners->end());
						delete listener;
						break;
					}
				}
			}
		}

	private:
		EventSystem() 
		{ 
			instance = this;
			instanceFlag = true;
		}
		~EventSystem() {}

		template<class EventType>
		void Dispatch(EventType* _event) {
			ListenerHandlers* listeners = _listeners[typeid(EventType)];
			if (listeners != nullptr) {
				for (ListenerHandler* listener : *listeners) {
					if (listener != nullptr) {
						listener->Execute(_event);
					}
				}
			}
		}

		void Dispatch(EventHandler _eventHandler) {
			ListenerHandlers* listeners = _listeners[_eventHandler.eventType];
			if (listeners != nullptr) {
				for (ListenerHandler* listener : *listeners) {
					if (listener != nullptr) {
						listener->Execute(_eventHandler._event);
					}
				}
			}
		}

		void DispatchBatch() {
			while (!_eventQueue.empty()) {
				Dispatch(_eventQueue.front());
				_eventQueue.pop();
			}
		}

		std::map<std::type_index, ListenerHandlers*> _listeners;
		std::queue<EventHandler> _eventQueue;

		static EventSystem* instance;
		static bool instanceFlag;

		//std::set<UUID> _eventIds;
		friend class Application;
	};
}

