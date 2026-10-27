#pragma once

//#include "H_JCVB_camera.hpp"
#include "maidenpch.hpp"
#include "H_DescriptorSetLayout.hpp"
#include "H_RenderScene.hpp"
//#include "H_Scene.hpp"
//#include "H_Game_Object.hpp"

//lib
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <entt/entt.hpp>

//Will remove in future
namespace Madam {

	class Scene;

	#define MAX_LIGHTS 10

	struct PointLightData
	{
		glm::vec4 position{}; // ignore w
		glm::vec4 color{}; // w is intensity
	};

	struct DirectionalLightData
	{
		glm::vec4 direction{};
		glm::vec4 color{};
	};

	struct SpotLightData
	{
		glm::vec4 position{};
		glm::vec4 direction{};
		glm::vec4 color{};
		glm::vec4 cone{}; // inner cosine, outer cosine, range
	};

	#define MAX_DIRECTIONAL_LIGHTS 2
	#define MAX_SPOT_LIGHTS 8

	//When making changes to buffer object structures(struct below) always double check the alignment rules
	/*
		Do note that with vec3 some older shader languages will require you to conform to vec4 size, this is a
		implementation issue of these languages. Some programmers may try to fix this by adding a extra variable
		for padding.
		Some languages will try to compensate by padding vec3 as a vec4 so it's important to check alignment!
		It may be worth to change all vec3 to vec4 in the future as some older shader language versions
		and gpus may not handle this well. (alignment issue)
	*/

	//Uniform buffer object
	struct GlobalUbo {
		glm::mat4 projection{ 1.f }; // mat4 is a 4x4 matrix, it will hold 64bytes of data.
		glm::mat4 view{ 1.f }; // mat4 is a 4x4 matrix, it will hold 64bytes of data.
		glm::mat4 inverseView{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f }; // w is intensity. Holds 16bytes of data. (NOT ESSENTIAL, SHOULD BE MOVED).
		PointLightData pointLights[MAX_LIGHTS]; // NOT ESSENTIAL TO ALL OBJ, SHOULD BE MOVED
		int numLights = 0;
		int numDirectionalLights = 0;
		int numSpotLights = 0;
		int lightPadding = 0;
		DirectionalLightData directionalLights[MAX_DIRECTIONAL_LIGHTS]{};
		SpotLightData spotLights[MAX_SPOT_LIGHTS]{};
		glm::mat4 shadowViewProjection{ 1.0f };
		glm::vec4 shadowParams{ 0.002f, 0.0f, 0.0f, 0.0f };
		glm::ivec4 shadowInfo{ 0 }; // mode, type (1 directional, 2 spot), light index
	};
	static_assert(offsetof(GlobalUbo, directionalLights) == 544);
	static_assert(offsetof(GlobalUbo, spotLights) == 608);
	static_assert(offsetof(GlobalUbo, shadowViewProjection) == 1120);
	static_assert(offsetof(GlobalUbo, shadowInfo) == 1200);
	static_assert(sizeof(GlobalUbo) == 1216);

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VkDescriptorSet globalDescriptorSet;
		DescriptorPool& frameDescriptorPool;
		//Map& gameObjects;
		Ref<Scene>& scene;
		Rendering::RenderScene& renderScene;
		GlobalUbo ubo;
	};

	//FrameInfo* globalFrameInfo;
}
