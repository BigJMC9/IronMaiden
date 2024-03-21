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

	struct ApplicationConfig
	{
		std::string name = "IronMaidenEngine";
		std::string windowName = "Iron Maiden Engine";
		std::string version = "0.05";
		uint32_t windowWidth = 1600, windowHeight = 900;
		std::string workingDirectory;
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

		void addSurface(Surface& surface) {
			pSurface = std::make_unique<Surface>(surface);
		}

		static Application& Get() {
			//static Application instance;
			MADAM_CORE_ASSERT(instanceFlag, "Application instance not created");
			return *instance;
		}
		Window& getWindow() { return window;  }
		Device& getDevice() { return device; }
		Rendering::RenderStack& getMasterRenderSystem() { return renderStack; }
		const std::vector<std::shared_ptr<Rendering::RenderLayer>>& getRenderLayers() const;
		Rendering::Renderer& getRenderer() { return renderer; }
		Scene& getScene() { return *scene; }
		Time& getTime() { return time; }
		ApplicationConfig getConfig() {
			return config;
		}
		
		//Put in engine config header
		const float MAX_FRAME_TIME = 0.1f;

		App::PipeHandler pipeHandler;

		bool debug = false;

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void run();

		bool isRuntime = false;
		std::unique_ptr<Camera> pCamera;

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

		bool firstFrame = true;
		bool isRunning = false;

		std::unique_ptr<Scene> scene;
		SceneSerializer* pSceneSerializer;
	protected:
		std::unique_ptr<Surface> pSurface;
	};

	//Defined by client
	Application* CreateApplication();
	
}