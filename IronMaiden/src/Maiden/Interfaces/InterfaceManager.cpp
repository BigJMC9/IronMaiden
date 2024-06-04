#include "maidenpch.hpp"
#include "H_InterfaceManager.h"

namespace Madam {
	InterfaceManager::InterfaceManager() {
		InterfaceInsert = interfaces.begin();
	}

	InterfaceManager::~InterfaceManager() {
		for (EngineInterface* layer : interfaces) {
			delete layer;
		}
	}

	void InterfaceManager::PushInterface(EngineInterface* engineInterface) {
		InterfaceInsert = interfaces.emplace(InterfaceInsert, engineInterface);
	}

	void InterfaceManager::PushOverlay(EngineInterface* engineInterface) {
		interfaces.emplace_back(engineInterface);
	}

	void InterfaceManager::PopInterface(EngineInterface* engineInterface) {
		auto it = std::find(interfaces.begin(), interfaces.end(), engineInterface);
		if (it != interfaces.end()) {
			interfaces.erase(it);
			InterfaceInsert--;
		}
	}

	void InterfaceManager::PopOverlay(EngineInterface* engineInterface) {
		auto it = std::find(interfaces.begin(), interfaces.end(), engineInterface);
		if (it != interfaces.end()) {
			interfaces.erase(it);
		}
	}
}