#include "maidenpch.hpp"
#include "H_Application.hpp"
#include "../Scene/H_SceneSerializer.hpp"
#include "H_CmdHandler.hpp"
#include "../Events/H_Input.hpp"
#include "../Rendering/H_Buffer.hpp"

//Define fixes, hacky solution, will fix when headers are restructured
#ifdef min
#undef min
#endif

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Madam {

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::instance = nullptr;
	bool Application::instanceFlag = false;

	Application::Application() {
		
		StartUp();
		globalPool = 
			DescriptorPool::Builder(device)
				.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT) //Storage Allocation
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
				.build();

		// build frame descriptor pools
		framePools.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		auto framePoolBuilder = DescriptorPool::Builder(device)
			.setMaxSets(1000) //Storage allocation
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
		for (int i = 0; i < framePools.size(); i++) {
			framePools[i] = framePoolBuilder.build();
		}
		
		scene = std::make_unique<Scene>();
		pSceneSerializer = new SceneSerializer(scene, device);

		char buff[FILENAME_MAX];
		GetCurrentDir(buff, FILENAME_MAX);
		config.workingDirectory = buff;
	}

	Application::~Application() {
		if (isRunning) {
			MADAM_CORE_WARN("Application prematurally shutdown");
			ShutDown();
		}
	}

	void Application::StartUp() {
		window.StartUp(config.windowWidth, config.windowHeight, config.windowName);
		device.StartUp();
		renderer.StartUp();
		isRunning = true;
		instance = this;
		instanceFlag = true;
	}

	void Application::ShutDown() {
		renderStack.ShutDown();
		renderer.ShutDown();
		device.ShutDown();
		window.ShutDown();
		isRunning = false;
	}

	void Application::onEvent(Event& e) {
		//EventDispatcher dispatcher(e);
		//dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::onWindowClose));
		//dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::onWindowResize));
		MADAM_CORE_INFO("{0}", e);
	}

	const std::vector<std::shared_ptr<Rendering::RenderLayer>>& Application::getRenderLayers() const {
		return renderStack.getRenderLayers();
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

		auto globalSetLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			JcvbDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		renderStack.initialize(globalSetLayout);

		firstFrame = true;

		pSurface->OnAttach();
		//Will be done by Scene
        /*Camera camera{};
        camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.f, 0.f, 0.f));
		Entity viewerObject = scene->CreateGameObject();
		viewerObject.SetName("Editor Camera");
		viewerObject.GetComponent<Transform>().translation.z = -2.5f;
        KeyboardMovementController cameraController{};*/

		time.StartTime();
        //auto currentTime = std::chrono::high_resolution_clock::now();

		while (!window.shouldClose()) {
			glfwPollEvents();

			time.UpdateTime();
            //auto newTime = std::chrono::high_resolution_clock::now();
            //float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            //currentTime = newTime;

            //frameTime = glm::min(frameTime, MAX_FRAME_TIME);

			std::string command = pipeHandler.Read();
			if (!command.empty()) {
				std::cout << "command: " << command << std::endl;
				App::CommandHandler::HandleCommand(command, pipeHandler, pSceneSerializer);
			}
			//cameraController.handleCommands(window.getGLFWwindow(), pSceneSerializer);
			
			pSurface->OnUpdate();
			scene->Update();
            /*cameraController.moveInPlaneXZ(window.getGLFWwindow(), time.GetFrameTime(), viewerObject);
            camera.setViewYXZ(viewerObject.GetComponent<Transform>().translation, viewerObject.GetComponent<Transform>().rotation);*/

            /*float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 500.0f);*/ //Should only update when window is resized

			if (auto commandBuffer = renderer.beginFrame()) {
				int frameIndex = renderer.getFrameIndex();
				framePools[frameIndex]->resetPool();
				GlobalUbo ubo{};
				FrameInfo frameInfo {
					frameIndex,
					time.GetFrameTime(),
					commandBuffer,
					globalDescriptorSets[frameIndex],
					* framePools[frameIndex],
					scene,
					ubo};
				/*if (debug) {
					std::cout << "View: " << glm::to_string(ubo.view) << std::endl;
					std::cout << "Inverse View: " << glm::to_string(ubo.inverseView) << std::endl;
					std::cout << "Projection: " << glm::to_string(ubo.projection) << std::endl;
					debug = false;
				}*/

				//Should be done in render stack
				Rendering::CameraHandle& camera = Rendering::CameraHandle::getMain();
				frameInfo.ubo.projection = camera.getProjection();
				frameInfo.ubo.view = camera.getView();
				frameInfo.ubo.inverseView = camera.getInverseView();
				renderStack.preRender(frameInfo);
				uboBuffers[frameIndex]->writeToBuffer(&frameInfo.ubo);
				uboBuffers[frameIndex]->flush();

				// render
				scene->Render();
				renderer.beginSwapChainRenderPass(commandBuffer);
				renderStack.render(frameInfo);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
				if (firstFrame) {
					firstFrame = false;
				}
			}
		}
		
		std::cout << "Closing Program" << std::endl;
		vkDeviceWaitIdle(device.device());
		ShutDown();
	}
}