#pragma once

#include "H_Buffer.hpp"
#include "H_Pipeline.hpp"
#include "H_RenderScene.hpp"
#include "FrameInfo.hpp"
#include "../Scene/H_Scene.hpp"

#include <array>
#include <filesystem>

namespace Madam { class StaticMesh; }

namespace Madam::Rendering
{
	// A single selected directional or spot light casts shadows. The realtime map is
	// double buffered; the baked map is kept separately and can be saved with a scene.
	class ShadowSystem
	{
	public:
		struct FrameResult
		{
			uint32_t drawCalls = 0;
			bool rendered = false;
			bool cached = false;
		};

		explicit ShadowSystem(Device& device) : device(device) {}
		~ShadowSystem() { Shutdown(); }
		void Init();
		void Shutdown();
		void SyncBake(Scene& scene);
		FrameResult Prepare(Scene& scene, const RenderScene& renderScene, GlobalUbo& ubo,
			uint32_t frameIndex, VkCommandBuffer commandBuffer);
		VkDescriptorImageInfo RealtimeDescriptor(uint32_t frameIndex) const;
		VkDescriptorImageInfo BakedDescriptor() const;
		static constexpr uint32_t Resolution = 2048;

	private:
		struct Map
		{
			VkImage image = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
			VkImageView view = VK_NULL_HANDLE;
			VkFramebuffer framebuffer = VK_NULL_HANDLE;
		};

		struct ShadowPush
		{
			glm::mat4 lightViewProjection{ 1.0f };
			glm::mat4 model{ 1.0f };
		};

		struct ShadowDraw
		{
			StaticMesh* mesh = nullptr;
			glm::mat4 model{ 1.0f };
		};

		struct BakeHeader
		{
			char magic[8] = { 'I', 'M', 'S', 'H', 'A', 'D', 'O', 'W' };
			uint32_t version = 2;
			uint32_t resolution = Resolution;
			glm::mat4 lightViewProjection{ 1.0f };
			char caster[64]{};
		};

		void CreateMap(Map& map);
		void DestroyMap(Map& map);
		void RenderMap(Map& map, const glm::mat4& lightViewProjection, VkCommandBuffer commandBuffer);
		void CopyBakeToBuffer(VkCommandBuffer commandBuffer);
		bool LoadBake(const std::filesystem::path& path);
		void FinalizeBake(Scene& scene);
		static std::filesystem::path ResolveBakePath(const Scene& scene);

		Device& device;
		VkFormat depthFormat = VK_FORMAT_UNDEFINED;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		Scope<Pipeline> pipeline;
		VkSampler sampler = VK_NULL_HANDLE;
		std::array<Map, 2> realtimeMaps;
		std::array<uint64_t, 2> realtimeSignatures{};
		std::array<bool, 2> realtimeValid{};
		std::vector<ShadowDraw> drawList;
		Map bakedMap;
		Scope<Buffer> bakeReadback;
		std::filesystem::path loadedBakePath;
		std::filesystem::path pendingBakePath;
		glm::mat4 bakedMatrix{ 1.0f };
		std::string bakedCaster;
		bool bakedValid = false;
		bool pendingReadback = false;
		bool initialized = false;
	};
}
