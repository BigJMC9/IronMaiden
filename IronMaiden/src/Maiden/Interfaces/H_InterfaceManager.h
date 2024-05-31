#pragma once

#include "maidenpch.hpp"
#include "../Core/Main/Core.hpp"
#include "H_Interface.h"

namespace Madam
{
	class MADAM_API InterfaceManager
	{
	public:
		InterfaceManager();
		~InterfaceManager();

		void PushInterface(EngineInterface* engineInterface);
		void PopInterface(EngineInterface* engineInterface);
		void PushOverlay(EngineInterface* engineInterface);
		void PopOverlay(EngineInterface* engineInterface);

		std::vector<EngineInterface*>::iterator begin() { return interfaces.begin(); }
		std::vector<EngineInterface*>::iterator end() { return interfaces.end(); }
	private:
		std::vector<EngineInterface*> interfaces;
		std::vector<EngineInterface*>::iterator InterfaceInsert;
	};
}

