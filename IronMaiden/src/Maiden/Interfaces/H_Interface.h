#pragma once

#include "maidenpch.hpp"
#include "../Core/Main/Core.hpp"
#include "../Core/H_Logger.hpp"
//#include "../Events/H_EventSystem.h"

//using namespace Madam::Events;
namespace Madam {
	class MADAM_API EngineInterface
	{
	public:
		EngineInterface(const std::string& name = "EngineInterface");
		virtual ~EngineInterface();

		virtual void OnAttach() { MADAM_CORE_INFO("EngineInterface Attach"); }
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnRender() {}

		inline const std::string& GetName() const { return interfaceName; }
	protected:
		std::string interfaceName;
	};
}