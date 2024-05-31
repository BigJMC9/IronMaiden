#pragma once
#include "maidenpch.hpp"
#include "../Interfaces/H_Interface.h"
#include "../Platform/Platforms.hpp"
#include "../Events/H_EventSystem.h"


#define IMGUI_ENABLE_DOCKING
#define IMGUI_ENABLE_VIEWPORTS
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>
//#define USE_IMGUI_API
#include "ImGuizmo.h"

//Windows
#include <windows.h>
#include <commdlg.h>

//Changes made to ImGui
//All Backend ImGui_Impl_Vulkan formats now take R8G8B8A8_SRGB

#include <cmath>

namespace Madam {
	class DescriptorPool;
	class DescriptorSetLayout;
	class Entity;
	class Pipeline;
	namespace Rendering{
		class RenderStack;
		class RenderLayer;
	}
}
using namespace Madam::Events;

namespace Madam::UI {

static bool saveFileDialog(HWND hWnd, LPWSTR fileName, DWORD fileNameSize) {
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = L"All Files\0*.*\0";
	ofn.lpstrFile = fileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = fileNameSize;
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn)) {
		return true;
	}
	else {
		return false;
	}
}

namespace Madam::UI {
	struct PipelineInfo {
		Ref<Pipeline> pipeline;
		VkPipelineLayout layout;
	};

	class GUI : public EngineInterface {
	public:
		GUI();
		virtual ~GUI() override;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate() override;
		virtual void OnRender() override;

		void OnRenderPassEvent(NextRenderPassEvent* e);
		void OnResizeEvent(WindowResizeEvent* e);
		void OnSceneChangeEvent(SceneChangeEvent* e);

		void SetUpEvents();
		void Record(VkCommandBuffer commandBuffer);

		void Style(ImGuiIO& io);

		void EditorUI();

		//MenuBars
		void MenuBar();

		//Windows
		void DockingSpace();
		void Viewport();
		void Hierarchy();
		void Inspector();
		void Project();
		void Console();
		void RenderingSettings();
		

		static void DrawViewport(const ImDrawList* parentList, const ImDrawCmd* pcmd);
	private:

		enum WindowStates {
			RENDER_SETTINGS_WINDOW = 1 << 0
		};

		int windowStates = 0;
		void DrawVec3(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		void DrawViewportGizmoButtons();
		void CreateViewportPipeline();
		void DrawEntityNode(Entity entity);
		void DrawEntityComponents(Entity entity);
		void DrawPipelineSettings(const Ref<Rendering::RenderLayer> pipeline, int index);
		bool isRecreate = false;
		//void DrawViewport(const ImDrawList* parentList, const ImDrawCmd* cmd);

		ImGui_ImplVulkan_InitInfo* init_info;
		Ref<DescriptorPool> guiPool;
		VkDescriptorSet viewportSet;
		VkDescriptorSetLayout viewportSetLayout;
		VkSampler viewportSampler;
		std::unique_ptr<DescriptorSetLayout> viewportLayout;
		float uiTime = 0.0f;
		Ref<Entity> selectedEntity = nullptr;
		std::pair<Ref<Rendering::RenderLayer>, int> selectedPipeline = { nullptr, -1 };
		std::array<bool, 3> gizmoButtonStates;

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

		int ImGuizmoType = -1;
		
	};
}