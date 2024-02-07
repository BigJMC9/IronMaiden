#include "H_first_app.hpp"

#include "H_keyboard_movement_controller.hpp"
#include "H_JCVB_buffer.hpp"
//#include "H_JCVB_camera.hpp"
//#include "H_simple_render_system.hpp"
//#include "H_point_light_system.hpp"
//#include "H_texture_render_system.hpp"
//#include "H_JCVB_mesh_renderer.hpp"
//#include "H_JCVB_material.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <chrono>
#include <array>
#include <cassert>
#include <thread>
#include <iostream>

namespace Digestion {

	FirstApp::FirstApp() {
		globalPool = 
			JcvbDescriptorPool::Builder(jcvbDevice)
				.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
				.build();

		// build frame descriptor pools
		framePools.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		auto framePoolBuilder = JcvbDescriptorPool::Builder(jcvbDevice)
			.setMaxSets(1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
		for (int i = 0; i < framePools.size(); i++) {
			framePools[i] = framePoolBuilder.build();
		}
		
		scene = new Scene(jcvbDevice);
		loadGameObjects();
	}

	FirstApp::~FirstApp() {

	}

	void FirstApp::run() {

		std::vector < std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<Buffer>(
				jcvbDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = JcvbDescriptorSetLayout::Builder(jcvbDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			JcvbDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		masterRenderSystem.initialize(globalSetLayout);


        Camera camera{};

        camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.f, 0.f, 0.f));
        //camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));
		Entity viewerObject = scene->CreateGameObject();
        //auto viewerObject = GameObject::createGameObject();
		viewerObject.GetComponent<Transform>().translation.z = -2.5f;
		//viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

		//entt::registry& entities = scene->Reg();
		//auto view = entities.view<Transform>();
		/*for (auto& kv : gameObjects) {
			kv.second.start();
		}*/
		while (!jcvbWindow.shouldClose()) {
			glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME);

            cameraController.moveInPlaneXZ(jcvbWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = jcvbRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

			if (auto commandBuffer = jcvbRenderer.beginFrame()) {
				int frameIndex = jcvbRenderer.getFrameIndex();
				framePools[frameIndex]->resetPool();
				GlobalUbo ubo{};
				FrameInfo frameInfo {
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					* framePools[frameIndex],
					scene,
					ubo};
				//delete globalFrameInfo;
				//globalFrameInfo = &frameInfo;
				// update
				/*for (auto& kv : gameObjects)
				{
					kv.second.update();
				}*/
				//GlobalUbo ubo{};
				frameInfo.ubo.projection = camera.getProjection();
				frameInfo.ubo.view = camera.getView();
				frameInfo.ubo.inverseView = camera.getInverseView();
				//pointLightSystem.update(frameInfo, ubo);
				masterRenderSystem.preRender(frameInfo);
				uboBuffers[frameIndex]->writeToBuffer(&frameInfo.ubo);
				uboBuffers[frameIndex]->flush();

				// render
				jcvbRenderer.beginSwapChainRenderPass(commandBuffer);

				//order here matters
				/*for (auto& kv : gameObjects)
				{
					kv.second.render();
				}*/
				//Master render system here
				masterRenderSystem.render(frameInfo);
				//textureRenderSystem.render(frameInfo);
				//simpleRenderSystem.render(frameInfo);
				//pointLightSystem.render(frameInfo);
				jcvbRenderer.endSwapChainRenderPass(commandBuffer);
				jcvbRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(jcvbDevice.device());
	}

	void FirstApp::loadGameObjects() {
		std::cout << "Loading GameObjects" << std::endl;

        std::shared_ptr<Model> flatModel = Model::createModelFromFile(jcvbDevice, "models/flat_vase.obj");
		Entity flatVase = scene->LoadGameObject(flatModel);
        //auto flatVase = GameObject::createGameObject("FlatVase");
		//auto meshRenderer = std::make_unique<JcvbMeshRenderer>(jcvbModel);
		//flatVase.addComponent(std::move(meshRenderer));
		flatVase.GetComponent<Transform>().translation = { -0.5f, .5f, 0.f };
		flatVase.GetComponent<Transform>().scale = { 3.f, 1.5f, 3.f };
        //flatVase.transform.translation = { -0.5f, .5f, 0.f };
		//flatVase.transform.scale = {3.f, 1.5f, 3.f};
        //gameObjects.emplace(flatVase.getId(), std::move(flatVase));

		std::shared_ptr<Model> smoothModel = Model::createModelFromFile(jcvbDevice, "models/smooth_vase.obj");
		Entity smoothVase = scene->LoadGameObject(smoothModel);
		//auto smoothVase = GameObject::createGameObject("SmoothVase");
		//meshRenderer = std::make_unique<JcvbMeshRenderer>(jcvbModel);
		//smoothVase.addComponent(std::move(meshRenderer));
		smoothVase.GetComponent<Transform>().translation = { 0.5f, .5f, 0.f };
		smoothVase.GetComponent<Transform>().scale = { 3.f, 1.5f, 3.f };
		//smoothVase.transform.translation = { 0.5f, .5f, 0.f };
		//smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
		//gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

		std::shared_ptr<Model> quadModel = Model::createModelFromFile(jcvbDevice, "models/quad.obj");
		std::shared_ptr<Shader> shader = std::make_shared<Shader>();
		shader->vertShaderPath = "shaders/texture_shader.vert.spv";
		shader->fragShaderPath = "shaders/texture_shader.frag.spv";
		std::shared_ptr<Material> sandMaterial = std::make_shared<Material>();

		//sandMaterial->vertShaderPath = "shaders/texture_shader.vert.spv";
		//sandMaterial->fragShaderPath = "shaders/texture_shader.frag.spv";
		sandMaterial->diffuseMap = Texture::createTextureFromFile(jcvbDevice, "textures/GroundSand005_COL_2K.jpg");
		sandMaterial->normalMap = Texture::createTextureFromFile(jcvbDevice, "textures/GroundSand005_NRM_2K.jpg");
		sandMaterial->ambientOcclusionMap = Texture::createTextureFromFile(jcvbDevice, "textures/GroundSand005_AO_2K.jpg");
		sandMaterial->glossMap = Texture::createTextureFromFile(jcvbDevice, "textures/GroundSand005_GLOSS_2K.jpg");
		//std::shared_ptr<JcvbTexture> sandTexture = JcvbTexture::createTextureFromFile(jcvbDevice, "textures/GroundSand005_COL_2K.jpg");
		//std::shared_ptr<JcvbTexture> sandNormalTexture = JcvbTexture::createTextureFromFile(jcvbDevice, "textures/GroundSand005_NRM_2K.jpg");
		//std::shared_ptr<JcvbTexture> sandAmbientOcclusionTexture = JcvbTexture::createTextureFromFile(jcvbDevice, "textures/GroundSand005_AO_2K.jpg");
		//std::shared_ptr<JcvbTexture> sandGlossTexture = JcvbTexture::createTextureFromFile(jcvbDevice, "textures/GroundSand005_GLOSS_2K.jpg");
		Entity floor = scene->LoadGameObject(quadModel, sandMaterial);
		//auto floor = GameObject::createGameObject("FlatPlane");
		//meshRenderer = std::make_unique<JcvbMeshRenderer>(jcvbModel, sandMaterial);
		//floor.addComponent(std::move(meshRenderer));
		//floor.diffuseMap = sandTexture;
		//floor.normalMap = sandNormalTexture;
		//floor.ambientOcclusionMap = sandAmbientOcclusionTexture;
		//floor.glossMap = sandGlossTexture;
		floor.GetComponent<Transform>().translation = { 0.f, .5f, 0.f };
		floor.GetComponent<Transform>().scale = { 3.f, 1.f, 3.f };
		//floor.transform.translation = { 0.f, .5f, 0.f };
		//floor.transform.scale = { 3.f, 1.f, 3.f };
		//gameObjects.emplace(floor.getId(), std::move(floor));
		
		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}
		};

		for (int i = 0; i < lightColors.size(); i++)
		{
			//std::string name = "Light " + std::to_string(i);
			Entity entity = scene->CreateGameObject();
			entity.AddComponent<PointLight>();
			//PointLight pointLight = entity.GetComponent<PointLight>();
			entity.GetComponent<PointLight>().color = lightColors[i];
			entity.GetComponent<PointLight>().intensity = 0.2f;
			entity.GetComponent<PointLight>().radius = 0.1f;
			auto rotateLight = glm::rotate(
				glm::mat4(1.f), 
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			entity.GetComponent<Transform>().translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			//std::cout << std::to_string(i) << ": ( " << std::to_string(pointLight.color.x) << ", " << std::to_string(pointLight.color.y) << ", " << std::to_string(pointLight.color.z) << " )" << std::endl;
			//entity.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			//gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}
	}
}