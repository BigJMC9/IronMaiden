#pragma once

#include "maidenpch.hpp"
#include "../Core/Main/Core.hpp"
#include "Vulkan/H_VulkanDevice.hpp"


namespace Madam {

    class Buffer {
    public:
        Buffer(Device& device, VkDeviceSize instance_size, uint32_t instance_count, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags, VkDeviceSize min_offset_alignment = 1);
        ~Buffer();

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap();

        void WriteToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void WriteToIndex(void* data, int index);
        VkResult FlushIndex(int index);
        VkDescriptorBufferInfo DescriptorInfoForIndex(int index);
        VkResult InvalidateIndex(int index);

        VkBuffer GetBuffer() const { return m_buffer; }
        void* GetMappedMemory() const { return m_mapped; }
        uint32_t GetInstanceCount() const { return m_instance_count; }
        VkDeviceSize GetInstanceSize() const { return m_instance_size; }
        VkDeviceSize GetAlignmentSize() const { return m_instance_size; }
        VkBufferUsageFlags GetUsageFlags() const { return m_usage_flags; }
        VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return m_memory_property_flags; }
        VkDeviceSize GetBufferSize() const { return m_buffer_size; }

    private:
        static VkDeviceSize GetAlignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment);

        Device& m_device;
        void* m_mapped = nullptr;
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_memory = VK_NULL_HANDLE;

        VkDeviceSize m_buffer_size;
        uint32_t m_instance_count;
        VkDeviceSize m_instance_size;
        VkDeviceSize m_alignment_size;
        VkBufferUsageFlags m_usage_flags;
        VkMemoryPropertyFlags m_memory_property_flags;
    };

}