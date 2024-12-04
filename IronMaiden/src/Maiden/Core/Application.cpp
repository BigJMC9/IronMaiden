#include "maidenpch.hpp"
#include "H_Application.hpp"
#include "H_Logger.hpp"
#include "../Scene/H_SceneSerializer.hpp"
#include "../Events/H_Input.hpp"
#include "../Rendering/H_Buffer.hpp"
#include "../GUI/H_GUI.hpp"
#include "../Project/H_Project.h"
#include "../Platform/Platforms.hpp"

#include <fstream>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Madam {

	Application* Application::instance = nullptr;
	bool Application::instanceFlag = false;

	Application::Application() {
		
		init();
		//Data to be shared among all objects (UBO)
		globalPool = 
			DescriptorPool::Builder(device)
				.setMaxSets(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT) //Maximun number of frames being rendered
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT)
				.build();

		// build frame descriptor pools
		// ???
		// Should be in descriptor Manager
		// framePools.resize(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT); //Maximun number of frames being rendered
		auto framePoolBuilder = DescriptorPool::Builder(device)
			.setMaxSets(1000) //Storage allocation
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
		for (int i = 0; i < Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			 Scope<DescriptorPool> framePool = framePoolBuilder.build();
			 framePools.emplace_back(std::move(framePool));
		}
		
		_scene = std::make_shared<Scene>();
		pSceneSerializer = new SceneSerializer(_scene, device);
	}

	Application::~Application() {
		if (isRunning) {
			MADAM_CORE_WARN("Application prematurally shutdown");
			deinit();
		}
	}

	void Application::init() {
		instance = this;
		instanceFlag = true;
		configureApp();
		window.init(config.windowWidth, config.windowHeight, config.windowName);
		device.init();
		renderer.init();
		isRunning = true;
	}

	void Application::deinit() {
		renderStack.deinit();
		renderer.deinit();
		window.deinit();
		delete pSceneSerializer;
		isRunning = false;
	}

	void Application::addSurface(Scope<EngineInterface> _surface)
	{
		pSurface = std::move(_surface);
		MADAM_CORE_INFO("EngineInterface added");
	}

	Application& Application::Get()
	{
		MADAM_CORE_ASSERT(instanceFlag, "Application instance not created");
		return *instance;
	}

	Application* Application::GetPtr()
	{
		MADAM_CORE_ASSERT(instanceFlag, "Application instance not created");
		return instance;
	}

	SceneSerializer* Application::GetSceneSerializer()
	{
		MADAM_CORE_ASSERT(instanceFlag, "Application instance not created");
		return instance->pSceneSerializer;
	}


	const std::vector<Ref<Rendering::RenderLayer>>& Application::getRenderLayers() const {
		return renderStack.getRenderLayers();
	}

	void Application::run() {

		Scope<UI::GUI> pGUI = std::make_unique<UI::GUI>();
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

			pSurface->OnUpdate();
			pGUI->OnUpdate();
			_scene->Update();

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
					_scene,
					ubo};

				//Should be done in renderer
				Rendering::CameraHandle& camera = Rendering::CameraHandle::GetMain();
				frameInfo.ubo.projection = camera.GetProjection();
				frameInfo.ubo.view = camera.GetView();
				frameInfo.ubo.inverseView = camera.GetInverseView();
				
				//This Specific Behaviour should be done by a proper render system obj (after renderstack and layers are refactored)
				auto group = _scene->Reg().view<CTransform, CPointLight>();
				int lightIndex = 0;
				for (auto entity : group)
				{
					auto [transform, pointLight] = group.get<CTransform, CPointLight>(entity);

					//copy light to ubo
					frameInfo.ubo.pointLights[lightIndex].position = glm::vec4(transform.translation, 1.f);
					frameInfo.ubo.pointLights[lightIndex].color = glm::vec4(pointLight.color, pointLight.intensity);

					lightIndex ++;
				}
				frameInfo.ubo.numLights = lightIndex;
				uboBuffers[frameIndex]->writeToBuffer(&frameInfo.ubo);
				uboBuffers[frameIndex]->flush();

				// render
				_scene->Render();
				renderer.beginRenderPass(commandBuffer, 0);
				renderStack.render(frameInfo);
				renderer.endRenderPass(commandBuffer);
				renderer.PipelineBarrier(commandBuffer, false, false, frameIndex, 0);
				renderer.beginSwapChainRenderPass(commandBuffer);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
				
				if (firstFrame) {
					firstFrame = false;
				}
			}
		}
		
		MADAM_CORE_INFO("Closing Program");
		framePools.clear();
		globalPool.reset();
		saveSession();
		vkDeviceWaitIdle(device.device());
		pGUI = nullptr;
		deinit();
	}

	void Application::configureApp()
	{
		std::ifstream prefFile;
		if (!std::filesystem::exists("pref.conf"))
		{
			std::ofstream fout("pref.conf");
			fout << "ProjectsDirectory: " << config.projectsDirectory.string() << std::endl;
			fout << "WindowWidth: " << config.windowWidth << std::endl;
			fout << "WindowHeight: " << config.windowHeight << std::endl;
			fout.close();
		}

		if (!Platform::OpenFile(prefFile, "pref.conf"))
		{
			MADAM_CORE_ERROR("Failed to open pref.conf");
			return;
		}

		std::string line;
		while (std::getline(prefFile, line))
		{
			std::string key = line.substr(0, line.find(':'));
			std::string value = line.substr(line.find(':') + 2);
			if (key == "ProjectsDirectory")
			{
				config.projectsDirectory = std::filesystem::u8path(value);
				MADAM_CORE_INFO("Directory: {0}", config.projectsDirectory);
			}
			else if (key == "WindowWidth")
			{
				config.windowWidth = static_cast<uint32_t>(std::stoul(value));
			}
			else if (key == "WindowHeight")
			{
				config.windowHeight = static_cast<uint32_t>(std::stoul(value));
			}
		}
		prefFile.close();

		std::ifstream lastSession;

		if (Platform::OpenFile(lastSession, "session.ini"))
		{
			std::string line;
			while (std::getline(lastSession, line))
			{
				std::string key = line.substr(0, line.find(':'));
				std::string value = line.substr(line.find(':') + 2);
				if (key == "LastProject")
				{
					if (Project::loadProject(std::filesystem::u8path(value))) 
					{
						config.windowName += " - " + config.version + " - " + Project::Get().getProjectInfo().projectName;
					}
					else
					{
						ProjectConfig projConfig;
						projConfig.projectAuthor = "Me";
						projConfig.projectName = "NewProject";
						projConfig.projectVersion = config.version;
						projConfig.projectsDirectory = config.projectsDirectory;
						Project::newProject(projConfig);
					}
				}
				else if (key == "WindowWidth")
				{
					config.windowWidth = static_cast<uint32_t>(std::stoul(value));
				}
				else if (key == "WindowHeight")
				{
					config.windowHeight = static_cast<uint32_t>(std::stoul(value));
				}
			}
			lastSession.close();
		}
		else
		{
			MADAM_CORE_INFO("Failed to open session.ini");

			ProjectConfig projConfig;
			projConfig.projectAuthor = "Me";
			projConfig.projectName = "NewProject";
			projConfig.projectVersion = config.version;
			projConfig.projectsDirectory = config.projectsDirectory;
			Project::newProject(projConfig);
		}
	}

	void Application::saveSession()
	{
		std::ofstream lastSession("session.ini");
		if (lastSession.is_open())
		{
			lastSession << "LastProject: " << Project::Get().getProjectDirectory().string() << std::endl;
			lastSession << "WindowWidth: " << window.getWidth() << std::endl;
			lastSession << "WindowHeight: " << window.getHeight() << std::endl;
			lastSession.close();
		}
		else
		{
			MADAM_CORE_ERROR("Failed to open session.ini");
		}
		Project::saveProject();
	}

	void Application::quit() {
		window.quit();
	}
}