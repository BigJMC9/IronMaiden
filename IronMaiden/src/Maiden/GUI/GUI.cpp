#include "maidenpch.hpp"
#include "H_GUI.hpp"
#include "../Events/H_Input.hpp"
#include "../Core/H_Application.hpp"

namespace Madam::UI {

	GUI::GUI() : Layer("GUI")
	{

	}

	GUI::~GUI() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void GUI::OnAttach() {
		VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		VkDescriptorPool imguiPool;
		vkCreateDescriptorPool(Application::Get().getDevice().device(), &pool_info, nullptr, &imguiPool);

		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		init_info = Application::Get().getRenderer().getImGuiInitInfo();
		init_info->DescriptorPool = imguiPool;
		init_info->MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplGlfw_InitForVulkan(Application::Get().getWindow().getGLFWwindow(), true);
		ImGui_ImplVulkan_Init(init_info);

		//We need to submit it to the gpu so that it can be there for us to use.
		//This requires a immediate submit function to be created which should be made in the renderer.
	}

	/*void GUI::OnDetach() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}*/

	void GUI::OnUpdate() {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Application::Get().getRenderer().getCurrentCommandBuffer());

		ImGui::ShowDemoWindow();
	}

	void GUI::OnSceneLoad() {

	}
}