#include "maidenpch.hpp"
#include "H_Application.hpp"
#include "H_Logger.hpp"
#include "../Scene/H_SceneSerializer.hpp"
#include "../Events/H_Input.hpp"
#include "../Rendering/H_Buffer.hpp"
#include "../GUI/H_GUI.hpp"
#include "../Project/H_Project.h"
#include "../Asset/H_AssetSystem.h"
#include "../Asset/H_MaterialAsset.h"
#include "../Rendering/H_ShadowSystem.hpp"
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
			.setMaxSets(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT * 4)
			.build();

		const uint32_t maxMaterialSets = 2000;
		const uint32_t texturesPerMaterial = 32;
		materialPool = DescriptorPool::Builder(device)
			.setMaxSets(maxMaterialSets)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxMaterialSets * texturesPerMaterial)
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

	void Application::SwitchScenes(Ref<Scene> scene)
	{
		if (m_sceneManager.GetActiveSceneRef() == scene) return;
		// Scene components can own mesh and material GPU resources referenced by
		// the frames already submitted to Vulkan.
		vkDeviceWaitIdle(device.device());
		m_renderScene.Clear();
		m_sceneManager.SwitchScene(std::move(scene));
	}

	IrmResult Application::LoadScene(std::filesystem::path file_path)
	{
		vkDeviceWaitIdle(device.device());
		m_renderScene.Clear();
		return m_sceneManager.LoadScene(file_path);
	}

	IrmResult Application::SaveScene(std::filesystem::path file_path)
	{
		return m_sceneManager.SaveScene(file_path);
	}

	void Application::Run()
	{

		Scope<UI::GUI> p_gui = std::make_unique<UI::GUI>();
		Rendering::ShadowSystem shadowSystem(device);
		shadowSystem.Init();
		std::vector < Scope<Buffer>> uboBuffers(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT);
		std::vector<Scope<Buffer>> gameUboBuffers(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT);
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
			gameUboBuffers[i] = std::make_unique<Buffer>(
				device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			gameUboBuffers[i]->Map();
		}
		MADAM_CORE_INFO("uboBuffers mapped");
		Scope<DescriptorSetLayout> globalSetLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT);
		std::vector<VkDescriptorSet> gameGlobalDescriptorSets(Rendering::SwapChain::MAX_FRAMES_IN_FLIGHT);
		MADAM_CORE_INFO("globalSetLayout vector populated");
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->DescriptorInfo();
			auto realtimeShadowInfo = shadowSystem.RealtimeDescriptor(i);
			auto bakedShadowInfo = shadowSystem.BakedDescriptor();
			if (!DescriptorWriter(*globalSetLayout, *globalPool)
				.WriteBuffer(0, &bufferInfo)
				.WriteImage(1, &realtimeShadowInfo)
				.WriteImage(2, &bakedShadowInfo)
				.Build(globalDescriptorSets[i]))
				throw std::runtime_error("Could not allocate the viewport camera descriptor set");
			auto gameBufferInfo = gameUboBuffers[i]->DescriptorInfo();
			if (!DescriptorWriter(*globalSetLayout, *globalPool)
				.WriteBuffer(0, &gameBufferInfo)
				.WriteImage(1, &realtimeShadowInfo)
				.WriteImage(2, &bakedShadowInfo)
				.Build(gameGlobalDescriptorSets[i]))
				throw std::runtime_error("Could not allocate the game camera descriptor set");
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
		const auto elapsedMs = [](const auto start, const auto end) {
			return std::chrono::duration<float, std::milli>(end - start).count();
		};

		while (!window.shouldClose())
		{
			glfwPollEvents();
			time.UpdateTime();
			const float frameSeconds = time.GetFrameTime();
			if (frameSeconds > 0.0f) {
				const float sampleMs = frameSeconds * 1000.0f;
				const float blend = 1.0f - std::exp(-8.0f * frameSeconds);
				performanceStats.frameMs = performanceStats.frameMs == 0.0f ? sampleMs
					: performanceStats.frameMs + (sampleMs - performanceStats.frameMs) * blend;
				performanceStats.fps = 1000.0f / performanceStats.frameMs;
			}
			if (reload_shaders == true)
			{
				render_stack.reloadPipeline(1, renderer.GetMainRenderPass(), globalSetLayout);
				render_stack.reloadPipeline(3, renderer.GetMainRenderPass(), globalSetLayout);
				if (Project::HasActive())
				{
					auto& assets = Project::Get().getAssetManager();
					for (const auto& entry : assets.ListAssets(AssetType::MATERIAL))
					{
						auto material = std::dynamic_pointer_cast<MaterialAsset>(assets.GetAsset(entry.handle));
						if (material)
							material->GetRuntimeMaterial(assets);
					}
				}
				reload_shaders = false;
			}
			const auto updateStart = std::chrono::steady_clock::now();
			p_surface->OnUpdate();
			const auto surfaceEnd = std::chrono::steady_clock::now();
			p_gui->OnUpdate();
			const auto uiEnd = std::chrono::steady_clock::now();
			Scene& activeScene = m_sceneManager.GetActiveScene();
			activeScene.Update();
			const auto sceneEnd = std::chrono::steady_clock::now();
			shadowSystem.SyncBake(activeScene);
			const auto bakeEnd = std::chrono::steady_clock::now();
			performanceStats.surfaceCpuMs = elapsedMs(updateStart, surfaceEnd);
			performanceStats.uiCpuMs = elapsedMs(surfaceEnd, uiEnd);
			performanceStats.sceneCpuMs = elapsedMs(uiEnd, sceneEnd);
			performanceStats.bakeCpuMs = elapsedMs(sceneEnd, bakeEnd);
			performanceStats.updateCpuMs = elapsedMs(updateStart, bakeEnd);

			if (renderer.BeginFrame())
			{
				activeScene.BuildRenderScene(m_renderScene);
				performanceStats.meshCount = static_cast<uint32_t>(m_renderScene.meshEntities.size());
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
				const auto viewportExtent = renderer.GetOffscreenExtent(0);
				auto& editorCameraData = camera.GetMutableCameraData();
				const float viewportAspect = static_cast<float>(viewportExtent.width) / viewportExtent.height;
				if (editorCameraData.projectionType == Rendering::CameraData::ProjectionType::Perspective)
					editorCameraData.perspective.aspect = viewportAspect;
				else if (editorCameraData.projectionType == Rendering::CameraData::ProjectionType::Orthographic)
					editorCameraData.orthographic.aspect = viewportAspect;
				camera.SetProjection();
				frameInfo.ubo.projection = camera.GetProjection();
				frameInfo.ubo.view = camera.GetView();
				frameInfo.ubo.inverseView = camera.GetInverseView();

				const auto shadowStart = std::chrono::steady_clock::now();
				const auto shadowResult = shadowSystem.Prepare(activeScene, m_renderScene, frameInfo.ubo,
					static_cast<uint32_t>(frame_index), command_buffer);
				performanceStats.shadowCpuMs = elapsedMs(shadowStart, std::chrono::steady_clock::now());
				performanceStats.shadowDrawCalls = shadowResult.drawCalls;
				performanceStats.shadowRendered = shadowResult.rendered;
				performanceStats.shadowCached = shadowResult.cached;
				performanceStats.lightCount = frameInfo.ubo.numLights +
					frameInfo.ubo.numDirectionalLights + frameInfo.ubo.numSpotLights;
				uboBuffers[frame_index]->WriteToBuffer(&frameInfo.ubo);
				uboBuffers[frame_index]->Flush();

				// render
				activeScene.Render();
				const auto viewportStart = std::chrono::steady_clock::now();
				renderer.BeginRenderPass(command_buffer, 0);
				render_stack.render(frameInfo);
				renderer.EndRenderPass(command_buffer);
				performanceStats.viewportCpuMs = elapsedMs(viewportStart, std::chrono::steady_clock::now());
				renderer.PipelineBarrier(command_buffer, false, false, frame_index, 0);
				Entity gameCameraEntity = activeScene.GetGameCameraEntity();
				const auto gameStart = std::chrono::steady_clock::now();
				renderer.BeginRenderPass(command_buffer, 2);
				if (gameCameraEntity)
				{
					auto cameraData = gameCameraEntity.GetComponent<CCamera>().GetCameraData();
					const auto gameExtent = renderer.GetOffscreenExtent(2);
					const float gameAspect = static_cast<float>(gameExtent.width) / gameExtent.height;
					if (cameraData.projectionType == Rendering::CameraData::ProjectionType::Perspective)
						cameraData.perspective.aspect = gameAspect;
					else if (cameraData.projectionType == Rendering::CameraData::ProjectionType::Orthographic)
						cameraData.orthographic.aspect = gameAspect;
					Rendering::CameraHandle gameCamera(cameraData);
					const auto& gameTransform = gameCameraEntity.GetComponent<CTransform>();
					glm::vec3 worldPosition = gameTransform.translation;
					glm::quat worldRotation = gameTransform.rotation;
					glm::vec3 worldScale, worldSkew;
					glm::vec4 worldPerspective;
					glm::decompose(activeScene.GetWorldTransform(gameCameraEntity), worldScale,
						worldRotation, worldPosition, worldSkew, worldPerspective);
					gameCamera.SetViewYXZ(worldPosition, glm::normalize(worldRotation));
					gameCamera.SetProjection();
					FrameInfo gameFrameInfo = frameInfo;
					gameFrameInfo.globalDescriptorSet = gameGlobalDescriptorSets[frame_index];
					gameFrameInfo.ubo.projection = gameCamera.GetProjection();
					gameFrameInfo.ubo.view = gameCamera.GetView();
					gameFrameInfo.ubo.inverseView = gameCamera.GetInverseView();
					gameUboBuffers[frame_index]->WriteToBuffer(&gameFrameInfo.ubo);
					gameUboBuffers[frame_index]->Flush();
					render_stack.render(gameFrameInfo, false);
				}
				renderer.EndRenderPass(command_buffer);
				performanceStats.gameCpuMs = gameCameraEntity
					? elapsedMs(gameStart, std::chrono::steady_clock::now()) : 0.0f;
				renderer.PipelineBarrier(command_buffer, false, false, frame_index, 2);
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
		shadowSystem.SyncBake(m_sceneManager.GetActiveScene());
		renderer.FreeCommandBuffers();
		renderer.DestroyCommandPool();
		render_stack.deinit();
		shadowSystem.Shutdown();
		framePools.clear();
		globalDescriptorSets.clear();
		gameGlobalDescriptorSets.clear();
		globalSetLayout = nullptr;
		uboBuffers.clear();
		gameUboBuffers.clear();
		SaveSession();
		// Release editor, scene and asset GPU resources while the Vulkan device is alive.
		p_gui.reset();
		p_surface.reset();
		m_renderScene.Clear();
		m_sceneManager.SetActiveScene(nullptr);
		Project::clear();
		materialPool.reset();
		globalPool.reset();
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
