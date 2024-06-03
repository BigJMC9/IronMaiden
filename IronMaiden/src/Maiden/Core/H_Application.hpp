#pragma once

//Fix these headers, only essential headers are needed, the rest can reside in the implementation
#include "maidenpch.hpp"
#include "Main/core.hpp"
#include "H_Utils.hpp"
#include "H_Logger.hpp"
#include "../Rendering/H_DescriptorSetLayout.hpp"
#include "../Rendering/H_Renderer.hpp"
#include "../Rendering/H_RenderSystems.hpp"
#include "../Scene/H_Scene.hpp"
#include "H_Time.hpp"
#include "../Interfaces/H_Interface.h"
#include "../Events/H_EventSystem.h"

//int main();

namespace Madam {

	class SceneSerializer;

	//Should be passed in when App is created
	struct ApplicationConfig
	{
		std::string name = "IronMaidenEngine";
		std::string windowName = "Iron Maiden Engine";
		std::string version = "0.05";
		uint32_t windowWidth = 1600, windowHeight = 900;
		std::string workingDirectory;
		std::string projectFolder = "C:/Users/xbox/Desktop/Development/IronMaiden Projects/Sandbox/Sandbox/"; //This should be the folder to the project is stored
		std::string projectWorkingDirectory = "C:/Users/xbox/Desktop/Development/IronMaiden Projects/Sandbox/"; //This should be the folder where the project folder is stored
		std::string internals = "Internal/";
		std::string assets = "Assets/";
		bool is2D = false;
	};


	// With madam api macro, the compiler will throw a warning, may need change in future, look at microsoft C4251 warn page for more details
	class MADAM_API Application {
	protected:
		Application();
	public:
		virtual ~Application();
		
		void StartUp();
		void ShutDown();

		float getAspectRatio() {
			return renderer.getAspectRatio();
		}

		void addSurface(Scope<EngineInterface> _surface) {
			pSurface = std::move(_surface);
			MADAM_CORE_INFO("EngineInterface added");
		}

		static Application& Get() {
			//static Application instance;
			MADAM_CORE_ASSERT(instanceFlag, "Application instance not created");
			return *instance;
		}

		static Application* GetPtr() {
			MADAM_CORE_ASSERT(instanceFlag, "Application instance not created");
			return instance;
		}

		static SceneSerializer* GetSceneSerializer() {
			MADAM_CORE_ASSERT(instanceFlag, "Application instance not created");
			return instance->pSceneSerializer;
		}

		// Use const func() const {} for readonly vars

		Window& getWindow() { return window;  }
		Rendering::RenderStack& getMasterRenderSystem() { return renderStack; }

		//Depreciated
		const std::vector<Ref<Rendering::RenderLayer>>& getRenderLayers() const;

		Scene& getScene() { return *scene; }
		const Time& getTime() const { return time; }
		ApplicationConfig getConfig() {
			return config;
		}
		
		//Put in engine config header
		const float MAX_FRAME_TIME = 0.1f;

		bool debug = false;

		std::string CreateScript() {
			std::string returnVal = createNative;
			if (returnVal != "") {
				MADAM_CORE_INFO("Creating Native Script: {0}", returnVal);
			}
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

		/*bool isCompile() {
			bool temp = isCompiling;
			isCompiling = false;
			return temp;
		}

		void setCompile() {
			isCompiling = true;
		}*/

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
				SwitchScenes(true);
			}
		}


		//Temp solution. Need to move to Scene Management class and needs to be safer
		void PrimeReserve(Ref<Scene> _scene) {
			reservedScene = _scene;
		}

		void SwitchScenes() {
			if (reservedScene == nullptr) {
				MADAM_CORE_ERROR("Scene has not been primed into reserve");
			}
			else {
				Ref<Scene> temp = scene;
				scene = reservedScene;
				reservedScene = temp;
			}
		}

		void SwitchScenes(bool drop) {
			if (reservedScene == nullptr) {
				MADAM_CORE_ERROR("Scene has not been primed into reserve");
			}
			else {
				Ref<Scene> temp = scene;
				scene = reservedScene;
				if (drop) {
					reservedScene = nullptr;
				}
				else {
					reservedScene = temp;
				}
			}
		}

		/*bool isTest() {
			bool temp = isTesting;
			isTesting = false;
			return temp;
		}

		void setTest() {
			isTesting = true;
		}*/

		bool isUpdate() {
			bool temp = isUpdating;
			isUpdating = false;
			return temp;
		}

		void setUpdate() {
			isUpdating = true;
		}

		bool getScripts() {
			bool temp = isGettingScripts;
			isGettingScripts = false;
			return temp;
		}

		void setScripts() {
			isGettingScripts = true;
		}

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void run();

		void quit();

		Scope<EngineInterface> pSurface = nullptr;

	private:
		static Application* instance;
		static bool instanceFlag;
		ApplicationConfig config;

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
		Ref<Scene> scene;
		Ref<Scene> reservedScene = nullptr;
		SceneSerializer* pSceneSerializer;
	protected:
		
	};

	//Defined by client
	Application* CreateApplication();
	
}