#pragma once

//Fix these headers, only essential headers are needed, the rest can reside in the implementation
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



// std
#include <memory>
#include <vector>

int main();

namespace Madam {
	/*namespace App {
		class PipeHandler;
	}*/
	class Application {

	public:
		//Put in app config header
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		

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

		//Put in app config header
		const float MAX_FRAME_TIME = 0.1f;

		Madam::App::PipeHandler pipeHandler;

		bool debug = false;

	private:
		//static Application* instance;

		Application();
		~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void run();
		
		Window window = Window{ WIDTH, HEIGHT, "Iron Maiden Engine - 0.04" };
		Device device = Device{ window };
		Rendering::Renderer renderer = Rendering::Renderer{window, device};
		Rendering::MasterRenderSystem masterRenderSystem = Rendering::MasterRenderSystem{ device, renderer };

		//Move this somewhere
		std::unique_ptr<JcvbDescriptorPool> globalPool{};
		std::vector<std::unique_ptr<JcvbDescriptorPool>> framePools;

		bool firstFrame = true;
		
		
		std::unique_ptr<Scene> scene;
		SceneSerializer* pSceneSerializer;
		
		friend int ::main();
	};
}