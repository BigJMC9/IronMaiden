#include "H_first_app.hpp"

#include "H_keyboard_movement_controller.hpp"
#include "H_JCVB_buffer.hpp"
//#include "H_Scene.hpp"
//#include "H_SceneSerializer.hpp"
//#include "H_JCVB_camera.hpp"
//#include "H_simple_render_system.hpp"
//#include "H_point_light_system.hpp"
//#include "H_texture_render_system.hpp"
//#include "H_JCVB_mesh_renderer.hpp"
//#include "H_JCVB_material.hpp"


//Define fixes, hacky solution, will fix when headers are restructured
#ifdef min
#undef min
#endif

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <chrono>
#include <array>
#include <cassert>
#include <thread>
#include <iostream>

namespace Madam {

	Application::Application() {
		globalPool = 
			JcvbDescriptorPool::Builder(device)
				.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT) //Storage Allocation
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
				.build();

		// build frame descriptor pools
		framePools.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		auto framePoolBuilder = JcvbDescriptorPool::Builder(device)
			.setMaxSets(1000) //Storage allocation
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
		for (int i = 0; i < framePools.size(); i++) {
			framePools[i] = framePoolBuilder.build();
		}
		
		scene = std::make_unique<Scene>();
		pSceneSerializer = new SceneSerializer(scene, device);
	}

	Application::~Application() {

	}

	void Application::run() {

		//PipeLogic
		if (!pipeHandler.CreatePipe()) {
			std::cerr << "Failed to create pipe" << std::endl;
			pipeHandler.isCreated = false;
			pipeHandler.isConnected = false;
		}
		else {
			pipeHandler.isCreated = true;
			pipeHandler.isConnected = true;
		}

		if (pipeHandler.isConnected) {
			std::cout << "Is connected, starting read!" << std::endl;
			pipeHandler.StartAsyncRead();
		}


		std::vector < std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<Buffer>(
				device,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = JcvbDescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			JcvbDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		masterRenderSystem.initialize(globalSetLayout);

		firstFrame = true;

		//Done by Scene
        Camera camera{};
        camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.f, 0.f, 0.f));
		Entity viewerObject = scene->CreateGameObject();
		viewerObject.SetName("Editor Camera");
		viewerObject.GetComponent<Transform>().translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!window.shouldClose()) {
			glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME);		

			std::string command = pipeHandler.Read();
			if (!command.empty()) {
				std::cout << "command: " << command << std::endl;
				App::CommandHandler::HandleCommand(command, pipeHandler, pSceneSerializer);
			}
			cameraController.handleCommands(window.getGLFWwindow(), pSceneSerializer);
			
            cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 500.0f);

			if (auto commandBuffer = renderer.beginFrame()) {
				int frameIndex = renderer.getFrameIndex();
				framePools[frameIndex]->resetPool();
				GlobalUbo ubo{};
				FrameInfo frameInfo {
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					* framePools[frameIndex],
					scene,
					ubo};
				if (debug) {
					std::cout << "View: " << glm::to_string(ubo.view) << std::endl;
					std::cout << "Inverse View: " << glm::to_string(ubo.inverseView) << std::endl;
					std::cout << "Projection: " << glm::to_string(ubo.projection) << std::endl;
					debug = false;
				}
				frameInfo.ubo.projection = camera.getProjection();
				frameInfo.ubo.view = camera.getView();
				frameInfo.ubo.inverseView = camera.getInverseView();
				masterRenderSystem.preRender(frameInfo);
				uboBuffers[frameIndex]->writeToBuffer(&frameInfo.ubo);
				uboBuffers[frameIndex]->flush();

				// render
				renderer.beginSwapChainRenderPass(commandBuffer);
				masterRenderSystem.render(frameInfo);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
				if (firstFrame) {
					firstFrame = false;
				}
			}
		}

		vkDeviceWaitIdle(device.device());
	}
}