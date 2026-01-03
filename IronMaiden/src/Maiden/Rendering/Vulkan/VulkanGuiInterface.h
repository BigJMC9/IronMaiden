#pragma once
#include "../../GUI/GuiInterface.h"

namespace Madam
{
	class VulkanGuiInterface : public GuiInterface
	{
		VulkanGuiInterface();
		VulkanGuiInterface(const std::string& name);
		virtual ~VulkanGuiInterface();

		virtual void Begin() override;
		virtual void End() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		//virtual void OnImGuiRender() override;
	};

}