#pragma once

//Fix these headers, only essential headers are needed, the rest can reside in the implementation
#include "maidenpch.hpp"
#include "Main/core.hpp"
#include "H_Logger.hpp"
#include "../Rendering/H_Descriptors.hpp"
#include "../Rendering/H_Renderer.hpp"
#include "../Rendering/H_RenderSystems.hpp"
#include "../Scene/H_Scene.hpp"
#include "H_ProcessHandler.hpp"
#include "../Events/Event.hpp"
#include "H_Time.hpp"
#include "H_Surface.h"

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
		std::string projectFolder = "../Sandbox/"; //This should be the folder where the project is stored
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

		void onEvent(Event& e);

		float getAspectRatio() {
			return renderer.getAspectRatio();
		}

		void addSurface(std::unique_ptr<Surface> _surface) {
			pSurface = std::move(_surface);
			MADAM_CORE_INFO("Surface added");
		}

		static Application& Get() {
			//static Application instance;
			MADAM_CORE_ASSERT(instanceFlag, "Application instance not created");
			return *instance;
		}

		// Use const func() const {} for readonly vars

		Window& getWindow() { return window;  }
		Device& getDevice() { return device; }
		Rendering::RenderStack& getMasterRenderSystem() { return renderStack; }
		const std::vector<std::shared_ptr<Rendering::RenderLayer>>& getRenderLayers() const;
		Rendering::Renderer& getRenderer() { return renderer; }
		Scene& getScene() { return *scene; }
		const Time& getTime() const { return time; }
		ApplicationConfig getConfig() {
			return config;
		}
		
		//Put in engine config header
		const float MAX_FRAME_TIME = 0.1f;

		App::PipeHandler pipeHandler;

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
		void PrimeReserve(std::shared_ptr<Scene> _scene) {
			reservedScene = _scene;
		}

		void SwitchScenes() {
			if (reservedScene == nullptr) {
				MADAM_CORE_ERROR("Scene has not been primed into reserve");
			}
			else {
				std::shared_ptr<Scene> temp = scene;
				scene = reservedScene;
				reservedScene = temp;
			}
		}

		void SwitchScenes(bool drop) {
			if (reservedScene == nullptr) {
				MADAM_CORE_ERROR("Scene has not been primed into reserve");
			}
			else {
				std::shared_ptr<Scene> temp = scene;
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

		std::unique_ptr<Surface> pSurface = nullptr;

	private:

		static Application* instance;
		static bool instanceFlag;
		ApplicationConfig config;

		Window window = Window{};
		Device device = Device{ window };
		Rendering::Renderer renderer = Rendering::Renderer{window, device};
		Rendering::RenderStack renderStack = Rendering::RenderStack{ device, renderer };
		Time time = Time{};

		//Move this somewhere
		std::unique_ptr<DescriptorPool> globalPool{};
		std::vector<std::unique_ptr<DescriptorPool>> framePools;

		bool isRunning = false;
		bool firstFrame = true;
		//For Testing Remove when redundant
		std::string createNative = "";
		bool isScanning = true;
		bool isCompiling = false;
		bool runtime = false;
		bool runtimeFlag = false;
		bool isGettingScripts = false;
		//bool isTesting = false;
		bool isUpdating = false;

		//Need Scene Management class
		std::shared_ptr<Scene> scene;
		std::shared_ptr<Scene> reservedScene = nullptr;
		SceneSerializer* pSceneSerializer;
	protected:
		
	};

	//Defined by client
	Application* CreateApplication();
	
}