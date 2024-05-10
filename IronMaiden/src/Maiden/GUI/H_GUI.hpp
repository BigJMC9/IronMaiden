#pragma once
#include "maidenpch.hpp"
#include "../Core/H_Layer.h"
#define IMGUI_ENABLE_DOCKING
#define IMGUI_ENABLE_VIEWPORTS
#define IMGUI_IMPL_API
#define IMGUI_API
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace Madam {
	class DescriptorPool;
	class DescriptorSetLayout;
}
namespace Madam::UI {
	class GUI : public Layer {
	public:
		GUI();
		virtual ~GUI() override;
		virtual void OnAttach() override;
		//virtual void OnDetach() override;
		virtual void OnUpdate() override;
		virtual void OnSceneLoad() override;

		void Record(VkCommandBuffer commandBuffer);
		void EditorUI();
		void DockingSpace();
		void Viewport();
	private:
		ImGui_ImplVulkan_InitInfo* init_info;
		Ref<DescriptorPool> guiPool;
		VkDescriptorSet viewportSet;
		VkSampler viewportSampler;
		std::unique_ptr<DescriptorSetLayout> viewportLayout;
		float uiTime = 0.0f;
	};
}