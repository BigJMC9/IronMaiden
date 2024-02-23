#pragma once

#include "maidenpch.hpp"
#include "H_JCVB_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace Madam {

    class JcvbDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(Device& jcvbDevice) : jcvbDevice{ jcvbDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<JcvbDescriptorSetLayout> build() const;

        private:
            Device& jcvbDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        JcvbDescriptorSetLayout(
            Device& jcvbDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~JcvbDescriptorSetLayout();
        JcvbDescriptorSetLayout(const JcvbDescriptorSetLayout&) = delete;
        JcvbDescriptorSetLayout& operator=(const JcvbDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        Device& jcvbDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class JcvbDescriptorWriter;
    };

    class JcvbDescriptorPool {
    public:
        class Builder {
        public:
            Builder(Device& jcvbDevice) : jcvbDevice{ jcvbDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<JcvbDescriptorPool> build() const;

        private:
            Device& jcvbDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        JcvbDescriptorPool(
            Device& jcvbDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~JcvbDescriptorPool();
        JcvbDescriptorPool(const JcvbDescriptorPool&) = delete;
        JcvbDescriptorPool& operator=(const JcvbDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        Device& jcvbDevice;
        VkDescriptorPool descriptorPool;

        friend class JcvbDescriptorWriter;
    };

    class JcvbDescriptorWriter {
    public:
        JcvbDescriptorWriter(JcvbDescriptorSetLayout& setLayout, JcvbDescriptorPool& pool);

        JcvbDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        JcvbDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        JcvbDescriptorSetLayout& setLayout;
        JcvbDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };
}