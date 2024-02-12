#pragma once

#include "H_JCVB_base.hpp"
#include "H_JCVB_descriptors.hpp"
#include "H_JCVB_device.hpp"
#include "H_Game_Object.hpp"
#include "H_components.hpp"
#include "H_Scene.hpp"
#include "H_SceneSerializer.hpp"
#include "H_ProcessHandler.hpp"
#include "H_CmdHandler.hpp"
//#include "H_JCVB_game_object.hpp"
#include "H_JCVB_renderer.hpp"
#include "H_JCVB_window.hpp"
#include "H_Render_systems.hpp"

//Define fixes
#ifdef min
#undef min
#endif


// std
#include <memory>
#include <vector>

namespace Digestion {
	class FirstApp {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		
		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp &) = delete;
		FirstApp &operator=(const FirstApp &) = delete;

		//GameObject CreateGameObject();
		//GameObject LoadGameObject(std::shared_ptr<Model> model);
		//GameObject LoadGameObject(std::shared_ptr<Model> model, std::shared_ptr<Material> mat);

		//entt::registry& Reg() { return registry; }



		const float MAX_FRAME_TIME = 0.1f;

		//Map gameObjects;
		void run();

		Digestion::App::PipeHandler pipeHandler;
	private:
		//entt::registry registry;
		std::vector<Model> models;
		void loadGameObjects();
		
		Window jcvbWindow = Window{ WIDTH, HEIGHT, "JCVB - 0.01" };
		Device jcvbDevice = Device{ jcvbWindow };
		Rendering::Renderer jcvbRenderer = Rendering::Renderer{jcvbWindow, jcvbDevice};
		Rendering::MasterRenderSystem masterRenderSystem = Rendering::MasterRenderSystem{ jcvbDevice, jcvbRenderer };

		//note: order of declaration matters
		std::unique_ptr<JcvbDescriptorPool> globalPool{};
		std::vector<std::unique_ptr<JcvbDescriptorPool>> framePools;
		
		Scene* scene;
		SceneSerializer* pSceneSerializer;
		//SceneSerializer sceneSerializer;
	};
}