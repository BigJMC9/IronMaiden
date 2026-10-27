#pragma once
#include "maidenpch.hpp"
#include "../Interfaces/H_Interface.h"
#include "../Events/H_EventSystem.h"
#include "../Rendering/Vulkan/H_DescriptorManager.h"
#include "../Rendering/Vulkan/H_VulkanTexture.h"

#include <filesystem>
#include <functional>

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
		VkPipelineLayout layout = VK_NULL_HANDLE;
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
		void Game();
		void Hierarchy();
		void Inspector();
		void Project();
		void Console();
		void RenderingSettings();
		void LightingWindow();
		void DebugWindow();

		static void DrawViewport(const ImDrawList* parentList, const ImDrawCmd* pcmd);
		//Ref<Asset>& AssetReference(const AssetType filter);
	private:

		enum WindowStates {
			RENDER_SETTINGS_WINDOW = BIT(0),
			DEBUG_INFO_WINDOW = BIT(1),
			LIGHTING_WINDOW = BIT(2)
		};

		int windowStates = 0;
		void DrawVec3(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		void DrawViewportOverlays();
		void DrawViewportGizmoButtons();
		void DrawPerformanceOverlay(ImVec2 topLeft, ImVec2 size, const char* title);
		void DrawEntityNode(Entity entity);
		void DrawEntityComponents(Entity entity);
		void DrawAssetInfo(Ref<Asset>& asset);
		void DrawAssetReference(const char* label, UUID current, AssetType type,
			const std::function<void(UUID)>& onSelect);
		void DrawMaterialThumbnail(UUID handle, const char* buttonId, ImVec2 size);
		void DrawPipelineSettings(const Ref<Rendering::RenderLayer> pipeline, int index);

		std::string TruncateTextToFit(const std::string& text, float maxWidth);

		void CreateViewportPipeline();
		void RefreshRenderTargetDescriptors();

		ImGui_ImplVulkan_InitInfo initInfo{};
		ImGui_ImplVulkan_InitInfo* init_info = &initInfo;
		Ref<DescriptorPool> guiPool = nullptr;
		
		float uiTime = 0.0f;
		int ImGuizmoType = -1;

		std::filesystem::path curDir = ASSET_DIR;
		std::filesystem::path sceneDir = "";
		std::filesystem::path browserDirectory;
		std::filesystem::file_time_type browserDirectoryStamp{};
		std::vector<std::filesystem::directory_entry> browserFolders;
		std::vector<std::filesystem::directory_entry> browserFiles;
		struct BrowserTile
		{
			std::string buttonId;
			std::string label;
			float labelOffset = 0.0f;
		};
		std::vector<BrowserTile> browserFolderTiles;
		std::vector<BrowserTile> browserFileTiles;
		std::vector<UUID> browserHandles;
		std::vector<AssetType> browserTypes;

		Ref<Asset> selectedAsset = nullptr;
		bool isSRGB = false;

		bool isMovingViewportCamera = false;
		bool showViewportStats = true;
		bool showGameStats = true;

		Ref<Entity> selectedEntity = nullptr;
		Ref<Entity> pendingEntityDeletion;
		bool hasEntityRelationshipChanged = false;
		UUID newParentEntityUUID;
		UUID newChildEntityUUID;

		std::pair<Ref<Rendering::RenderLayer>, int> selectedPipeline = { nullptr, -1 };
		std::array<bool, 3> gizmoButtonStates = {false, false, false};

		std::vector<ImFont*> fonts;
		ImGuiStyle style;
		ImGuiID dockspaceId = 0;
		ImVec2 viewportImageTopLeft{ 0, 0 };
		ImVec2 viewportImageSize{ 0, 0 };

		PipelineInfo viewportPipelineInfo;
		ImDrawCallback viewportCallback = nullptr;

		//Descriptors
		VkDescriptorSet viewportSet = nullptr;
		VkDescriptorSet gameSet = nullptr;
		std::vector<VkDescriptorSet> viewportSets;
		std::vector<VkDescriptorSet> gameSets;
		VkSampler viewportSampler = nullptr;

		std::unordered_map<std::string, IconInfo> icons;
		std::unordered_map<UUID, IconInfo> loadedIconTextures;
		char assetPickerSearch[256] = {};
		std::string shaderCompileFeedback;
		UUID shaderCompileAsset = UUID("");
		std::filesystem::path popupContextSelectedItem;
		bool isPopupContextOpen = false;
		std::filesystem::path renameTarget;
		std::array<char, 256> renameName{};
		std::string renameError;
		bool openRenamePopup = false;

#define ICON_SIZE 9
		std::filesystem::path iconFilepaths[ICON_SIZE] =
		{
			"resources\\icons\\godot-raster\\PlayButton.png",
			"resources\\icons\\godot-raster\\PauseButton.png",
			"resources\\icons\\godot-raster\\StopButton.png",
			"resources\\icons\\godot-raster\\Folder.png",
			"resources\\icons\\godot-raster\\File.png",
			"resources\\icons\\godot-raster\\Scene.png",
			"resources\\icons\\godot-raster\\Script.png",
			"resources\\icons\\godot-raster\\Shader.png",
			"resources\\icons\\godot-raster\\Camera.png"
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
