#pragma once

#include "maidenpch.hpp"
#include "Main/Core.hpp"
#include "../Events/Event.hpp"
#include "H_Logger.hpp"

namespace Madam {

	class MADAM_API Surface
	{
	public:
		Surface(const std::string& name = "Surface");
		virtual ~Surface();

		virtual void OnAttach() { MADAM_CORE_INFO("Surface Attach"); }
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(Event& event)
		{

		}

		virtual void OnSceneLoad() {}

		inline const std::string& GetName() const { return surfaceName; }
	private:
		std::string surfaceName;
	};

}



