#include "maidenpch.hpp"
#include "H_GUI.hpp"
#include "../Events/H_Input.hpp"
#include "../Scene/H_Entity.hpp"
#include "../Core/H_Application.hpp"
#include "../Scene/H_SceneSerializer.hpp"
#include "../Rendering/H_Pipeline.hpp"
#include "../Project/H_Project.h"
#include "../Asset/H_AssetSystem.h"
#include "../Asset/H_MaterialAsset.h"
#include "../Rendering/H_Mesh.h"
#include "../Platform/Platforms.hpp"

#define INCLUDE_GLM_UTILS
#include "../Utils/H_Logger_Utils.h"

#include <cwchar>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <tuple>

using namespace Madam::Platform;

static std::vector<uint32_t> vertCode = {
	0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
	0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
	0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
	0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
	0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
	0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
	0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
	0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
	0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
	0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
	0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
	0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
	0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
	0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
	0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
	0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
	0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
	0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
	0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
	0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
	0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
	0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
	0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
	0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
	0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
	0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
	0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
	0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
	0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
	0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
	0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
	0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
	0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
	0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
	0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
	0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
	0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
	0x0000002d,0x0000002c,0x000100fd,0x00010038
};

static std::vector<uint32_t> fragCode = {
	0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
	0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
	0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
	0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
	0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
	0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
	0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
	0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
	0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
	0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
	0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
	0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
	0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
	0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
	0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
	0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
	0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
	0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
	0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
	0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
	0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
	0x00010038
};

// Reusable buffers used for rendering 1 current in-flight frame, for ImGui_ImplVulkan_RenderDrawData()
// [Please zero-clear before use!]
struct ImGui_ImplVulkan_FrameRenderBuffers
{
	VkDeviceMemory      VertexBufferMemory;
	VkDeviceMemory      IndexBufferMemory;
	VkDeviceSize        VertexBufferSize;
	VkDeviceSize        IndexBufferSize;
	VkBuffer            VertexBuffer;
	VkBuffer            IndexBuffer;
};

// Each viewport will hold 1 ImGui_ImplVulkanH_WindowRenderBuffers
// [Please zero-clear before use!]
struct ImGui_ImplVulkan_WindowRenderBuffers
{
	uint32_t            Index;
	uint32_t            Count;
	ImGui_ImplVulkan_FrameRenderBuffers* FrameRenderBuffers;
};

// For multi-viewport support:
// Helper structure we store in the void* RendererUserData field of each ImGuiViewport to easily retrieve our backend data.
struct ImGui_ImplVulkan_ViewportData
{
	ImGui_ImplVulkanH_Window                Window;                 // Used by secondary viewports only
	ImGui_ImplVulkan_WindowRenderBuffers    RenderBuffers;          // Used by all viewports
	bool                                    WindowOwned;
	bool                                    SwapChainNeedRebuild;   // Flag when viewport swapchain resized in the middle of processing a frame

	ImGui_ImplVulkan_ViewportData() { WindowOwned = SwapChainNeedRebuild = false; memset(&RenderBuffers, 0, sizeof(RenderBuffers)); }
	~ImGui_ImplVulkan_ViewportData() { }
};

// Vulkan data
struct ImGui_ImplVulkan_Data
{
	ImGui_ImplVulkan_InitInfo   VulkanInitInfo;
	VkDeviceSize                BufferMemoryAlignment;
	VkPipelineCreateFlags       PipelineCreateFlags;
	VkDescriptorSetLayout       DescriptorSetLayout;
	VkPipelineLayout            PipelineLayout;
	VkPipeline                  Pipeline;               // pipeline for main render pass (created by app)
	VkPipeline                  PipelineForViewports;   // pipeline for secondary viewports (created by backend)
	VkShaderModule              ShaderModuleVert;
	VkShaderModule              ShaderModuleFrag;

	// Font data
	VkSampler                   FontSampler;
	VkDeviceMemory              FontMemory;
	VkImage                     FontImage;
	VkImageView                 FontView;
	VkDescriptorSet             FontDescriptorSet;
	VkCommandPool               FontCommandPool;
	VkCommandBuffer             FontCommandBuffer;

	// Render buffers for main window
	ImGui_ImplVulkan_WindowRenderBuffers MainWindowRenderBuffers;

	ImGui_ImplVulkan_Data()
	{
		memset((void*)this, 0, sizeof(*this));
		BufferMemoryAlignment = 256;
	}
};


namespace Madam::UI {

	void GUI::DrawViewport(const ImDrawList* parentList, const ImDrawCmd* pcmd) {
		
		ImDrawData* draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_Data* bd = ImGui::GetCurrentContext() ? (ImGui_ImplVulkan_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
		VkCommandBuffer cmdBuffer = Rendering::Renderer::Get().GetCurrentCommandBuffer();
		ImGui_ImplVulkan_ViewportData* viewportRenderData = (ImGui_ImplVulkan_ViewportData*)draw_data->OwnerViewport->RendererUserData;
		ImGui_ImplVulkan_WindowRenderBuffers* wrb = &viewportRenderData->RenderBuffers;
		ImGui_ImplVulkan_FrameRenderBuffers* rb = &wrb->FrameRenderBuffers[wrb->Index];

		int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
		int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);

		PipelineInfo* pipelineInfo = reinterpret_cast<PipelineInfo*>(pcmd->UserCallbackData);

		// Bind pipeline:
		pipelineInfo->pipeline->bind(cmdBuffer);

		// Bind Vertex And Index Buffer:
		if (draw_data->TotalVtxCount > 0)
		{
			VkBuffer vertex_buffers[1] = { rb->VertexBuffer };
			VkDeviceSize vertex_offset[1] = { 0 };
			vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertex_buffers, vertex_offset);
			vkCmdBindIndexBuffer(cmdBuffer, rb->IndexBuffer, 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
		}

		// Setup viewport:
		{
			VkViewport viewport;
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = (float)fb_width;
			viewport.height = (float)fb_height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		}

		// Setup scale and translation:
		// Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
		{
			float scale[2];
			scale[0] = 2.0f / draw_data->DisplaySize.x;
			scale[1] = 2.0f / draw_data->DisplaySize.y;
			float translate[2];
			translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
			translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];
			vkCmdPushConstants(cmdBuffer, pipelineInfo->layout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
			vkCmdPushConstants(cmdBuffer, pipelineInfo->layout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);
		}

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		// (Because we merged all buffers into a single one, we maintain our own offset into them)
		int global_vtx_offset = 0;
		int global_idx_offset = 0;

		// Project scissor/clipping rectangles into framebuffer space
		ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
		ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

		// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
		if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
		if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
		if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
		if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
		if (clip_max.x > clip_min.x || clip_max.y > clip_min.y) {
			return;
		}

		// Apply scissor/clipping rectangle
		VkRect2D scissor;
		scissor.offset.x = (int32_t)(clip_min.x);
		scissor.offset.y = (int32_t)(clip_min.y);
		scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
		scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

		// Bind DescriptorSet with font or user texture
		VkDescriptorSet desc_set[1] = { (VkDescriptorSet)pcmd->TextureId };

		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineInfo->layout , 0, 1, desc_set, 0, nullptr);

		// Draw
		vkCmdDrawIndexed(cmdBuffer, pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
	}


	GUI::GUI() : EngineInterface("GUI")
	{
		pendingEntityDeletion = CreateRef<Entity>(Entity());
	}

