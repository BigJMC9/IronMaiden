#include "maidenpch.hpp"
#include "H_GUI.hpp"
#include "../Events/H_Input.hpp"
#include "../Scene/H_Entity.hpp"
#include "../Core/H_Application.hpp"
#include "../Scene/H_SceneSerializer.hpp"
#include "../Rendering/H_Pipeline.hpp"

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
		VkCommandBuffer cmdBuffer = Rendering::Renderer::Get().getCurrentCommandBuffer();
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

	GUI::~GUI() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void GUI::OnAttach() {
		Device& device = Rendering::Renderer::GetDevice();
		guiPool =
			DescriptorPool::Builder(device)
			.setMaxSets(1000) //Maximun number of frames being rendered
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

		/*viewportLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.buildRef();*/

		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		Style(io); 

		init_info = &Rendering::Renderer::Get().getImGuiInitInfo();
		init_info->DescriptorPool = guiPool.get()->descriptorPool;
		init_info->MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info->RenderPass = Rendering::Renderer::Get().getSwapChainRenderPass();

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

		ImGui_ImplGlfw_InitForVulkan(Application::Get().getWindow().getGLFWwindow(), true);
		ImGui_ImplVulkan_Init(init_info);
		//ImGui_ImplVulkan_Data* bd = ImGui::GetCurrentContext() ? (ImGui_ImplVulkan_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;

		//viewport descriptors
		viewportSet = ImGui_ImplVulkan_AddTexture(viewportSampler, Rendering::Renderer::Get().getImageView(0), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		ImGui_ImplVulkan_CreateFontsTexture();
		viewportCallback = ImDrawCallback(DrawViewport);
		CreateViewportPipeline();
		gizmoButtonStates = { false, false, false};

		SetUpEvents();
	}

	void GUI::SetUpEvents() {
		EventSystem::Get().AddListener(this, &GUI::OnResizeEvent);
		EventSystem::Get().AddListener(this, &GUI::OnRenderPassEvent);
		EventSystem::Get().AddListener(this, &GUI::OnSceneChangeEvent);
	}

	void GUI::OnSceneChangeEvent(SceneChangeEvent* e) {
		selectedEntity = nullptr;
	}

	void GUI::OnRenderPassEvent(NextRenderPassEvent* e) {
		if (e->renderpassIndex == 1) {
			OnRender();
		}
	}

	void GUI::OnResizeEvent(WindowResizeEvent* e) {
	
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplVulkan_Shutdown();
		ImGui::DestroyContext();
		fonts.clear();
		OnAttach();
	}

	void GUI::OnDetach() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		fonts.clear();
	}

	void GUI::Style(ImGuiIO& io) {
		//Fonts
		ImFont* font = io.Fonts->AddFontFromFileTTF("Internal/fonts/Roboto-Medium.ttf", 16.0f);
		fonts.push_back(font);
		font = io.Fonts->AddFontFromFileTTF("Internal/fonts/Roboto-Bold.ttf", 16.0f);
		fonts.push_back(font);
		style = ImGui::GetStyle();
		//Windows
		style.Colors[ImGuiCol_WindowBg] = RGBCon(17, 17, 21);
		style.Colors[ImGuiCol_Border] = RGBCon(25, 31, 36);

		//Titles
		style.Colors[ImGuiCol_TitleBg] = RGBCon(21, 19, 25);
		style.Colors[ImGuiCol_TitleBgActive] = RGBCon(35, 33, 39);
		//style.Colors[ImGuiCol_TitleBgCollapsed] = RGBCon(21, 19, 25);

		style.Colors[ImGuiCol_MenuBarBg] = RGBCon(27, 22, 31);
		//Tabs
		style.Colors[ImGuiCol_Tab] = RGBCon(48, 111, 112);
		style.Colors[ImGuiCol_TabHovered] = RGBCon(47, 121, 125);
		style.Colors[ImGuiCol_TabActive] = RGBCon(57, 169, 113);
		style.Colors[ImGuiCol_TabUnfocused] = RGBCon(37, 60, 68);
		style.Colors[ImGuiCol_TabUnfocusedActive] = RGBCon(38, 103, 104);

		style.Colors[ImGuiCol_Button] = RGBCon(37, 98, 82);
		style.Colors[ImGuiCol_ButtonHovered] = RGBCon(36, 119, 83);
		style.Colors[ImGuiCol_ButtonActive] = RGBCon(36, 136, 78);

		style.Colors[ImGuiCol_Header] = RGBCon(37, 98, 82);
		style.Colors[ImGuiCol_HeaderHovered] = RGBCon(36, 119, 83);
		style.Colors[ImGuiCol_HeaderActive] = RGBCon(36, 136, 78);

		ImGui::GetStyle() = style;
	}

	void GUI::OnUpdate() {
		if (*pendingEntityDeletion) {
			Application::Get().getScene().DestroyEntity(*pendingEntityDeletion);
			pendingEntityDeletion = CreateRef<Entity>(Entity());
		}

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)Application::Get().getWindow().getWidth(), (float)Application::Get().getWindow().getHeight());

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		EditorUI();
	}

	void GUI::OnRender() {

		ImGui::Render();

		VkCommandBuffer commandBuffer = Rendering::Renderer::Get().getCurrentCommandBuffer();
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
		Viewport();
		Hierarchy();
		Inspector();
		Project();
		Console();
		ImGui::PopFont();
	}

	void GUI::MenuBar() {
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Scene")) {}
				if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
				{
					//Window Specific, need to move to platform namespace
					HWND hWnd = GetConsoleWindow(); // Get the window handle of the console window
					WCHAR fileName[MAX_PATH]; // Buffer to store the selected file name

					std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

					if (OpenFileDialog(hWnd, fileName, MAX_PATH)) {
						// File selected, do something with the file
						std::wstring ws(fileName);

						std::string fileNameStr = converter.to_bytes(ws);
						Application::GetSceneSerializer()->Deserialize(fileNameStr, true);
						SceneChangeEvent e;
						EventSystem::Get().PushEvent(&e, true);
						//MessageBox(hWnd, fileName, L"Selected File", MB_OK | MB_ICONINFORMATION);
					}
					else {
						// User canceled or error occurred
						MessageBox(hWnd, L"No file selected.", L"Info", MB_OK | MB_ICONINFORMATION);
					}
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					Application::GetSceneSerializer()->Serialize("temp.scene");
				}
				if (ImGui::MenuItem("Save As..")) {
					//Window Specific, Need to move to platform namespace
					HWND hWnd = GetConsoleWindow(); // Get the window handle of the console window
					WCHAR fileName[MAX_PATH]; // Buffer to store the selected file name

					std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

					if (SaveFileDialog(hWnd, fileName, MAX_PATH)) {
						std::wstring ws(fileName);

						std::string fileNameStr = converter.to_bytes(ws);
						Application::GetSceneSerializer()->Serialize(fileNameStr, true);
					}
					else {
						// User canceled or error occurred
						MessageBox(hWnd, L"Could not save scene.", L"Info", MB_OK | MB_ICONINFORMATION);
					}
				}
				if (ImGui::MenuItem("Exit")) Application::Get().quit();
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
			ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		ImGui::End();
	}

	void GUI::Viewport() {

		//update in future to scale with window size
		if (ImGui::Begin("Viewport")) {
			

			ImVec2 windowSize = ImGui::GetContentRegionAvail();
			ImVec2 imageSize = ImVec2(Application::Get().getConfig().windowWidth, Application::Get().getConfig().windowHeight);
			float imageAspectRatio = imageSize.x / imageSize.y;

			ImVec2 displaySize;
			if (windowSize.x / windowSize.y > imageAspectRatio) {
				displaySize.y = windowSize.y;
				displaySize.x = windowSize.y * imageAspectRatio;
			}
			else {
				displaySize.x = windowSize.x;
				displaySize.y = windowSize.x / imageAspectRatio;
			}

			ImVec2 uv0 = ImVec2(0, 0);
			ImVec2 uv1 = ImVec2(displaySize.x / imageSize.x, displaySize.y / imageSize.y);
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddCallback(viewportCallback, &viewportPipelineInfo);
			ImGui::Image(viewportSet, displaySize, uv0, ImVec2(1,1), ImVec4(1, 1, 1, 1));
			drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

			//imGuizmo
			DrawViewportGizmoButtons();
			if (selectedEntity) {
				if (Rendering::CameraHandle::getMain().CameraData().projectionType == Rendering::CameraData::ProjectionType::Orthographic) {
					ImGuizmo::SetOrthographic(true);
				}
				else {
					ImGuizmo::SetOrthographic(false);
				}
				ImGuizmo::SetDrawlist();
				float windowWidth = (float)ImGui::GetWindowWidth();
				float windowHeight = (float)ImGui::GetWindowHeight();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, displaySize.x, displaySize.y);

				auto& cameraHandle = Rendering::CameraHandle::getMain();
				glm::mat4 cameraProjection = cameraHandle.getProjection();
				cameraProjection[1][1] *= -1;

				glm::mat4 cameraView = cameraHandle.getView();

				glm::mat4 transform = selectedEntity->GetComponent<Transform>();

				
				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)ImGuizmoType,
					ImGuizmo::LOCAL, glm::value_ptr(transform));

				if (ImGuizmo::IsUsing()) {
					glm::vec3 rotation = selectedEntity->GetComponent<Transform>().rotation;

					selectedEntity->GetComponent<Transform>().UpdateTransform(transform);
					glm::vec3 deltaRotation = selectedEntity->GetComponent<Transform>().rotation - rotation;
					selectedEntity->GetComponent<Transform>().rotation = rotation + deltaRotation;
				}
			}
		}
		ImGui::End();
	}

	void GUI::Hierarchy() {

		if (ImGui::Begin("Hierarchy")) {
			Application::Get().getScene().GetAllEntitiesWith<GameObject>(entt::exclude<MaidenInternal>).each([&](auto entityId, auto& gameObject)
			{
				Entity entity {entityId, &Application::Get().getScene()};
				DrawEntityNode(entity);
			});
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			selectedEntity = nullptr;
		}

		if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems)) {
			if (ImGui::MenuItem("Create Empty GameObject")) {
				auto entity = Application::Get().getScene().CreateEntity();
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
					if (ImGui::MenuItem("Camera")) {
						selectedEntity->AddComponent<Camera>();
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Mesh Filter")) {
						selectedEntity->AddComponent<MeshFilter>();
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Mesh Renderer")) {
						selectedEntity->AddComponent<MeshRenderer>();
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Point Light")) {
						selectedEntity->AddComponent<PointLight>();
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
		
	}

	void GUI::Project() {
		if (ImGui::Begin("FileSystem")) {

			
		}
		ImGui::End();
		
	}

	void GUI::Console() {
		if (ImGui::Begin("Console")) {

			
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
				std::vector<VkRenderPass> renderpasses = Rendering::Renderer::Get().getRenderPasses();
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
							renderLayers = Application::Get().getMasterRenderSystem().getRenderLayers();
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
								DrawPipelineSettings(renderLayer, j);
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
					Application::Get().getMasterRenderSystem().switchRenderSystems(selectedPipeline.second, selectedPipeline.second - 1);
					selectedPipeline.second--;
				}
				else if (!isUpHit && isDownHit) {
					MADAM_CORE_INFO("Down has been hit");
					Application::Get().getMasterRenderSystem().switchRenderSystems(selectedPipeline.second, selectedPipeline.second + 1);
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

	void GUI::DrawEntityNode(Entity entity) {
		auto name = entity.GetComponent<GameObject>().name;

		ImGuiTreeNodeFlags flags = ((selectedEntity != nullptr && *selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0 ) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());
		if (ImGui::IsItemClicked()) {
			selectedEntity = CreateRef<Entity>(entity);
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Entity")) {
				entityDeleted = true;
			}
			ImGui::EndPopup();
		}

		if (opened) {
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
		if (entity.HasComponent<GameObject>()) {
			auto& gameObject = entity.GetComponent<GameObject>();

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), gameObject.name.c_str());

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			if (ImGui::InputText("##Name", buffer, sizeof(buffer))) {
				entity.GetComponent<GameObject>().name = std::string(buffer);
			}
			ImGui::PopStyleVar();
		}

		const ImGuiTreeNodeFlags headerFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;


		if (entity.HasComponent<Transform>()) {

			auto& transform = entity.GetComponent<Transform>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = ImGui::TreeNodeEx((void*)typeid(Transform).hash_code(), headerFlags, "Transform");
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
				//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
				DrawVec3("Translation", transform.translation);
				//ImGui::DragFloat3("Position", glm::value_ptr(transform.translation), 0.5f);
				glm::vec3 rotation = glm::degrees(transform.rotation);
				DrawVec3("Rotation", rotation);
				transform.rotation = glm::radians(rotation);
				DrawVec3("Scale", transform.scale);
				ImGui::PopStyleVar();
				//ImGui::PopStyleVar();
				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<Camera>()) {
			auto& camera = entity.GetComponent<Camera>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = ImGui::TreeNodeEx((void*)typeid(Camera).hash_code(), headerFlags, "Camera");
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
				Rendering::CameraData& cameraData = camera.cameraHandle->CameraData();
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
				entity.RemoveComponent<Camera>();
			}
		}

		if (entity.HasComponent<MeshFilter>()) {
			auto& meshFilter = entity.GetComponent<MeshFilter>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = ImGui::TreeNodeEx((void*)typeid(MeshFilter).hash_code(), headerFlags, "Mesh Filter");
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
				ImGui::Text("Mesh Filter");
				ImGui::TreePop();
			}

			if (removeComponent) {
				entity.RemoveComponent<MeshFilter>();
			}
		}

		if (entity.HasComponent<MeshRenderer>()) {
			auto& meshRenderer = entity.GetComponent<MeshRenderer>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = ImGui::TreeNodeEx((void*)typeid(MeshRenderer).hash_code(), headerFlags, "Mesh Renderer");
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
				ImGui::Text("Mesh Renderer");
				ImGui::TreePop();
			}

			if (removeComponent) {
				entity.RemoveComponent<MeshRenderer>();
			}
		}

		if (entity.HasComponent<PointLight>()) {
			auto& pointLight = entity.GetComponent<PointLight>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = ImGui::TreeNodeEx((void*)typeid(PointLight).hash_code(), headerFlags, "Point Light");
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
				entity.RemoveComponent<PointLight>();
			}
		}

		if (entity.HasComponent<Material>()) {
			auto& material = entity.GetComponent<Material>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = ImGui::TreeNodeEx((void*)typeid(Material).hash_code(), headerFlags, "Material");
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
				ImGui::Text("Mesh Filter");
				ImGui::TreePop();
			}

			if (removeComponent) {
				entity.RemoveComponent<Material>();
			}
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

	void GUI::DrawViewportGizmoButtons() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
		if (ImGui::Begin("GizmoButtons", nullptr, ImGuiWindowFlags_NoTitleBar)) {
			if (ImGui::Checkbox("Translate", &gizmoButtonStates[0])) {

				if (gizmoButtonStates[0]) {
					gizmoButtonStates[1] = false;
					gizmoButtonStates[2] = false;
					ImGuizmoType = ImGuizmo::TRANSLATE;
				}
			}
			if (ImGui::Checkbox("Rotate", &gizmoButtonStates[1])) {

				if (gizmoButtonStates[1]) {
					gizmoButtonStates[0] = false;
					gizmoButtonStates[2] = false;
					ImGuizmoType = ImGuizmo::ROTATE;
				}
			}
			if (ImGui::Checkbox("Scale", &gizmoButtonStates[2])) {

				if (gizmoButtonStates[2]) {
					gizmoButtonStates[0] = false;
					gizmoButtonStates[1] = false;
					ImGuizmoType = ImGuizmo::SCALE;
				}
			}
			bool isTrue = false;
			for (size_t i = 0; i < gizmoButtonStates.size(); i++)
			{
				if (gizmoButtonStates[i]) {
					isTrue = true;
					break;
				}
			}
			if (!isTrue) {
				ImGuizmoType = 0;
			}
			ImGui::End();
		}
		ImGui::PopStyleVar();
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
		configInfo.renderPass = Rendering::Renderer::Get().getSwapChainRenderPass();
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
		viewportPipelineInfo.pipeline = CreateRef<Pipeline>(device, vertCode, fragCode, configInfo);
		viewportPipelineInfo.layout = configInfo.pipelineLayout;
	}
}