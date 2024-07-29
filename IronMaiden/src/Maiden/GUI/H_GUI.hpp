#pragma once
#include "maidenpch.hpp"
#include "../Interfaces/H_Interface.h"
#include "../Platform/Platforms.hpp"
#include "../Events/H_EventSystem.h"
#include "../Rendering/Vulkan/H_DescriptorManager.h"
#include "../Rendering/Vulkan/H_VulkanTexture.h"


#define IMGUI_ENABLE_DOCKING
#define IMGUI_ENABLE_VIEWPORTS
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include "ImGuizmo.h"

//Changes made to ImGui
//All Backend ImGui_Impl_Vulkan formats now take R8G8B8A8_SRGB

#include <cmath>

namespace Madam {
	class DescriptorPool;
	class DescriptorSetLayout;
	class Entity;
	class Pipeline;
	class Asset;
	namespace Rendering{
		class RenderStack;
		class RenderLayer;
	}
}
using namespace Madam::Events;

namespace Madam::UI {
	
	struct PipelineInfo {
		Ref<Pipeline> pipeline;
		VkPipelineLayout layout;
	};

	struct IconInfo
	{
		Ref<Texture> texture = nullptr;
		VkDescriptorSet descriptorSet = nullptr;
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

		void SetupEvents();
		void SetupIcons();
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
		//Ref<Asset>& AssetReference(const AssetType filter);
	private:

		enum WindowStates {
			RENDER_SETTINGS_WINDOW = 1 << 0
		};

		int windowStates = 0;
		void DrawVec3(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		void DrawViewportOverlays();
		void DrawViewportGizmoButtons();
		void DrawEntityNode(Entity entity);
		void DrawEntityComponents(Entity entity);
		void DrawAssetInfo(Ref<Asset>& asset);
		void DrawPipelineSettings(const Ref<Rendering::RenderLayer> pipeline, int index);

		std::string TruncateTextToFit(const std::string& text, float maxWidth);

		void CreateViewportPipeline();

		ImGui_ImplVulkan_InitInfo* init_info;
		Ref<DescriptorPool> guiPool;
		
		float uiTime = 0.0f;
		int ImGuizmoType = -1;

		std::filesystem::path curDir = std::filesystem::u8path("Assets");
		Ref<Asset> selectedAsset = nullptr;
		bool isSRGB = false;

		Ref<Entity> selectedEntity = nullptr;
		Ref<Entity> pendingEntityDeletion;
		std::pair<Ref<Rendering::RenderLayer>, int> selectedPipeline = { nullptr, -1 };
		std::array<bool, 3> gizmoButtonStates;

		std::vector<ImFont*> fonts;
		ImGuiStyle style;

		PipelineInfo viewportPipelineInfo;
		ImDrawCallback viewportCallback;

		//Descriptors
		VkDescriptorSet viewportSet;
		VkSampler viewportSampler;

		std::unordered_map<std::string, IconInfo> icons;
		std::unordered_map<UUID, IconInfo> loadedIconTextures;

#define ICON_SIZE 5
		std::filesystem::path iconFilepaths[ICON_SIZE] =
		{
			"resources\\icons\\PlayButton.png",
			"resources\\icons\\PauseButton.png",
			"resources\\icons\\StopButton.png",
			"resources\\icons\\Folder.png",
			"resources\\icons\\File.png"
		};

		template<typename T, typename U>
		auto constexpr constPow(T base, U exponent) {
			return std::exp(exponent * std::log(base));
		}

		constexpr float ColToFloat(float rgb) { return rgb / 255.0f; }
		constexpr ImVec4 RGBCon(float r, float g, float b) { return ImVec4(ColToFloat(r), ColToFloat(g), ColToFloat(b), 1.0f); }
		bool tempDebug = false;
	};
}