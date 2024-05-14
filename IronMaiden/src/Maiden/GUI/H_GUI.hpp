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

//Changes made to ImGui
//All Backend ImGui_Impl_Vulkan formats now take R8G8B8A8_SRGB

#include <cmath>

namespace Madam {
	class DescriptorPool;
	class DescriptorSetLayout;
	class Entity;
	class Pipeline;
}

namespace Madam::UI {
	struct PipelineInfo {
		Ref<Pipeline> pipeline;
		VkPipelineLayout layout;
	};
	class GUI : public Layer {
	public:
		GUI();
		virtual ~GUI() override;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate() override;
		virtual void OnSceneLoad() override;
		void ReCreate();
		void Record(VkCommandBuffer commandBuffer);

		void Style(ImGuiIO& io);

		void EditorUI();
		void DockingSpace();
		void Viewport();
		void Hierarchy();
		void Inspector();
		void Project();
		void Console();

		static void DrawViewport(const ImDrawList* parentList, const ImDrawCmd* pcmd);
	private:

		void CreateViewportPipeline();
		void DrawEntityNode(Entity entity);
		void DrawEntityComponents(Entity entity);
		//void DrawViewport(const ImDrawList* parentList, const ImDrawCmd* cmd);

		ImGui_ImplVulkan_InitInfo* init_info;
		Ref<DescriptorPool> guiPool;
		VkDescriptorSet viewportSet;
		VkDescriptorSetLayout viewportSetLayout;
		VkSampler viewportSampler;
		std::unique_ptr<DescriptorSetLayout> viewportLayout;
		float uiTime = 0.0f;
		Ref<Entity> selectedEntity = nullptr;

		std::vector<ImFont*> fonts;
		ImGuiStyle style;

		//ImDrawCallback viewportCallback;
		//Ref<Pipeline> viewportPipeline;
		PipelineInfo viewportPipelineInfo;
		ImDrawCallback viewportCallback;

		template<typename T, typename U>
		auto constexpr constPow(T base, U exponent) {
			return std::exp(exponent * std::log(base));
		}


		constexpr float ColToFloat(float rgb) { return rgb / 255.0f; }
		constexpr ImVec4 RGBCon(float r, float g, float b) { return ImVec4(ColToFloat(r), ColToFloat(g), ColToFloat(b), 1.0f); }
		//constexpr ImVec4 RGBConWithGammaCorrection(float r, float g, float b) { return ImVec4(RGBGammaCorrection(r), RGBGammaCorrection(g), RGBGammaCorrection(b), 1.0f); }
		//constexpr float RGBGammaCorrection(float rgb) { return 1/(constPow((rgb/255), (1.0/2.2f))); }


		
	};
}