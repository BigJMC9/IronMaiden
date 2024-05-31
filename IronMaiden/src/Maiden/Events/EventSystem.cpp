#include "maidenpch.hpp"
#include "H_EventSystem.h"

namespace Madam::Events {
	EventSystem* EventSystem::instance = nullptr;
	bool EventSystem::instanceFlag = false;
}