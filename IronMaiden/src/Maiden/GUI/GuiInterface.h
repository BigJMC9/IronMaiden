#pragma once
#include "../Interfaces/H_Interface.h"

namespace Madam
{
	class GuiInterface : public EngineInterface
	{
	public:
		virtual void Begin() = 0;
		virtual void End() = 0;

		static GuiInterface* Create();
	};
}