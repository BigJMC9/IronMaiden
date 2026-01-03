#include "maidenpch.hpp"
#include "H_Pipeline.hpp"
#include "../Core/H_Application.hpp"
#include "../Rendering/Vulkan/H_VulkanMesh.h"
#include "../Project/H_Project.h"
#include "../Asset/AssetUtils.h"

namespace Madam {

	bool Pipeline::createShaderModule(Device& device, const ShaderModuleConfigInfo& configInfo, VkShaderModule* shaderModule)
	{
		if (configInfo == null)
		{
			return false;
		}
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = configInfo.data.size() * sizeof(int32_t);
		createInfo.pCode = (uint32_t*)configInfo.data.data();
		VkResult result = vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule);
		if (result != VK_SUCCESS) {
			return false;
		}
		else
		{
			return true;
		}
	}

	void Pipeline::destroyShaderModule(Device& device, VkShaderModule shaderModule)
	{
		if (shaderModule != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(device.device(), shaderModule, nullptr);
			shaderModule = VK_NULL_HANDLE;
		}
	}

	//In future seperate the Pipeline and config part
	Pipeline::Pipeline(Device& device, const PipelineConfigInfo& pipelineConfigInfo, const ShaderStageConfigInfo& shaderConfigInfo) : device{ device } {
		createGraphicsPipeline(pipelineConfigInfo, shaderConfigInfo);
	}

	Pipeline::~Pipeline() {
		destroyShaderModule(device, vertexShaderModule);
		destroyShaderModule(device, tessellationControlShaderModule);
		destroyShaderModule(device, tessellationEvaluationShaderModule);
		destroyShaderModule(device, geometryShaderModule);
		destroyShaderModule(device, fragmentShaderModule);
		destroyShaderModule(device, computeShaderModule);
		destroyShaderModule(device, rayGenerationShaderModule);
		destroyShaderModule(device, anyHitShaderModule);
		destroyShaderModule(device, intersectionShaderModule);
		destroyShaderModule(device, closestHitShaderModule);
		destroyShaderModule(device, missShaderModule);
		destroyShaderModule(device, callableShaderModule);
		vkDestroyPipeline(device.device(), graphicsPipeline, nullptr);
		graphicsPipeline = VK_NULL_HANDLE;
	}

	std::vector<char> Pipeline::readFile(const std::string& filepath) {
		std::ifstream file{ filepath, std::ios::ate | std::ios::binary };

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file: " + filepath);
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}

	void Pipeline::createGraphicsPipeline(const PipelineConfigInfo& pipelineConfigInfo, const ShaderStageConfigInfo& shaderConfigInfo) {

		assert(pipelineConfigInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no pipelineLayout provided in configInfo");
		assert(pipelineConfigInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no renderPass provided in configInfo");

		createShaderModule(device, shaderConfigInfo.vertexModule, &vertexShaderModule);
		createShaderModule(device, shaderConfigInfo.geometryModule, &geometryShaderModule);
		createShaderModule(device, shaderConfigInfo.fragmentModule, &fragmentShaderModule);

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		
		if (vertexShaderModule != nullptr)
		{
			VkPipelineShaderStageCreateInfo vertShaderStage;
			vertShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStage.module = vertexShaderModule;
			vertShaderStage.pName = "main";
			vertShaderStage.flags = 0;
			vertShaderStage.pNext = nullptr;
			vertShaderStage.pSpecializationInfo = nullptr;
			shaderStages.push_back(vertShaderStage);
		}

		if (geometryShaderModule != nullptr)
		{
			VkPipelineShaderStageCreateInfo geomShaderStage;
			geomShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			geomShaderStage.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			geomShaderStage.module = geometryShaderModule;
			geomShaderStage.pName = "main";
			geomShaderStage.flags = 0;
			geomShaderStage.pNext = nullptr;
			geomShaderStage.pSpecializationInfo = nullptr;
			shaderStages.push_back(geomShaderStage);
		}

		if (fragmentShaderModule != nullptr)
		{
			VkPipelineShaderStageCreateInfo fragShaderStage;
			fragShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStage.module = fragmentShaderModule;
			fragShaderStage.pName = "main";
			fragShaderStage.flags = 0;
			fragShaderStage.pNext = nullptr;
			fragShaderStage.pSpecializationInfo = nullptr;
			shaderStages.push_back(fragShaderStage);
		}

		auto& bindingDescriptions = pipelineConfigInfo.bindingDescriptions;
		auto& attributeDesriptions = pipelineConfigInfo.attributeDescriptions;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDesriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDesriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();


		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &pipelineConfigInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &pipelineConfigInfo.viewportInfo;
		pipelineInfo.pRasterizationState = &pipelineConfigInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &pipelineConfigInfo.multisampleInfo;
		pipelineInfo.pColorBlendState = &pipelineConfigInfo.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &pipelineConfigInfo.depthStencilInfo;
		pipelineInfo.pDynamicState = &pipelineConfigInfo.dynamicStateInfo;

		pipelineInfo.layout = pipelineConfigInfo.pipelineLayout;
		pipelineInfo.renderPass = pipelineConfigInfo.renderPass;
		pipelineInfo.subpass = pipelineConfigInfo.subpass;

		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create graphics pipeline");
		}
	}

	void Pipeline::bind(VkCommandBuffer commandBuffer) {
		//graphicsPipeline maybe null
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	}

	void Pipeline::setDescriptions(PipelineConfigInfo& pipelineConfigInfo) {

		pipelineConfigInfo.bindingDescriptions = VulkanStaticMesh::Vertex::GetBindingDescriptions();
		pipelineConfigInfo.attributeDescriptions = VulkanStaticMesh::Vertex::GetAttributeDescriptions();
	}

	void Pipeline::enableAlphaBlending(PipelineConfigInfo& pipelineConfigInfo) {
		pipelineConfigInfo.colorBlendAttachment.blendEnable = VK_TRUE;
		pipelineConfigInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		pipelineConfigInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipelineConfigInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipelineConfigInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		pipelineConfigInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		pipelineConfigInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		pipelineConfigInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	void Pipeline::debugPipelineConfigInfo(const PipelineConfigInfo& configInfo) {
		MADAM_CORE_INFO("PipelineConfigInfo");
		MADAM_CORE_INFO("------------------");
		MADAM_CORE_INFO("InputAssemblyInfo");
		MADAM_CORE_INFO("stype: {0}", configInfo.inputAssemblyInfo.sType);
		MADAM_CORE_INFO("topology: {0}", configInfo.inputAssemblyInfo.topology);
		MADAM_CORE_INFO("primitiveRestartEnable: {0}", configInfo.inputAssemblyInfo.primitiveRestartEnable);
		MADAM_CORE_INFO("---------------------------------------");
		MADAM_CORE_INFO("ViewportInfo");
		MADAM_CORE_INFO("stype: {0}", configInfo.viewportInfo.sType);
		MADAM_CORE_INFO("viewportCount: {0}", configInfo.viewportInfo.viewportCount);
		MADAM_CORE_INFO("pViewports: {0}", MADAM_CORE_POINTER(configInfo.viewportInfo.pViewports));
		MADAM_CORE_INFO("scissorCount: {0}", configInfo.viewportInfo.scissorCount);
		MADAM_CORE_INFO("pScissors: {0}", MADAM_CORE_POINTER(configInfo.viewportInfo.pScissors));
		MADAM_CORE_INFO("---------------------------------------");
		MADAM_CORE_INFO("RasterizationInfo");
		MADAM_CORE_INFO("stype: {0}", configInfo.rasterizationInfo.sType);
		MADAM_CORE_INFO("depthClampEnable: {0}", configInfo.rasterizationInfo.depthClampEnable);
		MADAM_CORE_INFO("rasterizerDiscardEnable: {0}", configInfo.rasterizationInfo.rasterizerDiscardEnable);
		MADAM_CORE_INFO("polygonMode: {0}", configInfo.rasterizationInfo.polygonMode);
		MADAM_CORE_INFO("lineWidth: {0}", configInfo.rasterizationInfo.lineWidth);
		MADAM_CORE_INFO("cullMode: {0}", configInfo.rasterizationInfo.cullMode);
		MADAM_CORE_INFO("frontFace: {0}", configInfo.rasterizationInfo.frontFace);
		MADAM_CORE_INFO("depthBiasEnable: {0}", configInfo.rasterizationInfo.depthBiasEnable);
		MADAM_CORE_INFO("---------------------------------------");
		MADAM_CORE_INFO("MultisampleInfo");
		MADAM_CORE_INFO("stype: {0}", configInfo.multisampleInfo.sType);
		MADAM_CORE_INFO("sampleShadingEnable: {0}", configInfo.multisampleInfo.sampleShadingEnable);
		MADAM_CORE_INFO("rasterizationSamples: {0}", configInfo.multisampleInfo.rasterizationSamples);
		MADAM_CORE_INFO("minSampleShading: {0}", configInfo.multisampleInfo.minSampleShading);
		MADAM_CORE_INFO("pSampleMask: {0}", MADAM_CORE_POINTER(configInfo.multisampleInfo.pSampleMask));
		MADAM_CORE_INFO("alphaToCoverageEnable: {0}", configInfo.multisampleInfo.alphaToCoverageEnable);
		MADAM_CORE_INFO("alphaToOneEnable: {0}", configInfo.multisampleInfo.alphaToOneEnable);
		MADAM_CORE_INFO("---------------------------------------");
		MADAM_CORE_INFO("ColorBlendAttachment");
		MADAM_CORE_INFO("colorWriteMask: {0}", configInfo.colorBlendAttachment.colorWriteMask);
		MADAM_CORE_INFO("blendEnable: {0}", configInfo.colorBlendAttachment.blendEnable);
		MADAM_CORE_INFO("srcColorBlendFactor: {0}", configInfo.colorBlendAttachment.srcColorBlendFactor);
		MADAM_CORE_INFO("dstColorBlendFactor: {0}", configInfo.colorBlendAttachment.dstColorBlendFactor);
		MADAM_CORE_INFO("colorBlendOp: {0}", configInfo.colorBlendAttachment.colorBlendOp);
		MADAM_CORE_INFO("srcAlphaBlendFactor: {0}", configInfo.colorBlendAttachment.srcAlphaBlendFactor);
		MADAM_CORE_INFO("dstAlphaBlendFactor: {0}", configInfo.colorBlendAttachment.dstAlphaBlendFactor);
		MADAM_CORE_INFO("alphaBlendOp: {0}", configInfo.colorBlendAttachment.alphaBlendOp);
		MADAM_CORE_INFO("---------------------------------------");
		MADAM_CORE_INFO("ColorBlendInfo");
		MADAM_CORE_INFO("stype: {0}", configInfo.colorBlendInfo.sType);
		MADAM_CORE_INFO("logicOpEnable: {0}", configInfo.colorBlendInfo.logicOpEnable);
		MADAM_CORE_INFO("logicOp: {0}", configInfo.colorBlendInfo.logicOp);
		MADAM_CORE_INFO("attachmentCount: {0}", configInfo.colorBlendInfo.attachmentCount);
		MADAM_CORE_INFO("pAttachments: {0}", MADAM_CORE_POINTER(configInfo.colorBlendInfo.pAttachments));
		MADAM_CORE_INFO("blendConstants[0]: {0}", configInfo.colorBlendInfo.blendConstants[0]);
		MADAM_CORE_INFO("blendConstants[1]: {0}", configInfo.colorBlendInfo.blendConstants[1]);
		MADAM_CORE_INFO("blendConstants[2]: {0}", configInfo.colorBlendInfo.blendConstants[2]);
		MADAM_CORE_INFO("blendConstants[3]: {0}", configInfo.colorBlendInfo.blendConstants[3]);
		MADAM_CORE_INFO("---------------------------------------");
		MADAM_CORE_INFO("DepthStencilInfo");
		MADAM_CORE_INFO("stype: {0}", configInfo.depthStencilInfo.sType);
		MADAM_CORE_INFO("depthTestEnable: {0}", configInfo.depthStencilInfo.depthTestEnable);
		MADAM_CORE_INFO("depthWriteEnable: {0}", configInfo.depthStencilInfo.depthWriteEnable);
		MADAM_CORE_INFO("depthCompareOp: {0}", configInfo.depthStencilInfo.depthCompareOp);
		MADAM_CORE_INFO("depthBoundsTestEnable: {0}", configInfo.depthStencilInfo.depthBoundsTestEnable);
		MADAM_CORE_INFO("minDepthBounds: {0}", configInfo.depthStencilInfo.minDepthBounds);
		MADAM_CORE_INFO("maxDepthBounds: {0}", configInfo.depthStencilInfo.maxDepthBounds);
		MADAM_CORE_INFO("stencilTestEnable: {0}", configInfo.depthStencilInfo.stencilTestEnable);
		MADAM_CORE_INFO("---------------------------------------");
		MADAM_CORE_INFO("DynamicStateInfo");
		MADAM_CORE_INFO("stype: {0}", configInfo.dynamicStateInfo.sType);
		MADAM_CORE_INFO("dynamicStateCount: {0}", configInfo.dynamicStateInfo.dynamicStateCount);
		MADAM_CORE_INFO("pDynamicStates: {0}", MADAM_CORE_POINTER(configInfo.dynamicStateInfo.pDynamicStates));
		MADAM_CORE_INFO("flags: {0}", configInfo.dynamicStateInfo.flags);
		MADAM_CORE_INFO("---------------------------------------\n");
	}
}