	//Fix please, descriptorsets and image are not be deallocated
	GUI::~GUI() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		guiPool = nullptr;
	}

	void GUI::OnAttach() {
		Device& device = Rendering::Renderer::GetDevice();
		guiPool =
			DescriptorPool::Builder(device)
			.setMaxSets(1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.build();

		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		Style(io); 

		initInfo = Rendering::Renderer::Get().GetImGuiInitInfo();
		init_info->DescriptorPool = guiPool.get()->descriptor_pool;
		init_info->MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info->RenderPass = Rendering::Renderer::Get().GetSwapChainRenderPass();

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(Rendering::Renderer::GetDevice().device(), &samplerInfo, nullptr, &viewportSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}

		ImGui_ImplGlfw_InitForVulkan(Application::Get().GetWindow().getGLFWwindow(), true);
		ImGui_ImplVulkan_Init(init_info);

		//viewport descriptors
		RefreshRenderTargetDescriptors();

		SetupIcons();

		ImGui_ImplVulkan_CreateFontsTexture();
		viewportCallback = ImDrawCallback(DrawViewport);
		CreateViewportPipeline();
		gizmoButtonStates = { false, false, false};

		SetupEvents();
	}

	void GUI::SetupIcons()
	{
		for (size_t i = 0; i < ICON_SIZE; i++)
		{
			TextureData textureData;
			textureData.format = Rendering::Image::Format::RGBA;
			textureData.samplerWrap = Rendering::TextureWrap::Clamp;
			
			IconInfo iconInfo;
			iconInfo.texture = Texture::Create(textureData, iconFilepaths[i]);

			iconInfo.descriptorSet = ImGui_ImplVulkan_AddTexture(std::static_pointer_cast<VulkanTexture>(iconInfo.texture)->GetSampler(), std::static_pointer_cast<VulkanTexture>(iconInfo.texture)->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			
			icons.emplace(iconFilepaths[i].filename().string(), iconInfo);
		}
	}

	void GUI::RefreshRenderTargetDescriptors()
	{
		viewportSets.clear();
		gameSets.clear();
		for (int frame = 0; frame < Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT; ++frame)
		{
			viewportSets.push_back(ImGui_ImplVulkan_AddTexture(viewportSampler,
				Rendering::Renderer::Get().GetImageViewForFrame(frame, 0), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
			gameSets.push_back(ImGui_ImplVulkan_AddTexture(viewportSampler,
				Rendering::Renderer::Get().GetImageViewForFrame(frame, 2), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
		}
		viewportSet = viewportSets.front();
		gameSet = gameSets.front();
	}

	void GUI::SetupEvents() {
		EventSystem::Get().AddListener(this, &GUI::OnResizeEvent);
		EventSystem::Get().AddListener(this, &GUI::OnRenderPassEvent);
		EventSystem::Get().AddListener(this, &GUI::OnSceneChangeEvent);
		EventSystem::Get().AddListener(this, &GUI::OnMouseMoveRawEvent);
		EventSystem::Get().AddListener(this, &GUI::OnMouseScrollEvent);
	}

	//Events
	void GUI::OnSceneChangeEvent(SceneChangeEvent* e) {
		selectedEntity = nullptr;
	}

	void GUI::OnRenderPassEvent(NextRenderPassEvent* e) {
		if (e->renderpassIndex == 2) {
			OnRender();
		}
	}

	void GUI::OnResizeEvent(WindowResizeEvent* e) {
		for (auto set : viewportSets) ImGui_ImplVulkan_RemoveTexture(set);
		for (auto set : gameSets) ImGui_ImplVulkan_RemoveTexture(set);
		viewportSets.clear();
		gameSets.clear();
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplVulkan_Shutdown();
		initInfo = Rendering::Renderer::Get().GetImGuiInitInfo();
		init_info->DescriptorPool = guiPool.get()->descriptor_pool;
		init_info->MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info->RenderPass = Rendering::Renderer::Get().GetSwapChainRenderPass();
		ImGui_ImplGlfw_InitForVulkan(Application::Get().GetWindow().getGLFWwindow(), true);
		ImGui_ImplVulkan_Init(init_info);
		RefreshRenderTargetDescriptors();
		CreateViewportPipeline();
	}

	void GUI::OnMouseMoveRawEvent(MouseMoveRawEvent* e)
	{
		if (!isMovingViewportCamera)
		{
			ImGui_ImplGlfw_CursorPosCallback(Application::Get().GetWindow().getGLFWwindow(), e->x, e->y);
		}
	}

	void GUI::OnMouseScrollEvent(MouseScrollEvent* e)
	{
		ImGui_ImplGlfw_ScrollCallback(Application::Get().GetWindow().getGLFWwindow(), e->x, e->y);
	}

	void GUI::OnDetach() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		fonts.clear();
		EventSystem::Get().RemoveListener(this, &GUI::OnResizeEvent);
		EventSystem::Get().RemoveListener(this, &GUI::OnRenderPassEvent);
		EventSystem::Get().RemoveListener(this, &GUI::OnSceneChangeEvent);
	}

	void GUI::Style(ImGuiIO& io) {
		//Fonts
		std::vector<std::string> fontPaths =
		{
			"resources\\fonts\\Roboto-Medium.ttf",
			"resources\\fonts\\Roboto-Bold.ttf"
		};
		ImFont* font;
		for each (std::string fontPath in fontPaths)
		{
			MADAM_CORE_INFO("Loading Font: {0}", fontPath);
			font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);
			fonts.push_back(font);
		}

		style = ImGui::GetStyle();
		style.WindowPadding = ImVec2(10, 10);
		style.FramePadding = ImVec2(7, 5);
		style.ItemSpacing = ImVec2(8, 7);
		style.WindowRounding = 5.0f;
		style.ChildRounding = 4.0f;
		style.FrameRounding = 4.0f;
		style.PopupRounding = 5.0f;
		style.GrabRounding = 4.0f;
		style.TabRounding = 4.0f;
		style.ScrollbarRounding = 5.0f;
		style.ScrollbarSize = 12.0f;
		style.WindowBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.Colors[ImGuiCol_Text] = RGBCon(226, 235, 244);
		style.Colors[ImGuiCol_TextDisabled] = RGBCon(132, 150, 169);
		style.Colors[ImGuiCol_WindowBg] = RGBCon(20, 26, 35);
		style.Colors[ImGuiCol_ChildBg] = RGBCon(18, 24, 33);
		style.Colors[ImGuiCol_PopupBg] = RGBCon(29, 38, 51);
		style.Colors[ImGuiCol_Border] = RGBCon(48, 62, 78);
		style.Colors[ImGuiCol_FrameBg] = RGBCon(31, 42, 56);
		style.Colors[ImGuiCol_FrameBgHovered] = RGBCon(42, 58, 75);
		style.Colors[ImGuiCol_FrameBgActive] = RGBCon(48, 71, 91);
		style.Colors[ImGuiCol_TitleBg] = RGBCon(25, 33, 44);
		style.Colors[ImGuiCol_TitleBgActive] = RGBCon(31, 43, 57);
		style.Colors[ImGuiCol_MenuBarBg] = RGBCon(26, 35, 47);
		style.Colors[ImGuiCol_Tab] = RGBCon(31, 42, 55);
		style.Colors[ImGuiCol_TabHovered] = RGBCon(47, 86, 105);
		style.Colors[ImGuiCol_TabActive] = RGBCon(48, 105, 127);
		style.Colors[ImGuiCol_TabUnfocused] = RGBCon(27, 36, 49);
		style.Colors[ImGuiCol_TabUnfocusedActive] = RGBCon(40, 70, 86);
		style.Colors[ImGuiCol_Button] = RGBCon(39, 62, 79);
		style.Colors[ImGuiCol_ButtonHovered] = RGBCon(53, 100, 125);
		style.Colors[ImGuiCol_ButtonActive] = RGBCon(62, 123, 152);
		style.Colors[ImGuiCol_Header] = RGBCon(38, 63, 79);
		style.Colors[ImGuiCol_HeaderHovered] = RGBCon(50, 91, 112);
		style.Colors[ImGuiCol_HeaderActive] = RGBCon(59, 116, 142);
		style.Colors[ImGuiCol_CheckMark] = RGBCon(114, 206, 226);
		style.Colors[ImGuiCol_SliderGrab] = RGBCon(81, 161, 190);
		style.Colors[ImGuiCol_SliderGrabActive] = RGBCon(114, 206, 226);
		style.Colors[ImGuiCol_Separator] = RGBCon(48, 62, 78);
		style.Colors[ImGuiCol_ResizeGrip] = RGBCon(49, 88, 107);
		style.Colors[ImGuiCol_ResizeGripHovered] = RGBCon(74, 140, 166);
		style.Colors[ImGuiCol_ResizeGripActive] = RGBCon(100, 177, 202);
		style.Colors[ImGuiCol_DockingPreview] = RGBCon(73, 143, 172);

		ImGui::GetStyle() = style;
	}

	void GUI::OnUpdate() {
		if (*pendingEntityDeletion) {
			Application::Get().GetScene().DestroyEntity(*pendingEntityDeletion);
			pendingEntityDeletion = CreateRef<Entity>(Entity());
		}

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)Application::Get().GetWindow().getWidth(), (float)Application::Get().GetWindow().getHeight());

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		const int nextFrame = Rendering::Renderer::Get().GetNextFrameIndex();
		viewportSet = viewportSets[nextFrame];
		gameSet = gameSets[nextFrame];
		ImGuizmo::BeginFrame();

		EditorUI();
	}

	void GUI::OnRender() {

		ImGui::Render();

		VkCommandBuffer commandBuffer = Rendering::Renderer::Get().GetCurrentCommandBuffer();
		Record(commandBuffer);
	}

	void GUI::Record(VkCommandBuffer commandBuffer) {
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void GUI::EditorUI() {
		ImGui::PushFont(fonts[0]);
		MenuBar();
		DockingSpace();
		if (windowStates & RENDER_SETTINGS_WINDOW) {
			RenderingSettings();
		}
		if (windowStates & DEBUG_INFO_WINDOW)
		{
			DebugWindow();
		}
		Viewport();
		Game();
		Hierarchy();
		Inspector();
		Project();
		Console();
		ImGui::PopFont();
	}

	void GUI::MenuBar() {
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open Project...", nullptr, false, !Application::Get().IsPlay())) {
					std::filesystem::path projectFile;
					if (OpenFileDialog(projectFile, L"conf",
						L"Iron Maiden Project\0ProjectInfo.conf\0All Files\0*.*\0")) {
						const auto projectPath = projectFile.parent_path();
						if (projectFile.filename() != L"ProjectInfo.conf" ||
							!std::filesystem::is_directory(projectPath / ASSET_DIR) ||
							!Project::loadProject(projectPath)) {
							MADAM_ERROR("Unable to open project: {0}", projectPath.string());
						}
						else {
							selectedAsset.reset();
							curDir = ASSET_DIR;
							sceneDir.clear();
							Application::Get().SwitchScenes(CreateRef<Scene>());
							Application::Get().SaveSession();
							const auto& config = Application::Get().GetConfig();
							const std::string title = std::string("Iron Maiden Engine - ") + config.version +
								" - " + Project::Get().getProjectInfo().projectName;
							glfwSetWindowTitle(Application::Get().GetWindow().getGLFWwindow(), title.c_str());
							MADAM_INFO("Opened project: {0}", projectPath.string());
						}
					}
				}
				ImGui::Separator();
				if (ImGui::MenuItem("New Scene")) {
					//Move this behaviour somewhere else (probably scene class)
					Scene newScene{};
					Entity camera = newScene.CreateEntity();
					camera.GetComponent<CMetadata>().name = "Editor Camera";
					camera.GetComponent<CMetadata>().is_hidden_entity = true;
					camera.GetComponent<CTransform>().translation.z = -2.5f;
					Rendering::CameraData cameraData;
					cameraData.projectionType = Rendering::CameraData::ProjectionType::Perspective;
					cameraData.perspective = Rendering::CameraData::Perspective(glm::radians(50.0f), Application::Get().GetAspectRatio(), 0.1f, 1000.0f);
					camera.AddComponent<CCamera>(cameraData);
					camera.GetComponent<CCamera>().editorOnly = true;
					camera.GetComponent<CCamera>().cameraHandle->SetViewDirection(glm::vec3(0.f, 2.0f, 0.f), glm::vec3(0.f, 0.f, 0.f));
					camera.GetComponent<CCamera>().cameraHandle->SetMain();
					Application::Get().SwitchScenes(CreateRef<Scene>(std::move(newScene)));
				}
				if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
					if (OpenFileDialog(sceneDir, L"scene", L"scene")) {
						if (Application::Get().LoadScene(sceneDir) != IRM_SUCCESS)
						{
							MADAM_ERROR("Unable to Load Scene");
						}
					}
					else {
						// User cancelled or error occurred
						ShowMessageBox(L"No scene file selected.", L"Info", MB_OK | MB_ICONINFORMATION);
					}
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					if (sceneDir.empty())
					{
						if (SaveFileDialog(sceneDir, L"scene", L"scene")) {
							MADAM_CORE_INFO("Current Directory: {0}", std::filesystem::current_path().string());
							Application::Get().SaveScene(sceneDir);
							MADAM_CORE_INFO("After SaveScene Current Directory: {0}", std::filesystem::current_path().string());
						}
						else {
							// User canceled or error occurred
							ShowMessageBox(L"Could not save scene.", L"Info", MB_OK | MB_ICONINFORMATION);
						}
					}
					Application::Get().SaveScene("temp.scene");
				}
				if (ImGui::MenuItem("Save As..")) {
					if (SaveFileDialog(sceneDir, L"scene", L"scene")) {
						MADAM_CORE_INFO("Current Directory: {0}", std::filesystem::current_path().string());
						Application::Get().SaveScene(sceneDir);
					}
					else {
						// User canceled or error occurred
						ShowMessageBox(L"Could not save scene.", L"Info", MB_OK | MB_ICONINFORMATION);
					}
				}
				if (ImGui::MenuItem("Exit")) Application::Get().Quit();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
				if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {}
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
				if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
				if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Settings"))
			{
				if (ImGui::MenuItem("Render Settings")) 
				{
					windowStates |= RENDER_SETTINGS_WINDOW;
				}
				if (ImGui::MenuItem("Debug Information"))
				{
					windowStates |= DEBUG_INFO_WINDOW;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void GUI::DockingSpace() {
		static bool dockspace_open = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", &dockspace_open, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
		{
			ImGui::PopStyleVar(2);
		}

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			dockspaceId = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		ImGui::End();
	}

	void GUI::Viewport() {
		bool imageHovered = false;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {

			const ImVec2 displaySize = ImGui::GetContentRegionAvail();

			ImVec2 uv0 = ImVec2(0, 0);
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddCallback(viewportCallback, &viewportPipelineInfo);
			ImGui::Image(viewportSet, displaySize, uv0, ImVec2(1,1), ImVec4(1, 1, 1, 1));
			viewportImageTopLeft = ImGui::GetItemRectMin();
			viewportImageSize = displaySize;
			imageHovered = ImGui::IsItemHovered();
			drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PROJECT_ITEM"))
				{
					const wchar_t* wPath = (const wchar_t*)payload->Data;
					std::string path = ConvertWideToUtf8(wPath);

					MADAM_CORE_INFO("Opening scene: " + path);
					if (Application::Get().LoadScene(path) != VK_SUCCESS)
					{
						MADAM_ERROR("Unable to Load Scene");
					}
				}
				ImGui::EndDragDropTarget();
			}

			DrawViewportOverlays();

			//imGuizmo
			DrawViewportGizmoButtons();
			if (selectedEntity) {
				if (Rendering::CameraHandle::GetMain().GetCameraData().projectionType == Rendering::CameraData::ProjectionType::Orthographic) {
					ImGuizmo::SetOrthographic(true);
				}
				else {
					ImGuizmo::SetOrthographic(false);
				}
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(viewportImageTopLeft.x, viewportImageTopLeft.y,
					viewportImageSize.x, viewportImageSize.y);
				

				auto& cameraHandle = Rendering::CameraHandle::GetMain();
				glm::mat4 cameraProjection = cameraHandle.GetProjection();
				cameraProjection[1][1] *= -1;

				glm::mat4 cameraView = cameraHandle.GetView();
				
				UUID parentUUID = selectedEntity->GetComponent<CRelationship>().parent;
				glm::mat4 parentTransform;
				glm::mat4 transform = selectedEntity->GetComponent<CTransform>().TransformMatrix();

				if (parentUUID != null)
				{
					parentTransform = Application::Get().GetScene().GetWorldTransform(parentUUID);
				}
				else
				{
					parentTransform = glm::mat4(1.0f);
				}

				glm::mat4 worldTransform = parentTransform * transform;

				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)ImGuizmoType,
					ImGuizmo::LOCAL, glm::value_ptr(worldTransform));

				if (ImGuizmo::IsUsing()) {
					if (parentUUID != null)
					{
						
						transform = glm::inverse(parentTransform) * worldTransform;
					}
					else
					{
						transform = worldTransform;
					}
					selectedEntity->GetComponent<CTransform>().UpdateTransform(transform);
				}
			}
		}
		if (imageHovered || isMovingViewportCamera)
		{
			if (Input::Get().IsMouseButtonPress(MouseCode::RIGHTMOUSEBUTTON) ||
				Input::Get().IsMouseButtonPress(MouseCode::MIDDLEMOUSEBUTTON))
			{
				Application::Get().GetWindow().SetCursorState(CursorState::DISABLED);
				if (glfwRawMouseMotionSupported())
					glfwSetInputMode(Application::Get().GetWindow().getGLFWwindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
				isMovingViewportCamera = true;
			}
			else
			{
				if (isMovingViewportCamera)
				{
					if (glfwRawMouseMotionSupported())
						glfwSetInputMode(Application::Get().GetWindow().getGLFWwindow(), GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
					isMovingViewportCamera = false;
					Application::Get().GetWindow().SetCursorState(CursorState::NORMAL);
					Application::Get().GetWindow().PopCursorPosition();
				}
				Application::Get().GetWindow().SetCursorState(CursorState::NORMAL);
				Application::Get().GetWindow().SetCursorIcon(CursorType::CROSSHAIR);
			}
		}
		else
		{
			Application::Get().GetWindow().SetCursorState(CursorState::NORMAL);
			Application::Get().GetWindow().SetCursorIcon(CursorType::ARROW);
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void GUI::Game()
	{
		if (dockspaceId) ImGui::SetNextWindowDockID(dockspaceId, ImGuiCond_FirstUseEver);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			const ImVec2 imageSize = ImGui::GetContentRegionAvail();
			if (imageSize.x > 1.0f && imageSize.y > 1.0f)
			{
				if (Application::Get().GetScene().GetGameCameraEntity())
				{
					ImGui::Image(gameSet, imageSize);
				}
				else
				{
					const ImVec2 topLeft = ImGui::GetCursorScreenPos();
					ImGui::InvisibleButton("##NoGameCamera", imageSize);
					ImDrawList* drawList = ImGui::GetWindowDrawList();
					const ImVec2 bottomRight(topLeft.x + imageSize.x, topLeft.y + imageSize.y);
					drawList->AddRectFilled(topLeft, bottomRight, IM_COL32(0, 0, 0, 255));
					const char* title = "No camera in scene";
					const char* help = "Add a Camera component to an entity to preview the game.";
					const ImVec2 titleSize = ImGui::CalcTextSize(title);
					const ImVec2 helpSize = ImGui::CalcTextSize(help);
					const float centerX = topLeft.x + imageSize.x * 0.5f;
					const float centerY = topLeft.y + imageSize.y * 0.5f;
					drawList->AddText(ImVec2(centerX - titleSize.x * 0.5f, centerY - titleSize.y - 5.0f),
						IM_COL32(225, 231, 238, 255), title);
					drawList->AddText(ImVec2(centerX - helpSize.x * 0.5f, centerY + 5.0f),
						IM_COL32(128, 143, 160, 255), help);
				}
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void GUI::Hierarchy() {

		if (ImGui::Begin("Hierarchy")) {
			Application::Get().GetScene().GetAllEntitiesWith<CMetadata>().each([&](auto entityId, auto& gameObject)
			{
				Entity entity {entityId, &Application::Get().GetScene()};
				if (entity.GetComponent<CRelationship>().parent == null && entity.GetComponent<CMetadata>().is_hidden_entity != true)
				{
					DrawEntityNode(entity);
				}
			});

			if (hasEntityRelationshipChanged)
			{
				glm::mat4 inverseParentWorldMatrix = glm::inverse(Application::Get().GetScene().GetWorldTransform(Application::Get().GetScene().GetEntity(newParentEntityUUID)));

				glm::mat4 localMatrixChild = inverseParentWorldMatrix * Application::Get().GetScene().GetWorldTransform(Application::Get().GetScene().GetEntity(newChildEntityUUID));

				Application::Get().GetScene().AddEntityRelationship(Application::Get().GetScene().GetEntity(newParentEntityUUID), Application::Get().GetScene().GetEntity(newChildEntityUUID));

				Application::Get().GetScene().GetEntity(newChildEntityUUID).GetComponent<CTransform>().UpdateTransform(localMatrixChild);

				hasEntityRelationshipChanged = false;
			}
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ITEM"))
			{
				UUID uuid = *(UUID*)payload->Data;
				Entity other = Application::Get().GetScene().GetEntity(uuid);
				if (other != null)
				{
					Application::Get().GetScene().RemoveParentEntityRelationship(other);
				}
				else
				{
					MADAM_CORE_ERROR("Entity {0} is NULL", uuid);
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			selectedEntity = nullptr;
		}

		if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems)) {
			if (ImGui::MenuItem("Create Empty")) {
				auto entity = Application::Get().GetScene().CreateEntity();
			}
			if (ImGui::BeginMenu("3D")) {
				if (ImGui::MenuItem("Quad"))
				{
					auto entity = Application::Get().GetScene().CreateEntity("Quad");
					entity.AddComponent<CMeshRenderer>();
					entity.GetComponent<CMeshRenderer>().mesh = StaticMesh::Create(MeshPrimatives::Quad);
				}
				if (ImGui::MenuItem("Cube"))
				{
					auto entity = Application::Get().GetScene().CreateEntity("Cube");
					entity.AddComponent<CMeshRenderer>();
					entity.GetComponent<CMeshRenderer>().mesh = StaticMesh::Create(MeshPrimatives::Cube);
				}
				if (ImGui::MenuItem("Sphere"))
				{
					auto entity = Application::Get().GetScene().CreateEntity("Sphere");
					entity.AddComponent<CMeshRenderer>();
					entity.GetComponent<CMeshRenderer>().mesh = StaticMesh::Create(MeshPrimatives::Sphere);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Lights"))
			{
				if (ImGui::MenuItem("Point Light"))
				{
					auto entity = Application::Get().GetScene().CreateEntity("Point Light");
					entity.AddComponent<CPointLight>();
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void GUI::Inspector() {
		if (ImGui::Begin("Properties")) {
			if (selectedEntity) {
				DrawEntityComponents(*selectedEntity);

				if (ImGui::Button("Add Component")) {
					ImGui::OpenPopup("AddComponent");
				}
				if (ImGui::BeginPopup("AddComponent")) 
				{
					const bool hasCamera = selectedEntity->HasComponent<CCamera>();
					const bool hasMeshRenderer = selectedEntity->HasComponent<CMeshRenderer>();
					const bool hasPointLight = selectedEntity->HasComponent<CPointLight>();
					const bool hasMaterial = selectedEntity->HasComponent<CMaterial>();
					const bool hasCSharpScript = selectedEntity->HasComponent<CCSharpScript>();

					if (ImGui::MenuItem("Camera", nullptr, false, !hasCamera)) {
						selectedEntity->AddComponent<CCamera>();
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Mesh Renderer", nullptr, false, !hasMeshRenderer)) {
						selectedEntity->AddComponent<CMeshRenderer>();
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Point Light", nullptr, false, !hasPointLight)) {
						selectedEntity->AddComponent<CPointLight>();
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Material", nullptr, false, !hasMaterial)) {
						selectedEntity->AddComponent<CMaterial>();
						CMaterial& mat = selectedEntity->GetComponent<CMaterial>();
						mat.is_custom = true;
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("C# Script", nullptr, false, !hasCSharpScript)) {
						selectedEntity->AddComponent<CCSharpScript>();
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
			}
			else {
				ImGui::Text("No entity selected");
			}
		}
		ImGui::End();

		if (ImGui::Begin("Asset")) {
			if (selectedAsset) {
				DrawAssetInfo(selectedAsset);
			}
			else {
				ImGui::Text("No asset selected");
			}
		}
		ImGui::End();
	}

	void GUI::Project() 
	{
		std::filesystem::path projectDir = Project::Get().getProjectDirectory();
		std::filesystem::path activeDir = projectDir / curDir;
		std::filesystem::path relativeActiveDir = curDir;
		std::filesystem::path hoveredItemPath;
		if (ImGui::Begin("FileSystem", nullptr, ImGuiWindowFlags_MenuBar)) 
		{
			ImGui::PushStyleColor(ImGuiCol_Button, RGBCon(30, 42, 56));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, RGBCon(49, 80, 101));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, RGBCon(58, 106, 130));
			if (ImGui::BeginMenuBar())
			{
				std::filesystem::path tempDir;
				bool arrow = false;
				for (const auto& partDir : relativeActiveDir)
				{
					tempDir /= partDir;
					if (arrow)
					{
						ImGui::SameLine();
						ImGui::Text(" > ");
						ImGui::SameLine();
					}
					else
					{
						arrow = true;
					}
					if (ImGui::Button(partDir.filename().string().c_str()))
					{
						curDir = tempDir;
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PROJECT_ITEM"))
						{
							const wchar_t* wPath = (const wchar_t*)payload->Data;
							std::filesystem::path path(wPath);

						const auto destination = projectDir / tempDir / path.filename();
						std::filesystem::rename(path, destination);
						Project::Get().getAssetManager().MoveMetadata(path, destination);
						Project::Get().getAssetManager().SaveMetaData();
						}
						ImGui::EndDragDropTarget();
					}
				}
				ImGui::EndMenuBar();
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			std::vector<std::filesystem::directory_entry> folders;
			std::vector<std::filesystem::directory_entry> files;
			for (const auto& entry : std::filesystem::directory_iterator(activeDir))
			{
				if (entry.is_directory()) folders.push_back(entry);
				else if (entry.is_regular_file()) files.push_back(entry);
			}

			const float tileSize = 84.0f;
			const float gap = 12.0f;
			const float availableWidth = ImGui::GetContentRegionAvail().x;
			const float contentStartX = ImGui::GetCursorPosX();
			const int columns = std::max(1, static_cast<int>((availableWidth + gap) / (tileSize + gap)));
			const int totalItems = static_cast<int>(folders.size() + files.size());
			int tileIndex = 0;
			const ImVec2 thumbnailSize(
				std::max(1.0f, tileSize - 2.0f * ImGui::GetStyle().FramePadding.x),
				std::max(1.0f, tileSize - 2.0f * ImGui::GetStyle().FramePadding.y));
			auto placeTile = [&]() {
				if (tileIndex % columns == 0) {
					const int rowCount = std::min(columns, totalItems - tileIndex);
					const float rowWidth = rowCount * tileSize + (rowCount - 1) * gap;
					ImGui::SetCursorPosX(contentStartX + std::max(0.0f, (availableWidth - rowWidth) * 0.5f));
				}
				else ImGui::SameLine(0.0f, gap);
				++tileIndex;
			};

			for (const auto& entry : folders)
			{
					placeTile();
					ImGui::BeginGroup();

					std::string buttonID = "FolderButton##" + entry.path().string();
					if (ImGui::ImageButton(buttonID.c_str(), icons["Folder.png"].descriptorSet, thumbnailSize))
					{
						curDir /= std::filesystem::u8path(entry.path().filename().string());
						MADAM_CORE_INFO("Folder selected");
					}
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped))
					{
						hoveredItemPath = entry.path();
					}
					std::string text = entry.path().filename().string();
					std::string truncatedText = TruncateTextToFit(text, tileSize);
					ImVec2 textSize = ImGui::CalcTextSize(truncatedText.c_str());
					float textXOffset = (tileSize - textSize.x) / 2;

					if (textXOffset < 0)
					{
						textXOffset = 0;
					}

					ImGui::SetCursorPosX(ImGui::GetCursorPos().x + textXOffset);
					ImGui::TextWrapped(truncatedText.c_str());
					ImGui::EndGroup();
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped))
						hoveredItemPath = entry.path();

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PROJECT_ITEM"))
						{
							const wchar_t* wPath = (const wchar_t*)payload->Data;
							std::filesystem::path path(wPath);

							const auto destination = entry.path() / path.filename();
							std::filesystem::rename(path, destination);
							Project::Get().getAssetManager().MoveMetadata(path, destination);
							Project::Get().getAssetManager().SaveMetaData();
						}
						ImGui::EndDragDropTarget();
					}

			}

			for (size_t i = 0; i < files.size(); i++)
			{
				const AssetMetadata* metadata = &Project::Get().getAssetManager().GetMetadata(files[i].path());

				if (!metadata->IsValid())
				{
					Project::Get().getAssetManager().AppendMetaData(files[i].path());
					metadata = &Project::Get().getAssetManager().GetMetadata(files[i].path());
				}
				placeTile();
				ImGui::BeginGroup();
				std::string buttonID = "FileButton##" + files[i].path().string();
				const AssetType type = metadata->IsValid()
					? Project::Get().getAssetManager().GetAssetType(metadata->uuid) : AssetType::NONE;
				if (type == AssetType::TEXTURE)
				{
					IconInfo& icon = loadedIconTextures[metadata->uuid];
					if (!icon.texture)
					{
						icon.texture = std::dynamic_pointer_cast<Texture>(Project::Get().getAssetManager().GetAsset(metadata->uuid));
						if (icon.texture)
							icon.descriptorSet = ImGui_ImplVulkan_AddTexture(std::static_pointer_cast<VulkanTexture>(icon.texture)->GetSampler(), std::static_pointer_cast<VulkanTexture>(icon.texture)->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					}
					if (ImGui::ImageButton(buttonID.c_str(), icon.descriptorSet ? icon.descriptorSet : icons["File.png"].descriptorSet, thumbnailSize))
						selectedAsset = Project::Get().getAssetManager().GetAsset(metadata->uuid);
				}
				else if (type == AssetType::SCENE)
				{
				if (ImGui::ImageButton(buttonID.c_str(), icons["Scene.png"].descriptorSet, thumbnailSize))
						selectedAsset = Project::Get().getAssetManager().GetAsset(metadata->uuid);
				}
				else if (type == AssetType::MATERIAL)
				{
					DrawMaterialThumbnail(metadata->uuid, buttonID.c_str(), ImVec2(tileSize, tileSize));
					if (ImGui::IsItemClicked())
						selectedAsset = Project::Get().getAssetManager().GetAsset(metadata->uuid);
				}
				else if (type == AssetType::SCRIPT || type == AssetType::SHADER)
				{
					const char* icon = type == AssetType::SCRIPT ? "Script.png" : "Shader.png";
					if (ImGui::ImageButton(buttonID.c_str(), icons[icon].descriptorSet, thumbnailSize))
						selectedAsset = Project::Get().getAssetManager().GetAsset(metadata->uuid);
				}
				else
				{
				if (ImGui::ImageButton(buttonID.c_str(), icons["File.png"].descriptorSet, thumbnailSize))
					{
						selectedAsset = metadata->IsValid() ? Project::Get().getAssetManager().GetAsset(metadata->uuid) : nullptr;
					}
				}
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped))
					hoveredItemPath = files[i].path();

					std::string text = files[i].path().stem().string();
					std::string truncatedText = TruncateTextToFit(text, tileSize);
					ImVec2 textSize = ImGui::CalcTextSize(truncatedText.c_str());
					float textXOffset = (tileSize - textSize.x) / 2;

					ImGui::SetCursorPosX(ImGui::GetCursorPos().x + textXOffset);
					ImGui::TextWrapped(truncatedText.c_str());
					ImGui::EndGroup();
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped))
						hoveredItemPath = files[i].path();

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
					{
						const wchar_t* itempath = files[i].path().c_str();
						ImGui::SetDragDropPayload("PROJECT_ITEM", itempath, (std::wcslen(itempath) + 1) * sizeof(wchar_t));
						ImGui::EndDragDropSource();
					}

			}

			if (ImGui::BeginPopupContextWindow(0, 1))
			{
				if (!isPopupContextOpen)
				{
					popupContextSelectedItem = hoveredItemPath;
					isPopupContextOpen = true;
				}
				if (ImGui::BeginMenu("Create"))
				{
					if (ImGui::MenuItem("Folder"))
					{
						CreateDirectory(activeDir / "New folder");
					}
					if (ImGui::MenuItem("Scene"))
					{
						//Update this
						if (Application::Get().SaveScene(activeDir) != IRM_SUCCESS)
						{
							MADAM_ERROR("Unable to create scene");
						}
					}
					if (ImGui::MenuItem("Material"))
					{
						std::filesystem::path path = activeDir / "New Material.mat";
						for (int index = 2; std::filesystem::exists(path); ++index)
							path = activeDir / ("New Material " + std::to_string(index) + ".mat");
						selectedAsset = Project::Get().getAssetManager().CreateMaterial(std::filesystem::absolute(path));
						if (!selectedAsset)
							MADAM_ERROR("Unable to create material: {0}", path.string());
					}
					if (ImGui::MenuItem("Shader"))
					{
						std::filesystem::path path = activeDir / "New Shader.frag";
						for (int index = 2; std::filesystem::exists(path) ||
							std::filesystem::exists(std::filesystem::path(path).replace_extension(".vert")); ++index)
							path = activeDir / ("New Shader " + std::to_string(index) + ".frag");
						selectedAsset = Project::Get().getAssetManager().CreateShader(std::filesystem::absolute(path));
						if (!selectedAsset) {
							MADAM_ERROR("Unable to create shader: {0}", path.string());
						}
						else {
							shaderCompileAsset = selectedAsset->handle;
							std::string message;
							if (!Project::Get().getAssetManager().CompileShader(selectedAsset->handle, &message))
								MADAM_ERROR("Shader compilation failed: {0}", message);
							shaderCompileFeedback = message;
						}
					}
					if (ImGui::MenuItem("C# Script"))
					{
						std::filesystem::path path = activeDir / "NewScript.cs";
						for (int index = 2; std::filesystem::exists(path); ++index)
							path = activeDir / ("NewScript" + std::to_string(index) + ".cs");
						selectedAsset = Project::Get().getAssetManager().CreateCSharpScript(std::filesystem::absolute(path));
						if (!selectedAsset) MADAM_ERROR("Unable to create C# script: {0}", path.string());
					}
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Rename", nullptr, false, !popupContextSelectedItem.empty()))
				{
					renameTarget = popupContextSelectedItem;
					const std::string originalName = renameTarget.has_extension() && !std::filesystem::is_directory(renameTarget)
						? renameTarget.stem().u8string() : renameTarget.filename().u8string();
					renameName.fill('\0');
					std::memcpy(renameName.data(), originalName.data(),
						std::min(originalName.size(), renameName.size() - 1));
					renameError.clear();
					openRenamePopup = true;
				}
				if (!popupContextSelectedItem.empty())
				{
					if (ImGui::MenuItem("Delete", NULL, false, true))
					{
						if (DeleteDirectory(popupContextSelectedItem, "Delete selected asset?", popupContextSelectedItem.string() + "\n\nYou cannot undo the delete assets action."))
						{
							Project::Get().getAssetManager().RemoveMetadata(popupContextSelectedItem);
							Project::Get().getAssetManager().SaveMetaData();
						}
					}
				}
				else if (ImGui::MenuItem("Delete", NULL, false, (projectDir / ASSET_DIR) == activeDir ? false : true))
				{
					if (DeleteDirectory(activeDir, "Delete selected asset?", activeDir.string() + "\n\nYou cannot undo the delete assets action."))
					{
						Project::Get().getAssetManager().RemoveMetadata(activeDir);
						Project::Get().getAssetManager().SaveMetaData();
					}
				}
				ImGui::EndPopup();
			}
			else
			{
				popupContextSelectedItem = "";
				isPopupContextOpen = false;
			}
			if (openRenamePopup) {
				ImGui::OpenPopup("Rename Asset");
				openRenamePopup = false;
			}
			if (ImGui::BeginPopupModal("Rename Asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::TextUnformatted("Name");
				ImGui::SetNextItemWidth(320.0f);
				if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
				const bool submitted = ImGui::InputText("##RenameName", renameName.data(), renameName.size(),
					ImGuiInputTextFlags_EnterReturnsTrue);
				if (std::filesystem::is_regular_file(renameTarget)) {
					ImGui::SameLine();
					ImGui::TextDisabled("%s", renameTarget.extension().u8string().c_str());
				}
				if (!renameError.empty()) ImGui::TextWrapped("%s", renameError.c_str());
				if (submitted || ImGui::Button("Rename")) {
					const std::string desiredName(renameName.data());
					const bool directory = std::filesystem::is_directory(renameTarget);
					const bool validName = !desiredName.empty() && desiredName != "." && desiredName != ".." &&
						desiredName.find_first_of("\\/:*?\"<>|") == std::string::npos &&
						desiredName.back() != ' ' && desiredName.back() != '.';
					if (!validName) renameError = "Enter a valid file or folder name.";
					else {
						const auto extension = directory ? std::filesystem::path{} : renameTarget.extension();
						const auto destination = renameTarget.parent_path() /
							std::filesystem::u8path(desiredName + extension.u8string());
						if (destination == renameTarget) ImGui::CloseCurrentPopup();
						else if (std::filesystem::exists(destination)) renameError = "An item with that name already exists.";
						else {
							std::filesystem::path pairedSource;
							std::filesystem::path pairedDestination;
							if (extension == ".frag" || extension == ".vert") {
								pairedSource = renameTarget;
								pairedDestination = destination;
								const auto pairedExtension = extension == ".frag" ? ".vert" : ".frag";
								pairedSource.replace_extension(pairedExtension);
								pairedDestination.replace_extension(pairedExtension);
								if (!std::filesystem::exists(pairedSource)) pairedSource.clear();
							}
							if (!pairedSource.empty() && std::filesystem::exists(pairedDestination)) {
								renameError = "The matching shader file already has that name.";
							}
							else {
							std::error_code error;
							std::filesystem::rename(renameTarget, destination, error);
							if (!error && !pairedSource.empty()) {
								std::filesystem::rename(pairedSource, pairedDestination, error);
								if (error) {
									std::error_code rollbackError;
									std::filesystem::rename(destination, renameTarget, rollbackError);
								}
							}
							if (error) renameError = error.message();
							else {
								auto& manager = Project::Get().getAssetManager();
								manager.MoveMetadata(renameTarget, destination);
								if (!pairedSource.empty()) manager.MoveMetadata(pairedSource, pairedDestination);
								if (!manager.SaveMetaData())
									MADAM_ERROR("Asset registry could not be saved after rename");
								selectedAsset.reset();
								ImGui::CloseCurrentPopup();
							}
							}
						}
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}

	void GUI::Console() {
		if (ImGui::Begin("Console")) {
			static ImGuiTextFilter filter;
			static bool autoScroll = true;
			if (ImGui::Button("Clear")) Logger::ClearConsoleMessages();
			ImGui::SameLine();
			ImGui::Checkbox("Auto-scroll", &autoScroll);
			filter.Draw("Filter", 220.0f);
			ImGui::Separator();

			if (ImGui::BeginChild("ConsoleMessages", ImVec2(0, 0), false,
				ImGuiWindowFlags_HorizontalScrollbar))
			{
				const bool wasAtBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;
				for (const auto& message : Logger::GetConsoleMessages())
				{
					if (!filter.PassFilter(message.text.c_str())) continue;
					if (message.level >= spdlog::level::err)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.40f, 0.40f, 1.0f));
					else if (message.level == spdlog::level::warn)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.76f, 0.35f, 1.0f));
					ImGui::TextUnformatted(message.text.c_str());
					if (message.level >= spdlog::level::err || message.level == spdlog::level::warn)
						ImGui::PopStyleColor();
				}
				if (autoScroll && wasAtBottom) ImGui::SetScrollHereY(1.0f);
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}

	void GUI::RenderingSettings() {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];
		bool isWindowOpened;
		if (ImGui::Begin("Render Settings", &isWindowOpened)) {

			const ImGuiTreeNodeFlags headerFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
			const ImGuiTreeNodeFlags settingListFlags = headerFlags | ImGuiTreeNodeFlags_Leaf;
			ImGui::Columns(2, "Settings");

			ImGui::Separator();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool renderPassSettings = ImGui::TreeNodeEx("Render Layers Settings", settingListFlags, "Render Layers");

			if (renderPassSettings)
			{
				ImGui::PopStyleVar();
				ImGui::TreePop();
				ImGui::NextColumn();
				//we need a renderpassInfo struct to get the name of the renderpass
				std::vector<VkRenderPass> renderpasses = Rendering::Renderer::Get().GetRenderPasses();
				std::vector<Ref<Rendering::RenderLayer>> renderLayers;
				for (size_t i = 0; i < renderpasses.size(); i++)
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
					bool open = ImGui::TreeNodeEx("RPSetting " + i, headerFlags, "RenderPass " + i);

					ImGui::PopStyleVar();

					if (open) {
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
						if (i == 0) //only temp until renderer is probably setup to handle multiple renderpasses with multiple pipelines
						{
							renderLayers = Application::Get().GetMasterRenderSystem().getRenderLayers();
							ImVec4 headerColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
							//ImVec4 headerHoveredColor = ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered];
							ImGui::PushStyleColor(ImGuiCol_HeaderHovered, headerColor);
							ImGui::PushFont(boldFont);
							ImGui::TreeNodeEx("Pipeline Start", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
							ImGui::PopFont();
							ImGui::PopStyleColor();
							for (size_t j = 0; j < renderLayers.size(); j++)
							{
								auto& renderLayer = renderLayers[j];
								DrawPipelineSettings(renderLayer, static_cast<int>(j));
							}
							ImGui::PushStyleColor(ImGuiCol_HeaderHovered, headerColor);
							ImGui::PushFont(boldFont);
							ImGui::TreeNodeEx("Pipeline End", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
							ImGui::PopFont();
							ImGui::PopStyleColor();
						}
						ImGui::PopStyleVar();
						ImGui::TreePop();
					}
				}

				auto button = [](const char* value) {
					if (ImGui::Button(value)) {
						return true;
					}
					else {
						return false;
					}
				};

				bool isUpHit = false;
				bool isDownHit = false;

				if (selectedPipeline.first != nullptr) {
					if (selectedPipeline.second > 0) {
						isUpHit = button("Up");
					}
					else {
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						isUpHit = button("Up");
						ImGui::PopItemFlag();
					}
					ImGui::SameLine();
					if (selectedPipeline.second < renderLayers.size() - 1) {
						isDownHit = button("Down");
					}
					else {
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						isDownHit = button("Down");
						ImGui::PopItemFlag();
					}

				}
				else {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					isUpHit = button("Up");
					ImGui::SameLine();
					isDownHit = button("Down");
					ImGui::PopItemFlag();
				}

				if (!isUpHit && !isDownHit) {

				}
				else if (isUpHit && !isDownHit) {
					Application::Get().GetMasterRenderSystem().switchRenderSystems(selectedPipeline.second, selectedPipeline.second - 1);
					selectedPipeline.second--;
				}
				else if (!isUpHit && isDownHit) {
					MADAM_CORE_INFO("Down has been hit");
					Application::Get().GetMasterRenderSystem().switchRenderSystems(selectedPipeline.second, selectedPipeline.second + 1);
					selectedPipeline.second++;
				}
				else {
					MADAM_CORE_ERROR("Both buttons hit at same time?!?!? wtf?");
				}

				if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
					selectedPipeline = { nullptr, -1 };
				}
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			}

			ImGui::PopStyleVar();
		}
		ImGui::End();
		if (!isWindowOpened) {
			windowStates &= ~RENDER_SETTINGS_WINDOW;
		}
	}

	void GUI::DebugWindow()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];
		bool isWindowOpened;
		if (ImGui::Begin("Debug Info Window", &isWindowOpened))
		{

			const ImGuiTreeNodeFlags headerFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
			const ImGuiTreeNodeFlags settingListFlags = headerFlags | ImGuiTreeNodeFlags_Leaf;
			ImGui::Columns(2, "Settings");

			ImGui::Separator();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool project_info = ImGui::TreeNodeEx("Project Info", settingListFlags, "Project Information");

			if (project_info)
			{

				ImGui::PopStyleVar();
				ImGui::TreePop();
				ImGui::NextColumn();
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
				std::string text = "Project Directory: " + Application::Get().GetConfig().projectsDirectory.string();
				ImGui::Text(text.c_str());
				text = "Current Working Directory: " + std::filesystem::current_path().string();
				ImGui::Text(text.c_str());
				ImGui::PopStyleVar();
			}
				
		}
		ImGui::End();
		if (!isWindowOpened)
		{
			windowStates &= ~DEBUG_INFO_WINDOW;
		}
	}

	void GUI::DrawEntityNode(Entity entity) {
		auto name = entity.GetComponent<CMetadata>().name;

		ImGuiTreeNodeFlags flags = ((selectedEntity != nullptr && *selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0 ) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());
		if (ImGui::IsItemClicked()) {
			selectedEntity = CreateRef<Entity>(entity);
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Create Empty")) {
				auto childEntity = Application::Get().GetScene().CreateEntity();
				Application::Get().GetScene().AddEntityRelationship(entity, childEntity);
			}
			if (ImGui::BeginMenu("3D")) {
				if (ImGui::MenuItem("Quad"))
				{
					auto childEntity = Application::Get().GetScene().CreateEntity("Quad");
					childEntity.AddComponent<CMeshRenderer>();
					childEntity.GetComponent<CMeshRenderer>().mesh = StaticMesh::Create(MeshPrimatives::Quad);
					Application::Get().GetScene().AddEntityRelationship(entity, childEntity);
				}
				if (ImGui::MenuItem("Cube"))
				{
					auto childEntity = Application::Get().GetScene().CreateEntity("Cube");
					childEntity.AddComponent<CMeshRenderer>();
					childEntity.GetComponent<CMeshRenderer>().mesh = StaticMesh::Create(MeshPrimatives::Cube);
					Application::Get().GetScene().AddEntityRelationship(entity, childEntity);
				}
				if (ImGui::MenuItem("Sphere"))
				{
					auto childEntity = Application::Get().GetScene().CreateEntity("Sphere");
					childEntity.AddComponent<CMeshRenderer>();
					childEntity.GetComponent<CMeshRenderer>().mesh = StaticMesh::Create(MeshPrimatives::Sphere);
					Application::Get().GetScene().AddEntityRelationship(entity, childEntity);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Lights"))
			{
				if (ImGui::MenuItem("Point Light"))
				{
					auto childEntity = Application::Get().GetScene().CreateEntity("Point Light");
					childEntity.AddComponent<CPointLight>();
					Application::Get().GetScene().AddEntityRelationship(entity, childEntity);
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Delete Entity")) {
				entityDeleted = true;
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ITEM"))
			{
				UUID uuid = *(UUID*)payload->Data;
				Entity other = Application::Get().GetScene().GetEntity(uuid);
				if (other != null)
				{
					newParentEntityUUID = entity.GetComponent<CUniqueIdentifier>().uuid;
					newChildEntityUUID = uuid;
					hasEntityRelationshipChanged = true;
				}
				else
				{
					MADAM_CORE_ERROR("Entity {0} is NULL", uuid);
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			UUID entityUUID = entity.GetComponent<CUniqueIdentifier>().uuid;
			ImGui::SetDragDropPayload("ENTITY_ITEM", &entity.GetComponent<CUniqueIdentifier>().uuid, sizeof(UUID));
			ImGui::EndDragDropSource();
		}

		if (opened) {
			if (entity.HasComponent<CRelationship>() && entity.GetComponent<CMetadata>().is_hidden_entity != true)
			{
				for each (UUID child in entity.GetComponent<CRelationship>().children) {
					Entity childEntity = Application::Get().GetScene().GetEntity(child);
					DrawEntityNode(childEntity);
				}
			}
			ImGui::TreePop();
		}

		if (entityDeleted) {
			if (selectedEntity && *selectedEntity == entity) {
				selectedEntity = nullptr;
			}
			pendingEntityDeletion = CreateRef<Entity>(entity);
		}
	}

	void GUI::DrawEntityComponents(Entity entity) {
		if (entity.HasComponent<CMetadata>()) {
			auto& gameObject = entity.GetComponent<CMetadata>();

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), gameObject.name.c_str());

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			if (ImGui::InputText("##Name", buffer, sizeof(buffer))) {
				entity.GetComponent<CMetadata>().name = std::string(buffer);
			}
			ImGui::PopStyleVar();
		}

		const ImGuiTreeNodeFlags headerFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;


		if (entity.HasComponent<CTransform>()) {

			auto& transform = entity.GetComponent<CTransform>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = ImGui::TreeNodeEx((void*)typeid(CTransform).hash_code(), headerFlags, "Transform");
			ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);

			float lineHeight = ImGui::GetFontSize() + 8;
			if (ImGui::Button("...", ImVec2{ 20, lineHeight })) {
				ImGui::OpenPopup("Component Settings");
			}
			ImGui::PopStyleVar();

			if (ImGui::BeginPopup("Component Settings")) {
				ImGui::EndPopup();
			}

			if (open) {
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
				DrawVec3("Translation", transform.translation);
				glm::vec3 rotation = glm::degrees(glm::eulerAngles(transform.rotation));
				DrawVec3("Rotation", rotation);
				if (glm::degrees(glm::eulerAngles(transform.rotation)) != rotation)
				{
					transform.rotation = glm::quat(rotation);
				}
				DrawVec3("Scale", transform.scale);
				ImGui::PopStyleVar();
				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<CCamera>()) {
			auto& camera = entity.GetComponent<CCamera>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = ImGui::TreeNodeEx((void*)typeid(CCamera).hash_code(), headerFlags, "Camera");
			ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);

			float lineHeight = ImGui::GetFontSize() + 8;
			if (ImGui::Button("...", ImVec2{ 20, lineHeight })) {
				ImGui::OpenPopup("Component Settings");
			}
			ImGui::PopStyleVar();

			bool removeComponent = false;
			if (ImGui::BeginPopup("Component Settings")) {
				if (ImGui::MenuItem("Remove Component")) {
					removeComponent = true;
				}
				ImGui::EndPopup();
			}

			if (open) {
				//ImGui::Checkbox("Primary", &camera.primary);
				const char* items[] = { "None", "Perspective", "Orthographic"};
				Rendering::CameraData& cameraData = camera.cameraHandle->GetCameraData();
				int currentItem = (int)cameraData.projectionType;
				if (ImGui::BeginCombo("Projection", items[currentItem])) {
					for (int i = 0; i < 3; i++) {
						bool isSelected = currentItem == i;
						if (ImGui::Selectable(items[i], isSelected)) {
							currentItem = i;
							cameraData.projectionType = (Rendering::CameraData::ProjectionType)i;
						}
						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				switch (currentItem)
				{
				case 0:
					break;
				case 1:
					ImGui::DragFloat("Field of View", &cameraData.perspective.fovy, glm::radians(50.0f));
					ImGui::DragFloat("Near Clip", &cameraData.perspective.near, 0.1f);
					ImGui::DragFloat("Far Clip", &cameraData.perspective.far, 100.0f);
					break;
				case 2:
					ImGui::DragFloat("Size", &cameraData.orthographic.size, 50.0f);
					ImGui::DragFloat("Near Clip", &cameraData.orthographic.near, 0.1f);
					ImGui::DragFloat("Far Clip", &cameraData.orthographic.far, 100.0f);
					break;
				default:
					break;
				}
				ImGui::TreePop();
			}

			if (removeComponent) {
				entity.RemoveComponent<CCamera>();
			}
		}

		if (entity.HasComponent<CMeshRenderer>()) {
			auto& meshRenderer = entity.GetComponent<CMeshRenderer>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = ImGui::TreeNodeEx((void*)typeid(CMeshRenderer).hash_code(), headerFlags, "Mesh Renderer");
			ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);

			float lineHeight = ImGui::GetFontSize() + 8;
			if (ImGui::Button("...", ImVec2{ 20, lineHeight })) {
				ImGui::OpenPopup("Component Settings");
			}
			ImGui::PopStyleVar();

			bool removeComponent = false;
			if (ImGui::BeginPopup("Component Settings")) {
				if (ImGui::MenuItem("Remove Component")) {
					removeComponent = true;
				}
				ImGui::EndPopup();
			}

			if (open) {
				DrawAssetReference("Material", meshRenderer.materialAsset, AssetType::MATERIAL, [&](UUID selected)
				{
					meshRenderer.materialAsset = selected;
					auto asset = std::dynamic_pointer_cast<MaterialAsset>(Project::Get().getAssetManager().GetAsset(selected));
					meshRenderer.material = asset ? asset->GetRuntimeMaterial(Project::Get().getAssetManager()) : nullptr;
				});
				ImGui::TreePop();
			}

			if (removeComponent) {
				entity.RemoveComponent<CMeshRenderer>();
			}
		}

		if (entity.HasComponent<CCSharpScript>()) {
			auto& script = entity.GetComponent<CCSharpScript>();
			bool open = ImGui::TreeNodeEx((void*)typeid(CCSharpScript).hash_code(), headerFlags, "C# Script");
			ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
			if (ImGui::Button("...", ImVec2{ 20, ImGui::GetFontSize() + 8 }))
				ImGui::OpenPopup("CSharp Settings");
			bool removeComponent = false;
			if (ImGui::BeginPopup("CSharp Settings")) {
				removeComponent = ImGui::MenuItem("Remove Component");
				ImGui::EndPopup();
			}
			if (open) {
				DrawAssetReference("Script", script.script, AssetType::SCRIPT, [&](UUID selected)
				{
					if (selected != null && Project::Get().getAssetManager().ResolveAssetPath(selected).extension() != ".cs")
					{
						MADAM_ERROR("Select a C# (.cs) script for this component");
						return;
					}
					script.script = selected;
					script.className.clear();
				});
				char className[256]{};
				strncpy_s(className, script.className.c_str(), _TRUNCATE);
				if (ImGui::InputTextWithHint("Class", "Filename by default", className, sizeof(className)))
					script.className = className;
				ImGui::TreePop();
			}
			if (removeComponent) entity.RemoveComponent<CCSharpScript>();
		}

		if (entity.HasComponent<CPointLight>()) {
			auto& pointLight = entity.GetComponent<CPointLight>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = ImGui::TreeNodeEx((void*)typeid(CPointLight).hash_code(), headerFlags, "Point Light");
			ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);

			float lineHeight = ImGui::GetFontSize() + 8;
			if (ImGui::Button("...", ImVec2{ 20, lineHeight })) {
				ImGui::OpenPopup("Component Settings");
			}
			ImGui::PopStyleVar();

			bool removeComponent = false;
			if (ImGui::BeginPopup("Component Settings")) {
				if (ImGui::MenuItem("Remove Component")) {
					removeComponent = true;
				}
				ImGui::EndPopup();
			}

			if (open) {
				ImGui::ColorEdit3("Color", glm::value_ptr(pointLight.color));
				ImGui::DragFloat("Intensity", &pointLight.intensity, 0.1f, 0.0f, 100.0f);
				ImGui::DragFloat("Radius", &pointLight.radius, 0.1f, 0.0f, 100.0f);

				ImGui::TreePop();
			}

			if (removeComponent) {
				entity.RemoveComponent<CPointLight>();
			}
		}

		if (entity.HasComponent<CMaterial>()) {
			auto& material = entity.GetComponent<CMaterial>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = ImGui::TreeNodeEx((void*)typeid(CMaterial).hash_code(), headerFlags, "Material");
			ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);

			float lineHeight = ImGui::GetFontSize() + 8;
			if (ImGui::Button("...", ImVec2{ 20, lineHeight })) {
				ImGui::OpenPopup("Component Settings");
			}
			ImGui::PopStyleVar();

			bool removeComponent = false;
			if (ImGui::BeginPopup("Component Settings")) {
				if (ImGui::MenuItem("Remove Component")) {
					removeComponent = true;
				}
				ImGui::EndPopup();
			}

			if (open) {
				ImGui::Checkbox("custom", &material.is_custom);
				ImGui::DragFloat("ax", &material.ax, 0.1f, -10.0f, 10.0f);
				ImGui::DragFloat("dx0", &material.dx0, 0.1f, -10.0f, 10.0f);
				ImGui::DragFloat("dx1", &material.dx1, 0.1f, -10.0f, 10.0f);
				ImGui::DragFloat("dx2", &material.dx2, 0.1f, -10.0f, 10.0f);
				ImGui::DragFloat("dx3", &material.dx3, 0.1f, -10.0f, 10.0f);
				if (ImGui::Button("Reload Shader", ImVec2{ 100, lineHeight }))
				{
					Application::Get().ReloadShaders();
				}
				ImGui::TreePop();
			}

			if (removeComponent) {
				entity.RemoveComponent<CMaterial>();
			}
		}
	}

	void GUI::DrawMaterialThumbnail(UUID handle, const char* buttonId, ImVec2 size)
	{
		ImGui::InvisibleButton(buttonId, size);
		const ImVec2 topLeft = ImGui::GetItemRectMin();
		const ImVec2 center(topLeft.x + size.x * 0.5f, topLeft.y + size.y * 0.49f);
		const float radius = std::max(4.0f, std::min(size.x, size.y) * 0.42f);
		auto* draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(topLeft, ImVec2(topLeft.x + size.x, topLeft.y + size.y),
			IM_COL32(31, 34, 43, 255), 4.0f);
		draw->AddCircleFilled(ImVec2(center.x + radius * 0.08f, center.y + radius * 0.12f),
			radius, IM_COL32(12, 14, 18, 180), 24);

		ImTextureID textureId = ImGui::GetIO().Fonts->TexID;
		bool hasPreviewTexture = false;
		auto& manager = Project::Get().getAssetManager();
		auto material = std::dynamic_pointer_cast<MaterialAsset>(manager.GetAsset(handle));
		if (material)
		{
			UUID previewTexture("");
			const auto diffuse = material->textures.find("diffuseMap");
			if (diffuse != material->textures.end())
				previewTexture = diffuse->second;
			else
				for (const auto& [name, texture] : material->textures)
					if (texture != null) { previewTexture = texture; break; }
			if (previewTexture != null)
			{
				IconInfo& icon = loadedIconTextures[previewTexture];
				if (!icon.texture)
				{
					icon.texture = std::dynamic_pointer_cast<Texture>(manager.GetAsset(previewTexture));
					if (icon.texture)
					{
						auto vkTexture = std::static_pointer_cast<VulkanTexture>(icon.texture);
						icon.descriptorSet = ImGui_ImplVulkan_AddTexture(vkTexture->GetSampler(),
							vkTexture->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					}
				}
					if (icon.descriptorSet)
					{
					textureId = icon.descriptorSet;
						hasPreviewTexture = true;
					}
			}
		}

		const auto point = [&](float longitude, float latitude)
		{
			const float x = std::cos(latitude) * std::sin(longitude);
			const float y = std::sin(latitude);
			const float z = std::cos(latitude) * std::cos(longitude);
			const float light = std::clamp(0.38f + 0.58f * (-0.38f * x + 0.55f * y + 0.74f * z), 0.16f, 1.0f);
			const int shade = static_cast<int>(light * 255.0f);
			return std::tuple<ImVec2, ImVec2, ImU32>{
				ImVec2(center.x + radius * x, center.y - radius * y),
				hasPreviewTexture
					? ImVec2(0.5f + longitude / (2.0f * 3.14159265f), 0.5f - latitude / 3.14159265f)
					: ImGui::GetFontTexUvWhitePixel(),
				IM_COL32(shade, shade, shade, 255) };
		};
		draw->PushTextureID(textureId);
		constexpr int steps = 12;
		for (int row = 0; row < steps; ++row)
			for (int column = 0; column < steps; ++column)
			{
				const float a = -1.57079633f + 3.14159265f * column / steps;
				const float b = -1.57079633f + 3.14159265f * (column + 1) / steps;
				const float c = -1.57079633f + 3.14159265f * row / steps;
				const float d = -1.57079633f + 3.14159265f * (row + 1) / steps;
				const auto p0 = point(a, c), p1 = point(b, c), p2 = point(b, d), p3 = point(a, d);
				const ImDrawIdx base = static_cast<ImDrawIdx>(draw->_VtxCurrentIdx);
				draw->PrimReserve(6, 4);
				draw->PrimWriteIdx(base); draw->PrimWriteIdx(base + 1); draw->PrimWriteIdx(base + 2);
				draw->PrimWriteIdx(base); draw->PrimWriteIdx(base + 2); draw->PrimWriteIdx(base + 3);
				draw->PrimWriteVtx(std::get<0>(p0), std::get<1>(p0), std::get<2>(p0));
				draw->PrimWriteVtx(std::get<0>(p1), std::get<1>(p1), std::get<2>(p1));
				draw->PrimWriteVtx(std::get<0>(p2), std::get<1>(p2), std::get<2>(p2));
				draw->PrimWriteVtx(std::get<0>(p3), std::get<1>(p3), std::get<2>(p3));
			}
		draw->PopTextureID();
		draw->AddCircle(center, radius, IM_COL32(165, 174, 190, 130), 30, 1.0f);
		if (ImGui::IsItemHovered())
			draw->AddRect(topLeft, ImVec2(topLeft.x + size.x, topLeft.y + size.y),
				IM_COL32(170, 184, 225, 255), 4.0f);
	}

	void GUI::DrawAssetReference(const char* label, UUID current, AssetType type,
		const std::function<void(UUID)>& onSelect)
	{
		auto& manager = Project::Get().getAssetManager();
		ImGui::PushID(label);
		const auto path = current == null ? std::filesystem::path{} : manager.ResolveAssetPath(current);
		const std::string name = current == null ? "None" :
			(path.empty() ? "Missing" : path.filename().string());
		ImGui::TextUnformatted(label);
		ImGui::SameLine(145.0f);
		if (ImGui::Button(name.c_str(), ImVec2(190.0f, 0.0f)))
		{
			assetPickerSearch[0] = '\0';
			ImGui::OpenPopup("Select Asset");
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PROJECT_ITEM"))
			{
				const std::filesystem::path droppedPath((const wchar_t*)payload->Data);
				const auto& dropped = manager.GetMetadata(droppedPath);
				if (dropped.assetType == type)
					onSelect(dropped.uuid);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SetNextWindowSize(ImVec2(500.0f, 460.0f), ImGuiCond_Appearing);
		if (ImGui::BeginPopupModal("Select Asset", nullptr, ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::Text("Choose %s", assetTypeToString(type).c_str());
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::InputTextWithHint("##Search", "Search assets...", assetPickerSearch, sizeof(assetPickerSearch));
			if (ImGui::Selectable("None"))
			{
				onSelect(UUID(""));
				ImGui::CloseCurrentPopup();
			}
			ImGui::Separator();
			if (ImGui::BeginChild("AssetList", ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing()), true))
			{
				auto assets = manager.ListAssets(type);
				const auto editorRoot = std::filesystem::absolute("resources");
				const auto projectRoot = std::filesystem::absolute(Project::Get().getProjectDirectory());
				const auto displayPath = [&](const AssetReference& item)
				{
					return item.path.lexically_relative(item.editorResource ? editorRoot : projectRoot).generic_string();
				};
				std::string search(assetPickerSearch);
				std::transform(search.begin(), search.end(), search.begin(),
					[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				assets.erase(std::remove_if(assets.begin(), assets.end(), [&](const AssetReference& item)
				{
					if (type == AssetType::SCRIPT && item.path.extension() != ".cs") return true;
					std::string name = displayPath(item);
					std::transform(name.begin(), name.end(), name.begin(),
						[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
					return name.find(search) == std::string::npos;
				}), assets.end());
				ImGuiListClipper clipper;
				clipper.Begin(static_cast<int>(assets.size()), 42.0f);
				while (clipper.Step())
				{
					for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
					{
						const auto& item = assets[index];
						ImTextureID icon = icons["File.png"].descriptorSet;
						if (type == AssetType::TEXTURE)
						{
							IconInfo& thumbnail = loadedIconTextures[item.handle];
							if (!thumbnail.texture)
							{
								thumbnail.texture = std::dynamic_pointer_cast<Texture>(manager.GetAsset(item.handle));
								if (thumbnail.texture)
								{
									auto vkTexture = std::static_pointer_cast<VulkanTexture>(thumbnail.texture);
									thumbnail.descriptorSet = ImGui_ImplVulkan_AddTexture(vkTexture->GetSampler(),
										vkTexture->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
								}
							}
							if (thumbnail.descriptorSet)
								icon = thumbnail.descriptorSet;
						}
						bool thumbnailSelected = false;
						if (type == AssetType::MATERIAL)
						{
							const std::string thumbnailId = "Thumbnail##" + (std::string)item.handle;
							DrawMaterialThumbnail(item.handle, thumbnailId.c_str(), ImVec2(32.0f, 32.0f));
							thumbnailSelected = ImGui::IsItemClicked();
						}
						else
							ImGui::Image(icon, ImVec2(32.0f, 32.0f));
						ImGui::SameLine();
						const std::string row = displayPath(item) +
							(item.editorResource ? "  [Editor]" : "  [Project]") + "##" + (std::string)item.handle;
						if (ImGui::Selectable(row.c_str(), false, 0, ImVec2(0.0f, 32.0f)) || thumbnailSelected)
						{
							onSelect(item.handle);
							ImGui::CloseCurrentPopup();
						}
					}
				}
			}
			ImGui::EndChild();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}

	void GUI::DrawAssetInfo(Ref<Asset>& asset)
	{
		if (asset->GetAssetType() == AssetType::SHADER)
		{
			if (shaderCompileAsset != asset->handle)
			{
				shaderCompileAsset = asset->handle;
				shaderCompileFeedback.clear();
			}
			auto& manager = Project::Get().getAssetManager();
			const auto path = manager.ResolveAssetPath(asset->handle);
			ImGui::TextWrapped("%s", path.string().c_str());
			if (path.extension() == ".frag" && manager.GetMetadata(asset->handle).IsValid())
			{
				if (ImGui::Button("Compile Shader"))
				{
					if (manager.CompileShader(asset->handle, &shaderCompileFeedback))
						Application::Get().ReloadShaders();
					else
						MADAM_ERROR("Shader compilation failed: {0}", shaderCompileFeedback);
				}
				if (!shaderCompileFeedback.empty())
					ImGui::TextWrapped("%s", shaderCompileFeedback.c_str());
			}
			return;
		}
		if (asset->GetAssetType() == AssetType::SCRIPT)
		{
			const auto path = Project::Get().getAssetManager().ResolveAssetPath(asset->handle);
			ImGui::TextWrapped("%s", path.string().c_str());
			ImGui::TextUnformatted("C# scripts compile when you press Play.");
			return;
		}
		if (asset->GetAssetType() == AssetType::MATERIAL)
		{
			auto material = std::dynamic_pointer_cast<MaterialAsset>(asset);
			if (!material)
				return;
			auto& manager = Project::Get().getAssetManager();
			bool changed = false;
			DrawAssetReference("Shader", material->shader, AssetType::SHADER, [&](UUID selected)
			{
				if (selected == null)
					selected = UUID("editor://shaders/texture_shader.frag");
				if (selected == material->shader)
					return;
				const auto definition = manager.GetShaderDefinition(selected);
				if (!definition.IsValid())
				{
					MADAM_ERROR("Cannot use shader: {0}", definition.error);
					return;
				}
				if (manager.GetMetadata(selected).IsValid() &&
					!manager.CompileShader(selected, &shaderCompileFeedback))
				{
					MADAM_ERROR("Cannot use shader: {0}", shaderCompileFeedback);
					return;
				}
				material->shader = selected;
				Application::Get().ReloadShaders();
				changed = true;
			});
			const auto definition = manager.GetShaderDefinition(material->shader);
			if (!definition.IsValid())
				ImGui::TextWrapped("%s", definition.error.c_str());
			else
			{
				for (const auto& input : definition.textures)
				{
					const auto found = material->textures.find(input.name);
					const UUID selected = found == material->textures.end() ? UUID("") : found->second;
					DrawAssetReference(input.name.c_str(), selected, AssetType::TEXTURE,
						[&, name = input.name](UUID texture)
						{
							material->textures[name] = texture;
							changed = true;
						});
				}
			}
			if (changed)
			{
				if (!manager.SaveAsset(asset))
					MADAM_ERROR("Unable to save material asset");
				material->GetRuntimeMaterial(manager);
			}
			return;
		}
		if (asset->GetAssetType() == AssetType::TEXTURE)
		{
			Ref<Texture> texture = std::dynamic_pointer_cast<Texture>(asset);
			
			auto format = texture->GetFormat();
			if (format == Rendering::Image::Format::RGBA_SRGB)
			{
				isSRGB = true;
			}
			else 
			{
				isSRGB = false;
			}
			if (ImGui::Checkbox("SRGB", &isSRGB))
			{

			}
			std::string text = "Size: " + std::to_string(texture->GetWidth()) + " x " + std::to_string(texture->GetHeight());
			ImGui::Text(text.c_str());
			text = "Depth: " + std::to_string(texture->GetDepth());
			ImGui::Text(text.c_str());
		}

	}

	void GUI::DrawVec3(const std::string& label, glm::vec3& values, float resetValue, float columnWidth) {

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 2, 4 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	void GUI::DrawViewportOverlays()
	{
		const ImVec2 restore = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos(ImVec2(viewportImageTopLeft.x + viewportImageSize.x * 0.5f - 45.0f,
			viewportImageTopLeft.y + 12.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, RGBCon(30, 43, 58));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, RGBCon(57, 107, 130));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, RGBCon(71, 146, 173));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
		ImGui::BeginGroup();
		if (ImGui::ImageButton("##Play", icons["PlayButton.png"].descriptorSet, ImVec2(22, 22)))
		{
			if (!Application::Get().IsPlay()) Application::Get().SetRuntimeFlag();
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Play scene");
		ImGui::SameLine();
		ImGui::BeginDisabled();
		ImGui::ImageButton("##Pause", icons["PauseButton.png"].descriptorSet, ImVec2(22, 22));
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::ImageButton("##Stop", icons["StopButton.png"].descriptorSet, ImVec2(22, 22)))
		{
			if (Application::Get().IsPlay()) Application::Get().SetRuntimeStopFlag();
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Stop scene");
		ImGui::EndGroup();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3);
		ImGui::SetCursorScreenPos(restore);
	}

	void GUI::DrawViewportGizmoButtons() {
		const ImVec2 restore = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos(ImVec2(viewportImageTopLeft.x + 12.0f,
			viewportImageTopLeft.y + 12.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, RGBCon(30, 43, 58));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, RGBCon(57, 107, 130));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, RGBCon(71, 146, 173));
		ImGui::BeginGroup();
		const char* labels[] = { "Move", "Rotate", "Scale" };
		const int modes[] = { ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE };
		for (int index = 0; index < 3; ++index)
		{
			if (index) ImGui::SameLine();
			const bool wasSelected = gizmoButtonStates[index];
			if (wasSelected)
				ImGui::PushStyleColor(ImGuiCol_Button, RGBCon(53, 116, 143));
			if (ImGui::Button(labels[index]))
			{
				gizmoButtonStates = { false, false, false };
				if (!wasSelected) gizmoButtonStates[index] = true;
				ImGuizmoType = wasSelected ? 0 : modes[index];
			}
			if (wasSelected) ImGui::PopStyleColor();
		}
		ImGui::EndGroup();
		ImGui::PopStyleColor(3);
		ImGui::SetCursorScreenPos(restore);
	}

	void GUI::DrawPipelineSettings(const Ref<Rendering::RenderLayer> pipeline, int index) {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ((selectedPipeline.first != nullptr && selectedPipeline.first == pipeline) ? ImGuiTreeNodeFlags_Selected : 0);
		bool opened = ImGui::TreeNodeEx(pipeline->name.c_str(), flags, pipeline->name.c_str());
		if (ImGui::IsItemClicked()) {
			selectedPipeline = std::make_pair(pipeline, index);
		}
		if (opened) {
			ImGui::TreePop();
		}
	}

	void GUI::CreateViewportPipeline() {
		PipelineConfigInfo configInfo{};
		configInfo.renderPass = Rendering::Renderer::Get().GetSwapChainRenderPass();
		std::vector< VkVertexInputBindingDescription> bindingDescriptions;
		bindingDescriptions.resize(1);
		bindingDescriptions[0].stride = sizeof(ImDrawVert);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.resize(3);
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].binding = bindingDescriptions[0].binding;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(ImDrawVert, pos);
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].binding = bindingDescriptions[0].binding;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(ImDrawVert, uv);
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].binding = bindingDescriptions[0].binding;
		attributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDescriptions[2].offset = offsetof(ImDrawVert, col);
		
		configInfo.bindingDescriptions = bindingDescriptions;
		configInfo.attributeDescriptions = attributeDescriptions;
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		configInfo.enableAlphaBlending();
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

		ImGui_ImplVulkan_Data* bd = ImGui::GetCurrentContext() ? (ImGui_ImplVulkan_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
		if (bd == nullptr)
		{
			MADAM_CORE_ERROR("GUI: Trying to dereference a nullptr");
		}
		ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;

		VkPushConstantRange push_constants[1] = {};
		push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		push_constants[0].offset = sizeof(float) * 0;
		push_constants[0].size = sizeof(float) * 4;
		VkDescriptorSetLayout set_layout[1] = { bd->DescriptorSetLayout };
		VkPipelineLayoutCreateInfo layout_info = {};
		layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layout_info.setLayoutCount = 1;
		layout_info.pSetLayouts = set_layout;
		layout_info.pushConstantRangeCount = 1;
		layout_info.pPushConstantRanges = push_constants;
		if (vkCreatePipelineLayout(Rendering::Renderer::GetDevice().device(), &layout_info, v->Allocator, &configInfo.pipelineLayout) != VK_SUCCESS) {
			MADAM_CORE_ERROR("Failed to create pipeline layout!");
		}
		Device& device = Rendering::Renderer::GetDevice();

		ShaderStageConfigInfo shaderStageConfigInfo;
		shaderStageConfigInfo.vertexModule = ShaderModuleConfigInfo(vertCode);
		shaderStageConfigInfo.fragmentModule = ShaderModuleConfigInfo(fragCode);

		viewportPipelineInfo.pipeline = CreateRef<Pipeline>(device, configInfo, shaderStageConfigInfo);
		viewportPipelineInfo.layout = configInfo.pipelineLayout;
	}

	/*Ref<Asset>& GUI::AssetReference(const AssetType filter)
	{
		switch (filter)
		{
			case AssetType::TEXTURE:

				break;
			case AssetType::MESH:

				break;
			default:
				break;
		}
	}*/

	std::string GUI::TruncateTextToFit(const std::string & text, float maxWidth)
	{
		if (ImGui::CalcTextSize(text.c_str()).x <= maxWidth) {
			return text;
		}

		std::string truncatedText = text;
		while (!truncatedText.empty() && ImGui::CalcTextSize((truncatedText + "...").c_str()).x > maxWidth) {
			truncatedText.pop_back();
		}

		return truncatedText + "...";
	}
}
