#include "maidenpch.hpp"
#include "H_DescriptorManager.h"

namespace Madam
{
	/*DescPool::DescPool(Device& device, const DescPoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags) : _device{device}
	{
		std::vector<VkDescriptorPoolSize> sizes;
		sizes.reserve(poolSizes.sizes.size());
		for (auto sz : poolSizes.sizes) {
			sizes.push_back({ sz.first, uint32_t(sz.second * count) });
		}
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = flags;
		pool_info.maxSets = count;
		pool_info.poolSizeCount = (uint32_t)sizes.size();
		pool_info.pPoolSizes = sizes.data();

		vkCreateDescriptorPool(_device.device(), &pool_info, nullptr, &pool);
	}

	void DescPool::ReleasePool()
	{
		vkDestroyDescriptorPool(_device.device(), pool, nullptr);
	}

	void DescPool::Release(DescSet& set)
	{
		allocatedSets--;
	}

	bool DescPool::Allocate(DescSet& set)
	{
		allocatedSets++;
		return true;
	}*/

	DescWriter::DescWriter(DescSet& descriptorSet, Device* device) : descSet { descriptorSet }
	{
		descSet.device = device;
	}

	DescWriter& DescWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
		MADAM_CORE_ASSERT(descSet.layout->bindings.count(binding) == 1, "Layout does not contain specified binding");

		auto& bindingDescription = descSet.layout->bindings[binding];

		MADAM_CORE_ASSERT(bindingDescription.descriptorCount == 1, "Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = 1;

		writes.push_back(write);
		return *this;
	}

	DescWriter& DescWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) {
		MADAM_CORE_ASSERT(descSet.layout->bindings.count(binding) == 1, "Layout does not contain specified binding");

		auto& bindingDescription = descSet.layout->bindings[binding];

		MADAM_CORE_ASSERT(bindingDescription.descriptorCount == 1, "Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = imageInfo;
		write.descriptorCount = 1;

		writes.push_back(write);
		return *this;
	}

	bool DescWriter::build() {

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = *descSet.pool;
		allocInfo.pSetLayouts = &descSet.layout->descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
		// a new pool whenever an old pool fills up. But this is beyond our current scope
		if (vkAllocateDescriptorSets(descSet.device->device(), &allocInfo, &descSet.set) != VK_SUCCESS) {
			return false;
		}
		overwrite();
		return true;
	}

	void DescWriter::overwrite() {
		for (auto& write : writes) {
			write.dstSet = descSet.set;
		}
		vkUpdateDescriptorSets(descSet.device->device(), writes.size(), writes.data(), 0, nullptr);
	}

	DescManager::DescManager(Device& device) : _device{device}
	{

	}

	void DescManager::Cleanup()
	{
		for (auto p : pools)
		{
			vkDestroyDescriptorPool(_device.device(), p, nullptr);
		}
	}

	bool DescManager::Allocate(DescSet& descSet)
	{
		if (pools.size() > 0)
		{
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;

			allocInfo.pSetLayouts = &descSet.layout->descriptorSetLayout;
			allocInfo.descriptorSetCount = 1;

			for (size_t i = 0; i < pools.size(); i++)
			{
				allocInfo.descriptorPool = pools.front();

				//try to allocate the descriptor set
				VkResult allocResult = vkAllocateDescriptorSets(_device.device(), &allocInfo, &descSet.set);

				switch (allocResult) {
				case VK_SUCCESS:
					//all good, return
					return true;
				case VK_ERROR_FRAGMENTED_POOL:
					return false;
				case VK_ERROR_OUT_OF_POOL_MEMORY:
					//reallocate pool
					break;
				default:
					//unrecoverable error
					return false;
				}

				VkDescriptorPool pool = pools.back();
				pools.pop_back();
				pools.push_front(pool);
			}
			
			//Falls through if all pools are full
			pools.push_front(CreatePool(descriptorSizes, maxSet, 0));

			allocInfo.descriptorPool = pools.front();

			//try to allocate the descriptor set
			VkResult allocResult = vkAllocateDescriptorSets(_device.device(), &allocInfo, &descSet.set);

			switch (allocResult) {
			case VK_SUCCESS:
				//all good, return
				return true;
			case VK_ERROR_FRAGMENTED_POOL:
				return false;
			case VK_ERROR_OUT_OF_POOL_MEMORY:
				return false;
			default:
				//unrecoverable error
				return false;
			}
		}
		else
		{
			pools.push_front(CreatePool(descriptorSizes, maxSet, 0));
		}
	}

	VkDescriptorPool DescManager::CreatePool(const DescPoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags)
	{
		std::vector<VkDescriptorPoolSize> sizes;
		sizes.reserve(poolSizes.sizes.size());
		for (auto sz : poolSizes.sizes) {
			sizes.push_back({ sz.first, uint32_t(sz.second * count) });
		}
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = flags;
		pool_info.maxSets = count;
		pool_info.poolSizeCount = (uint32_t)sizes.size();
		pool_info.pPoolSizes = sizes.data();

		VkDescriptorPool descriptorPool;
		vkCreateDescriptorPool(_device.device(), &pool_info, nullptr, &descriptorPool);

		return descriptorPool;
	}
}