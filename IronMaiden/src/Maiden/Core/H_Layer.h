#pragma once

#include "maidenpch.hpp"
#include "Main/Core.hpp"
#include "H_Logger.hpp"

namespace Madam {

	class MADAM_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() { MADAM_CORE_INFO("Layer Attach"); }
		virtual void OnDetach() {}
		virtual void OnUpdate() {}

		virtual void OnSceneLoad() {}

		inline const std::string& GetName() const { return surfaceName; }
	protected:
		std::string surfaceName;
	};

}



