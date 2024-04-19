#pragma once
#include "maidenpch.hpp"
#include "../Core/H_Layer.h"
#define IMGUI_IMPL_API
#define IMGUI_API
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace Madam::UI {
	class GUI : public Layer {
	public:
		GUI();
		virtual ~GUI() override;
		virtual void OnAttach() override;
		//virtual void OnDetach() override;
		virtual void OnUpdate() override;
		virtual void OnSceneLoad() override;
	private:
		ImGui_ImplVulkan_InitInfo* init_info;
	};
}