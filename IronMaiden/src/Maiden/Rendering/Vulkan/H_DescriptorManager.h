#pragma once
#include "maidenpch.hpp"
#include "../../Core/H_Utils.hpp"
#include "../../Core/Main/Core.hpp"
#include "H_VulkanDevice.hpp"

namespace Madam
{
	struct DescPoolSizes {
		std::vector<std::pair<VkDescriptorType, float>> sizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
		};
	};
	//class DescPool;

	struct DescSet
	{
	public:
		VkDescriptorSet set;
		VkDescriptorSetLayout layout;
		VkDescriptorPool* pool;
	};

	/*class DescPool
	{
	public:
		DescPool(Device& device, const DescPoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags);
		~DescPool()
		{
			ReleasePool();
		}

		void ReleasePool();
		void Release(DescSet& set);
		bool Allocate(DescSet& set);

		size_t Count() const
		{
			return allocatedSets;
		}
	private:
		Device& _device;
		uint32_t allocatedSets;
		VkDescriptorPool pool;
	};*/

	class DescManager
	{
	private:
		DescManager(Device& device);
		~DescManager() 
		{
			Cleanup();
		}

		void Cleanup();
		bool Allocate(DescSet& set);

		VkDescriptorPool CreatePool(const DescPoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags);

		Device& _device;
		std::list<VkDescriptorPool> pools;
		DescPoolSizes descriptorSizes;
		uint32_t maxSet = 1024;

	};
}
// All Descriptor users tell Descriptor Manager what they need and what they're going to use, and 
// Descriptor Manager will allocate the needed resources and decide the pool size based on that
