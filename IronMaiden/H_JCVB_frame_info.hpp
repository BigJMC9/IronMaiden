#pragma once

//#include "H_JCVB_camera.hpp"
#include "maidenpch.hpp"
#include "H_JCVB_descriptors.hpp"
//#include "H_Scene.hpp"
//#include "H_Game_Object.hpp"

//lib
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <EnTT/entt.hpp>


namespace Madam {

	class Scene;

	struct Camera;

	#define MAX_LIGHTS 10

	struct PointLightData
	{
		glm::vec4 position{}; // ignore w
		glm::vec4 color{}; // w is intensity
	};

	//When making changes to buffer object structures(struct below) always double check the alignment rules
	/*
		Do note that with vec3 some older shader languages will require you to conform to vec4 size, this is a
		implementation issue of these languages. Some programmers may try to fix this by adding a extra variable
		for padding.
		Some languages will try to compensate by padding vec3 as a vec4 so it's important to check alignment!
		It may be worth to change all vec3 to vec4 in the future as some older shader language versions
		and gpus may not handle this well. (alignment issue)
	*/
	struct GlobalUbo {
		glm::mat4 projection{ 1.f }; // mat4 is a 4x4 matrix, it will hold 64bytes of data.
		glm::mat4 view{ 1.f }; // mat4 is a 4x4 matrix, it will hold 64bytes of data.
		glm::mat4 inverseView{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f }; // w is intensity. Holds 16bytes of data. (NOT ESSENTIAL, SHOULD BE MOVED).
		PointLightData pointLights[MAX_LIGHTS]; // NOT ESSENTIAL TO ALL OBJ, SHOULD BE MOVED
		int numLights;
	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		Camera& camera;
		VkDescriptorSet globalDescriptorSet;
		JcvbDescriptorPool& frameDescriptorPool;
		//Map& gameObjects;
		std::unique_ptr<Scene>& scene;
		GlobalUbo ubo;
	};

	//FrameInfo* globalFrameInfo;
}