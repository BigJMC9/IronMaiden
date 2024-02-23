#pragma once

//Fix these headers, only essential headers are needed, the rest can reside in the implementation
#include "maidenpch.hpp"
#include "Maiden/core.hpp"
#include "H_JCVB_base.hpp"
#include "H_JCVB_descriptors.hpp"
#include "H_JCVB_device.hpp"
#include "H_Game_Object.hpp"
#include "H_components.hpp"
#include "H_JCVB_renderer.hpp"
#include "H_JCVB_window.hpp"
#include "H_Render_systems.hpp"
#include "H_Scene.hpp"
#include "H_SceneSerializer.hpp"
#include "H_ProcessHandler.hpp"
#include "H_CmdHandler.hpp"

//int main();

namespace Madam {

	struct ApplicationConfig
	{
		std::string name = "IronMaidenEngine";
		std::string windowName = "Iron Maiden Engine";
		std::string version = "0.05";
		int windowWidth = 1600, windowHeight = 900;
		std::string workingDirectory;
		std::string internals = "Internal/";
	};

	class MADAM_API Application {
	protected:
		Application();
	public:
		virtual ~Application();
		static Application& Get() {
			static Application instance;
			return instance;
		}
		Window& getWindow() { return window;  }
		Device& getDevice() { return device; }
		Rendering::MasterRenderSystem& getMasterRenderSystem() { return masterRenderSystem; }
		const std::vector<std::shared_ptr<Rendering::RenderSystem>>& getRenderSystems() const { 
			return masterRenderSystem.getRenderSystems();
		}
		Rendering::Renderer& getRenderer() { return renderer; }
		ApplicationConfig getConfig() {
			return config;
		}

		//Put in engine config header
		const float MAX_FRAME_TIME = 0.1f;

		Madam::App::PipeHandler pipeHandler;

		bool debug = false;

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void run();

	private:

		//static Application& instance;
		//static bool instanceFlag;
		ApplicationConfig config;

		Window window = Window{ config.windowWidth, config.windowHeight, config.windowName + " - " + config.version};
		Device device = Device{ window };
		Rendering::Renderer renderer = Rendering::Renderer{window, device};
		Rendering::MasterRenderSystem masterRenderSystem = Rendering::MasterRenderSystem{ device, renderer };

		//Move this somewhere
		std::unique_ptr<JcvbDescriptorPool> globalPool{};
		std::vector<std::unique_ptr<JcvbDescriptorPool>> framePools;

		bool firstFrame = true;
		
		std::unique_ptr<Scene> scene;
		SceneSerializer* pSceneSerializer;
	};

	//Defined by client
	Application* CreateApplication();
}