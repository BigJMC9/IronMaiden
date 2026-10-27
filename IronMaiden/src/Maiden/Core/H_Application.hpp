#pragma once

// Fix these headers, only essential headers are needed, the rest can reside in the implementation
#include "maidenpch.hpp"
#include "Version.h"
#include "Main/Core.hpp"
#include "H_CoreUtils.hpp"
#include "../Rendering/H_Renderer.hpp"
#include "../Rendering/H_RenderSystems.hpp"
#include "../Rendering/H_DescriptorSetLayout.hpp"
#include "../Scene/H_Scene.hpp"
#include "../Scene/H_SceneManager.hpp"
#include "../Rendering/H_RenderScene.hpp"
#include "H_Time.hpp"
#include "../Interfaces/H_Interface.h"
#include "../Events/H_EventSystem.h"

#include <filesystem>

namespace Madam
{

	class SceneSerializer;
	class Project;

	// Should be passed in when App is created
	struct ApplicationInfo
	{
		std::string name = "IronMaidenEngine";
		std::string version = ENGINE_VERSION;
		std::string windowName = "Iron Maiden Engine";
		uint32_t windowWidth = 1600, windowHeight = 900;
		std::filesystem::path projectsDirectory = "Projects";
	};

	struct PerformanceStats
	{
		float frameMs = 0.0f;
		float fps = 0.0f;
		float updateCpuMs = 0.0f;
		float surfaceCpuMs = 0.0f;
		float uiCpuMs = 0.0f;
		float sceneCpuMs = 0.0f;
		float bakeCpuMs = 0.0f;
		float shadowCpuMs = 0.0f;
		float viewportCpuMs = 0.0f;
		float gameCpuMs = 0.0f;
		uint32_t meshCount = 0;
		uint32_t lightCount = 0;
		uint32_t shadowDrawCalls = 0;
		uint32_t shadowTiles = 0;
		uint32_t shadowDroppedLights = 0;
		bool shadowRendered = false;
		bool shadowCached = false;
	};

	// With madam api macro, the compiler will throw a warning, may need change in future, look at microsoft C4251 warn page for more details
	class MADAM_API Application
	{
	protected:
		Application();
	public:
		virtual ~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void Init();
		void Deinit();

		float GetAspectRatio()
		{
			return renderer.GetAspectRatio();
		}

		void AddSurface(Scope<EngineInterface> _surface);

		static Application& Get();

		static Application* GetPtr();

		// Use const func() const {} for readonly vars

		Window& GetWindow() { return window; }
		Rendering::RenderStack& GetMasterRenderSystem() { return render_stack; }

		Scene& GetScene() { return m_sceneManager.GetActiveScene(); }
		Ref<Scene> GetSceneRef() { return m_sceneManager.GetActiveSceneRef(); }
		const Time& GetTime() const { return time; }
		const PerformanceStats& GetPerformanceStats() const { return performanceStats; }
		DescriptorPool& GetMaterialDescriptorPool() { return *materialPool; }
		ApplicationInfo GetConfig()
		{
			return config;
		}

		//Put in engine config header maybe?
		const float MAX_FRAME_TIME = 0.1f;

		std::string CreateScript()
		{
			std::string returnVal = createNative;
			createNative = "";
			return returnVal;
		}

		void SetCreateNative(const std::string scriptName)
		{
			createNative = scriptName;
		}

		bool IsScan()
		{
			bool temp = isScanning;
			isScanning = false;
			return temp;
		}

		void SetScan()
		{
			isScanning = true;
		}

		bool IsPlay() const
		{
			return runtime;
		}

		bool IsRuntimeFlag()
		{
			bool temp = runtimeFlag;
			runtimeFlag = false;
			return temp;
		}

		void SetRuntimeFlag()
		{
			runtimeFlag = true;
		}

		void SetRuntimeStopFlag()
		{
			runtimeStopFlag = true;
		}

		bool IsRuntimeStopFlag()
		{
			bool temp = runtimeStopFlag;
			runtimeStopFlag = false;
			return temp;
		}

		void RuntimeStart()
		{
			runtime = true;
		}

		void RuntimeStop()
		{
			if (runtime)
			{
				runtime = false;
				//SwitchScenes(true); //Fix this
			}
		}

		void SwitchScenes(Ref<Scene> scene);

		IrmResult LoadScene(std::filesystem::path file_path);
		IrmResult SaveScene(std::filesystem::path file_path);

		bool IsUpdate()
		{
			bool temp = isUpdating;
			isUpdating = false;
			return temp;
		}

		void SetUpdate()
		{
			isUpdating = true;
		}

		bool GetScripts()
		{
			bool temp = isGettingScripts;
			isGettingScripts = false;
			return temp;
		}

		void SetScripts()
		{
			isGettingScripts = true;
		}

		void ReloadShaders()
		{
			reload_shaders = true;
		}

		void ConfigureApp();
		void SaveSession();
		void Run();
		void Quit();

		Scope<EngineInterface> p_surface = nullptr;

	private:
		static Application* Instance;
		static bool instanceFlag;
		ApplicationInfo config;

		Window window = Window{};
		Device device = Device{ window };
		Rendering::Renderer renderer = Rendering::Renderer{ window, device };
		Rendering::RenderStack render_stack = Rendering::RenderStack{ device, renderer };
		Time time = Time{};
		PerformanceStats performanceStats{};
		EventSystem eventSystem = EventSystem{};

		//Move this somewhere
		Scope<DescriptorPool> globalPool{};
		Scope<DescriptorPool> materialPool{};
		std::vector<Scope<DescriptorPool>> framePools;

		bool isRunning = false;
		bool first_frame = true;
		//For Testing Remove when redundant
		std::string createNative = "";
		bool isScanning = true;
		bool isCompiling = false;
		bool runtime = false;
		bool runtimeFlag = false;
		bool runtimeStopFlag = false;
		bool isGettingScripts = false;
		//bool isTesting = false;
		bool isUpdating = false;

		bool reload_shaders = false;

		SceneManager m_sceneManager;
		Rendering::RenderScene m_renderScene{};

	protected:

	};

	//Defined by client
	Application* CreateApplication();

}
