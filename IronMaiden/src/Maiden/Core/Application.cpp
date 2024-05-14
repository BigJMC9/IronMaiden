#include "maidenpch.hpp"
#define MADAM_APP_IMPL_FLAG
#include "H_Application.hpp"
#include "../Scene/H_SceneSerializer.hpp"
#include "H_CmdHandler.hpp"
#include "../Events/H_Input.hpp"
#include "../Rendering/H_Buffer.hpp"
#include "../GUI/H_GUI.hpp"

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
		//Data to be shared among all objects (UBO)
		globalPool = 
			DescriptorPool::Builder(device)
				.setMaxSets(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT) //Maximun number of frames being rendered
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT)
				.build();

		// build frame descriptor pools
		// ???
		framePools.resize(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT); //Maximun number of frames being rendered
		auto framePoolBuilder = DescriptorPool::Builder(device)
			.setMaxSets(1000) //Storage allocation
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
		for (int i = 0; i < framePools.size(); i++) {
			framePools[i] = framePoolBuilder.build();
		}
		
		scene = std::make_shared<Scene>();
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
		pipeHandler.Shutdown();
		renderStack.ShutDown();
		renderer.ShutDown();
		//device.ShutDown(); //For proper shutdown, make singleton
		window.ShutDown();
		isRunning = false;
	}

	void Application::onEvent(Event& e) {
		//EventDispatcher dispatcher(e);
		//dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::onWindowClose));
		//dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::onWindowResize));
		MADAM_CORE_INFO("{0}", e);
	}

	const std::vector<Ref<Rendering::RenderLayer>>& Application::getRenderLayers() const {
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
			MADAM_CORE_INFO("Is connected, starting read!");
			pipeHandler.StartAsyncRead();
		}

		Scope<UI::GUI> pGUI = std::make_unique<UI::GUI>();
		MADAM_CORE_INFO("GUI Created");
		std::vector < Scope<Buffer>> uboBuffers(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT);
		MADAM_CORE_INFO("uboBuffers Created");
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
		MADAM_CORE_INFO("uboBuffers mapped");
		Scope<DescriptorSetLayout> globalSetLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();
		//MADAM_CORE_INFO("globalSetLayout Assigned");
		std::vector<VkDescriptorSet> globalDescriptorSets(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT);
		MADAM_CORE_INFO("globalSetLayout vector populated");
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			DescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}
		MADAM_CORE_INFO("globalSetLayout vector");
		renderStack.initialize(globalSetLayout);
		MADAM_CORE_INFO("renderStack initialized");
		firstFrame = true;

		if (device.device() != VK_NULL_HANDLE) {
			MADAM_CORE_INFO("Device is not null");
		}
		else {
			MADAM_CORE_INFO("Device is null");
		}
		pSurface->OnAttach();
		pGUI->OnAttach();

		time.StartTime();

		while (!window.shouldClose()) {
			glfwPollEvents();
			time.UpdateTime();

			std::string command = pipeHandler.Read();
			if (!command.empty()) {
				MADAM_INFO("command: {0}", command);
				App::CommandHandler::HandleCommand(command, pipeHandler, pSceneSerializer);
			}
			
			pSurface->OnUpdate();
			scene->Update();

			if (renderer.beginFrame()) {
				auto commandBuffer = renderer.beginCommandBuffer();
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

				//Should be done in renderer
				Rendering::CameraHandle& camera = Rendering::CameraHandle::getMain();
				frameInfo.ubo.projection = camera.getProjection();
				frameInfo.ubo.view = camera.getView();
				frameInfo.ubo.inverseView = camera.getInverseView();
				renderStack.preRender(frameInfo);
				uboBuffers[frameIndex]->writeToBuffer(&frameInfo.ubo);
				uboBuffers[frameIndex]->flush();

				// render
				scene->Render();
				renderer.beginRenderPass(commandBuffer, 0);
				renderStack.render(frameInfo);
				renderer.endRenderPass(commandBuffer);
				renderer.PipelineBarrier(commandBuffer, false, false, frameIndex, 0);
				renderer.beginSwapChainRenderPass(commandBuffer);
				pGUI->OnUpdate();
				pGUI->Record(commandBuffer);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
				if (window.wasWindowResized()) {
					pGUI->ReCreate();
					window.resetWindowResizedFlag();
				}
				
				if (firstFrame) {
					firstFrame = false;
				}
			}
		}
		
		MADAM_CORE_INFO("Closing Program");
		framePools.clear();
		globalPool.reset();
		vkDeviceWaitIdle(device.device());
		ShutDown();
	}

	void Application::quit() {
		window.quit();
	}
}