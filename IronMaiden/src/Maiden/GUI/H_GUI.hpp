#pragma once
#include "maidenpch.hpp"
#include "../Interfaces/H_Interface.h"
#include "../Events/H_EventSystem.h"
#include "../Rendering/Vulkan/H_DescriptorManager.h"
#include "../Rendering/Vulkan/H_VulkanTexture.h"

#include <filesystem>

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

#define ASSET_DIR std::filesystem::u8path("Assets")

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
		void OnMouseMoveRawEvent(MouseMoveRawEvent* e);
		void OnMouseScrollEvent(MouseScrollEvent* e);

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

		ImGui_ImplVulkan_InitInfo* init_info = nullptr;
		Ref<DescriptorPool> guiPool = nullptr;
		
		float uiTime = 0.0f;
		int ImGuizmoType = -1;

		std::filesystem::path curDir = ASSET_DIR;
		std::filesystem::path sceneDir = "";

		Ref<Asset> selectedAsset = nullptr;
		bool isSRGB = false;

		bool isMovingViewportCamera = false;

		Ref<Entity> selectedEntity = nullptr;
		Ref<Entity> pendingEntityDeletion;
		bool hasEntityRelationshipChanged = false;
		UUID newParentEntityUUID;
		UUID newChildEntityUUID;

		std::pair<Ref<Rendering::RenderLayer>, int> selectedPipeline = { nullptr, -1 };
		std::array<bool, 3> gizmoButtonStates = {false, false, false};

		std::vector<ImFont*> fonts;
		ImGuiStyle style;

		PipelineInfo viewportPipelineInfo;
		ImDrawCallback viewportCallback = nullptr;

		//Descriptors
		VkDescriptorSet viewportSet = nullptr;
		VkSampler viewportSampler = nullptr;

		std::unordered_map<std::string, IconInfo> icons;
		std::unordered_map<UUID, IconInfo> loadedIconTextures;
		std::filesystem::path popupContextSelectedItem;
		bool isPopupContextOpen = false;

#define ICON_SIZE 6
		std::filesystem::path iconFilepaths[ICON_SIZE] =
		{
			"resources\\icons\\PlayButton.png",
			"resources\\icons\\PauseButton.png",
			"resources\\icons\\StopButton.png",
			"resources\\icons\\Folder.png",
			"resources\\icons\\File.png",
			"resources\\icons\\Scene.png"
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