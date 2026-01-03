#pragma once

#include "maidenpch.hpp"
#include "../Core/Main/Core.hpp"
#include "Vulkan/H_VulkanDevice.hpp"
#include "FrameInfo.hpp"

namespace Madam {

	struct PipelineConfigInfo {
		//Does cause some validation errors but generally works
		PipelineConfigInfo() {
			inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
			inputAssemblyInfo.pNext = nullptr;
			inputAssemblyInfo.flags = 0;

			viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportInfo.viewportCount = 1;
			viewportInfo.pViewports = nullptr;
			viewportInfo.scissorCount = 1;
			viewportInfo.pScissors = nullptr;
			viewportInfo.pNext = nullptr;
			viewportInfo.flags = 0;

			rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationInfo.depthClampEnable = VK_FALSE;
			rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
			rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationInfo.lineWidth = 1.0f;
			rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
			rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizationInfo.depthBiasEnable = VK_FALSE;
			rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
			rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
			rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional
			rasterizationInfo.pNext = nullptr;
			rasterizationInfo.flags = 0;

			multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleInfo.sampleShadingEnable = VK_FALSE;
			multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampleInfo.minSampleShading = 1.0f;           // Optional
			multisampleInfo.pSampleMask = nullptr;             // Optional
			multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
			multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional
			multisampleInfo.pNext = nullptr;
			multisampleInfo.flags = 0;

			colorBlendAttachment.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

			colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendInfo.logicOpEnable = VK_FALSE;
			colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
			colorBlendInfo.attachmentCount = 1;
			colorBlendInfo.pAttachments = &colorBlendAttachment;
			colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
			colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
			colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
			colorBlendInfo.blendConstants[3] = 0.0f;  // Optional
			colorBlendInfo.pNext = nullptr;
			colorBlendInfo.flags = 0;

			depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilInfo.depthTestEnable = VK_TRUE;
			depthStencilInfo.depthWriteEnable = VK_TRUE;
			depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
			depthStencilInfo.minDepthBounds = 0.0f;  // Optional
			depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
			depthStencilInfo.stencilTestEnable = VK_FALSE;
			depthStencilInfo.front = {};  // Optional
			depthStencilInfo.back = {};   // Optional
			depthStencilInfo.pNext = nullptr;
			depthStencilInfo.flags = 0;

			dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicStateInfo.pDynamicStates = dynamicStateEnables.data();
			dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
			dynamicStateInfo.pNext = nullptr;
			dynamicStateInfo.flags = 0;
		}

		void enableAlphaBlending() {
			colorBlendAttachment.blendEnable = VK_TRUE;

			colorBlendAttachment.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		}
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	struct ShaderModuleConfigInfo {
	public:
		explicit ShaderModuleConfigInfo(const std::string rawFilepath)
		{
			data = readFile(rawFilepath);
		}

		explicit ShaderModuleConfigInfo(const std::vector<uint32_t> rawData) : data{ rawData }
		{

		}

		bool operator==(const null_t) const {
			return data.size() == 0;
		}
		bool operator!=(const null_t) const {
			return data.size() != 0;
		}

		std::vector<uint32_t> data;
	private:
		ShaderModuleConfigInfo() = default;

		std::vector<uint32_t> readFile(const std::string& filepath) {
			std::ifstream file{ filepath, std::ios::ate | std::ios::binary };
			if (!file.is_open()) {
				throw std::runtime_error("failed to open file: " + filepath);
			}
			size_t fileSize = static_cast<size_t>(file.tellg());
			if (fileSize % sizeof(uint32_t) != 0) {
				throw std::runtime_error("File size is not a multiple of uint32_t size");
			}
			std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
			file.seekg(0);
			file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

			file.close();
			return buffer;
		}
		friend struct ShaderStageConfigInfo;
	};

	struct ShaderStageConfigInfo {
	public:
		ShaderModuleConfigInfo vertexModule;
		ShaderModuleConfigInfo tessellationControlModule;
		ShaderModuleConfigInfo tessellationEvaluationModule;
		ShaderModuleConfigInfo geometryModule;
		ShaderModuleConfigInfo fragmentModule;
		ShaderModuleConfigInfo computeModule;
		ShaderModuleConfigInfo rayGenerationModule;
		ShaderModuleConfigInfo anyHitModule;
		ShaderModuleConfigInfo intersectionModule;
		ShaderModuleConfigInfo closestHitModule;
		ShaderModuleConfigInfo missModule;
		ShaderModuleConfigInfo callableModule;
	};

	class MADAM_API Pipeline {
	public:
		Pipeline(Device& device, const PipelineConfigInfo& pipelineConfigInfo, const ShaderStageConfigInfo& shaderConfigInfo);
		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline &operator=(const Pipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);

		static void setDescriptions(PipelineConfigInfo& pipelineConfigInfo);
		static void enableAlphaBlending(PipelineConfigInfo& pipelineConfigInfo);
		static void debugPipelineConfigInfo(const PipelineConfigInfo& pipelineConfigInfo);

		static bool createShaderModule(Device& device, const ShaderModuleConfigInfo& configInfo, VkShaderModule* shaderModule);
		static void destroyShaderModule(Device& device, VkShaderModule shaderModule);

	private:
		static std::vector<char> readFile(const std::string& filepath);

		void createGraphicsPipeline(const PipelineConfigInfo& configInfo, const ShaderStageConfigInfo& shaderConfigInfo);
		

		Device& device;
		VkPipeline graphicsPipeline;
		VkShaderModule vertexShaderModule = nullptr;
		VkShaderModule tessellationControlShaderModule = nullptr;
		VkShaderModule tessellationEvaluationShaderModule = nullptr;
		VkShaderModule geometryShaderModule = nullptr;
		VkShaderModule fragmentShaderModule = nullptr;
		VkShaderModule computeShaderModule = nullptr;
		VkShaderModule rayGenerationShaderModule = nullptr;
		VkShaderModule anyHitShaderModule = nullptr;
		VkShaderModule intersectionShaderModule = nullptr;
		VkShaderModule closestHitShaderModule = nullptr;
		VkShaderModule missShaderModule = nullptr;
		VkShaderModule callableShaderModule = nullptr;
	};
}