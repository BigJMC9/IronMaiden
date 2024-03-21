#pragma once

#include "maidenpch.hpp"
#include "Main/Core.hpp"
#include "../Events/Event.hpp"

namespace Madam {

	class MADAM_API Surface
	{
	public:
		Surface(const std::string& name = "Surface");
		virtual ~Surface();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(Event& event)
		{

		}

		inline const std::string& GetName() const { return surfaceName; }
	private:
		std::string surfaceName;
	};

}



