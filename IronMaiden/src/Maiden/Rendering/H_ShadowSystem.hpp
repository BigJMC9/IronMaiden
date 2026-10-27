#pragma once

#include "H_Buffer.hpp"
#include "H_Pipeline.hpp"
#include "H_RenderScene.hpp"
#include "FrameInfo.hpp"
#include "../Scene/H_Scene.hpp"

#include <array>
#include <filesystem>
#include <unordered_map>

namespace Madam { class StaticMesh; struct LightShadowSettings; }

namespace Madam::Rendering
{
	class ShadowSystem
	{
	public:
		static constexpr uint32_t AtlasSize = 4096;
		static constexpr uint32_t DirectionalAtlasSize = 2048;
		static constexpr uint32_t MinimumTile = 256;
		static constexpr uint32_t CascadeCount = 3;
		struct FrameResult
		{
			uint32_t drawCalls = 0;
			uint32_t activeTiles = 0;
			uint32_t droppedLights = 0;
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
		VkDescriptorImageInfo RealtimeDepthDescriptor(uint32_t frameIndex) const;
		VkDescriptorImageInfo BakedDepthDescriptor() const;
		VkDescriptorImageInfo DirectionalDescriptor(uint32_t frameIndex) const;
		VkDescriptorImageInfo DirectionalDepthDescriptor(uint32_t frameIndex) const;

	private:
		struct Tile
		{
			uint32_t x = 0;
			uint32_t y = 0;
			uint32_t size = 0;
			bool operator==(const Tile& other) const
			{
				return x == other.x && y == other.y && size == other.size;
			}
		};
		struct CachedTile
		{
			Tile tile{};
			uint64_t signature = 0;
			glm::mat4 matrix{ 1.0f };
		};
		struct Map
		{
			uint32_t size = 0;
			VkImage image = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
			VkImageView view = VK_NULL_HANDLE;
			VkFramebuffer framebuffer = VK_NULL_HANDLE;
			std::unordered_map<std::string, CachedTile> cached;
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
			uint32_t version = 4;
			uint32_t atlasSize = AtlasSize;
			uint32_t recordCount = 0;
		};
		struct BakeRecord
		{
			char key[96]{};
			Tile tile{};
			glm::mat4 matrix{ 1.0f };
		};

		void CreateMap(Map& map, uint32_t size);
		void DestroyMap(Map& map);
		void RenderTile(Map& map, Tile tile, const glm::mat4& lightViewProjection,
			const LightShadowSettings& settings, VkCommandBuffer commandBuffer);
		void CopyBakeToBuffer(VkCommandBuffer commandBuffer);
		bool LoadBake(const std::filesystem::path& path);
		void FinalizeBake(Scene& scene);
		static std::filesystem::path ResolveBakePath(const Scene& scene);

		Device& device;
		VkFormat depthFormat = VK_FORMAT_UNDEFINED;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		Scope<Pipeline> pipeline;
		VkSampler compareSampler = VK_NULL_HANDLE;
		VkSampler depthSampler = VK_NULL_HANDLE;
		std::array<Map, 2> realtimeAtlases;
		std::array<Map, 2> directionalAtlases;
		Map bakedAtlas;
		Scope<Buffer> bakeReadback;
		std::vector<ShadowDraw> drawList;
		std::filesystem::path loadedBakePath;
		std::filesystem::path pendingBakePath;
		bool pendingReadback = false;
		bool initialized = false;
	};
}
