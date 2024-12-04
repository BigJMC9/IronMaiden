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
#include "H_Time.hpp"
#include "../Interfaces/H_Interface.h"
#include "../Events/H_EventSystem.h"

#include <filesystem>

namespace Madam {
	
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

	// With madam api macro, the compiler will throw a warning, may need change in future, look at microsoft C4251 warn page for more details
	class MADAM_API Application {
	protected:
		Application();
	public:
		virtual ~Application();
		
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void init();
		void deinit();

		float getAspectRatio() {
			return renderer.getAspectRatio();
		}

		void addSurface(Scope<EngineInterface> _surface);

		static Application& Get();

		static Application* GetPtr();

		static SceneSerializer* GetSceneSerializer();

		// Use const func() const {} for readonly vars

		Window& getWindow() { return window;  }
		Rendering::RenderStack& getMasterRenderSystem() { return renderStack; }

		//Depreciated
		const std::vector<Ref<Rendering::RenderLayer>>& getRenderLayers() const;

		Scene& getScene() { return *_scene; }
		const Time& getTime() const { return time; }
		ApplicationInfo getConfig() {
			return config;
		}
		
		//Put in engine config header
		const float MAX_FRAME_TIME = 0.1f;

		bool debug = false;

		std::string CreateScript() {
			std::string returnVal = createNative;
			createNative = "";
			return returnVal;
		}

		void setCreateNative(const std::string scriptName) {
			createNative = scriptName;
		}

		bool isScan() {
			bool temp = isScanning;
			isScanning = false;
			return temp;
		}

		void setScan() {
			isScanning = true;
		}

		bool isPlay() const {
			return runtime;
		}

		bool isRuntimeFlag() {
			bool temp = runtimeFlag;
			runtimeFlag = false;
			return temp;
		}

		void setRuntimeFlag() {
			runtimeFlag = true;
		}

		void setRuntimeStopFlag() {
			runtimeStopFlag = true;
		}

		bool isRuntimeStopFlag() {
			bool temp = runtimeStopFlag;
			runtimeStopFlag = false;
			return temp;
		}

		void RuntimeStart() {
			runtime = true;
		}

		void RuntimeStop() {
			if (runtime) {
				runtime = false;
				//SwitchScenes(true); //Fix this
			}
		}

		void SwitchScenes(Ref<Scene> scene)
		{
			_scene = scene;
			SceneChangeEvent e;
			Events::EventSystem::Get().PushEvent(&e, true);
		}

		bool isUpdate() 
		{
			bool temp = isUpdating;
			isUpdating = false;
			return temp;
		}

		void setUpdate() 
		{
			isUpdating = true;
		}

		bool getScripts() 
		{
			bool temp = isGettingScripts;
			isGettingScripts = false;
			return temp;
		}

		void setScripts() 
		{
			isGettingScripts = true;
		}

		void configureApp();
		void saveSession();
		void run();
		void quit();

		Scope<EngineInterface> pSurface = nullptr;

	private:
		static Application* instance;
		static bool instanceFlag;
		ApplicationInfo config;

		Window window = Window{};
		Device device = Device{ window };
		Rendering::Renderer renderer = Rendering::Renderer{window, device};
		Rendering::RenderStack renderStack = Rendering::RenderStack{ device, renderer };
		Time time = Time{};
		EventSystem eventSystem = EventSystem{};

		//Move this somewhere
		Scope<DescriptorPool> globalPool{};
		std::vector<Scope<DescriptorPool>> framePools;

		bool isRunning = false;
		bool firstFrame = true;
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
		
		//Need Scene Management class
		Ref<Scene> _scene = nullptr;
		Ref<Scene> runtimeScene = nullptr;
		SceneSerializer* pSceneSerializer = nullptr;

	protected:
		
	};

	//Defined by client
	Application* CreateApplication();
	
}