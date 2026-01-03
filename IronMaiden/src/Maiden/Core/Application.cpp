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

namespace Madam
{

	Application* Application::Instance = nullptr;
	bool Application::instanceFlag = false;

	Application::Application()
	{

		Init();
		globalPool = DescriptorPool::Builder(device)
			.setMaxSets(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		auto framePoolBuilder = DescriptorPool::Builder(device)
			.setMaxSets(1000) //Storage allocation
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
		for (int i = 0; i < Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
		{
			Scope<DescriptorPool> framePool = framePoolBuilder.build();
			framePools.emplace_back(std::move(framePool));
		}

		m_sceneManager.SetActiveScene(CreateRef<Scene>());
	}

	Application::~Application()
	{
		if (isRunning)
		{
			MADAM_CORE_WARN("Application prematurally shutdown");
			Deinit();
		}
	}

	void Application::Init()
	{
		Instance = this;
		instanceFlag = true;
		ConfigureApp();
		window.init(config.windowWidth, config.windowHeight, config.windowName);
		device.init();
		renderer.Init();
		isRunning = true;
	}

	void Application::Deinit()
	{
		renderer.Deinit();
		window.deinit();
		device.deinit();
		isRunning = false;
	}

	void Application::AddSurface(Scope<EngineInterface> _surface)
	{
		p_surface = std::move(_surface);
		MADAM_CORE_INFO("EngineInterface added");
	}

	Application& Application::Get()
	{
		MADAM_CORE_ASSERT(instanceFlag, "Application Instance not created");
		return *Instance;
	}

	Application* Application::GetPtr()
	{
		MADAM_CORE_ASSERT(instanceFlag, "Application Instance not created");
		return Instance;
	}

	IrmResult Application::LoadScene(std::filesystem::path file_path)
	{
		return m_sceneManager.LoadScene(file_path);
	}

	IrmResult Application::SaveScene(std::filesystem::path file_path)
	{
		return m_sceneManager.SaveScene(file_path);
	}

	void Application::Run()
	{

		Scope<UI::GUI> p_gui = std::make_unique<UI::GUI>();
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
			uboBuffers[i]->Map();
		}
		MADAM_CORE_INFO("uboBuffers mapped");
		Scope<DescriptorSetLayout> globalSetLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT);
		MADAM_CORE_INFO("globalSetLayout vector populated");
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->DescriptorInfo();
			DescriptorWriter(*globalSetLayout, *globalPool)
				.WriteBuffer(0, &bufferInfo)
				.Build(globalDescriptorSets[i]);
		}
		MADAM_CORE_INFO("globalSetLayout vector");
		render_stack.initialize(globalSetLayout);
		MADAM_CORE_INFO("render_stack initialized");
		first_frame = true;

		if (device.device() != VK_NULL_HANDLE)
		{
			MADAM_CORE_INFO("Device is not null");
		}
		else
		{
			MADAM_CORE_INFO("Device is null");
		}
		p_surface->OnAttach();
		p_gui->OnAttach();

		time.StartTime();

		while (!window.shouldClose())
		{
			glfwPollEvents();
			time.UpdateTime();
			if (reload_shaders == true)
			{
				render_stack.reloadPipeline(3, renderer.GetMainRenderPass(), globalSetLayout);
				reload_shaders = false;
			}
			p_surface->OnUpdate();
			p_gui->OnUpdate();
			Scene& activeScene = m_sceneManager.GetActiveScene();
			activeScene.Update();

			if (renderer.BeginFrame())
			{
				activeScene.BuildRenderScene(m_renderScene);
				auto command_buffer = renderer.BeginCommandBuffer();
				int frame_index = renderer.GetFrameIndex();
				framePools[frame_index]->resetPool();
				GlobalUbo ubo{};
				FrameInfo frameInfo{
					frame_index,
					time.GetFrameTime(),
					command_buffer,
					globalDescriptorSets[frame_index],
					*framePools[frame_index],
					m_sceneManager.GetActiveSceneRef(),
					m_renderScene,
					ubo };

				//Should be done in renderer
				Rendering::CameraHandle& camera = Rendering::CameraHandle::GetMain();
				frameInfo.ubo.projection = camera.GetProjection();
				frameInfo.ubo.view = camera.GetView();
				frameInfo.ubo.inverseView = camera.GetInverseView();

				//This Specific Behaviour should be done by a proper render system obj (after renderstack and layers are refactored)
				entt::registry& entities = *m_renderScene.registry;
				int lightIndex = 0;
				for (auto entity : m_renderScene.pointLightEntities)
				{
					auto& pointLight = entities.get<CPointLight>(entity);

					//copy light to ubo

					UUID uuid = entities.get<CUniqueIdentifier>(entity).uuid;
					glm::mat4 worldTransform = activeScene.GetWorldTransform(uuid);
					glm::vec3 worldTranslation = glm::vec3(worldTransform[3][0], worldTransform[3][1], worldTransform[3][2]);
					frameInfo.ubo.pointLights[lightIndex].position = glm::vec4(worldTranslation, 1.f);
					frameInfo.ubo.pointLights[lightIndex].color = glm::vec4(pointLight.color, pointLight.intensity);

					lightIndex++;
				}
				frameInfo.ubo.numLights = lightIndex;
				uboBuffers[frame_index]->WriteToBuffer(&frameInfo.ubo);
				uboBuffers[frame_index]->Flush();

				// render
				activeScene.Render();
				renderer.BeginRenderPass(command_buffer, 0);
				render_stack.render(frameInfo);
				renderer.EndRenderPass(command_buffer);
				renderer.PipelineBarrier(command_buffer, false, false, frame_index, 0);
				renderer.BeginSwapChainRenderPass(command_buffer);
				renderer.EndSwapChainRenderPass(command_buffer);
				renderer.EndFrame();

				if (first_frame)
				{
					first_frame = false;
				}
			}
		}

		MADAM_CORE_INFO("Closing Program");
		vkDeviceWaitIdle(device.device());
		renderer.FreeCommandBuffers();
		renderer.DestroyCommandPool();
		render_stack.deinit();
		framePools.clear();
		globalDescriptorSets.clear();
		globalSetLayout = nullptr;
		uboBuffers.clear();
		globalPool.reset();
		m_sceneManager.SetActiveScene(nullptr);
		SaveSession();
		p_gui = nullptr;
		Deinit();
	}

	void Application::ConfigureApp()
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

	void Application::SaveSession()
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

	void Application::Quit()
	{
		window.quit();
	}
}
