#include "maidenpch.hpp"
#include "H_VulkanShaderCompiler.h"
#include "H_VulkanShaderPreprocessor.h"

namespace Madam
{
	std::map<VkShaderStageFlagBits, std::vector<uint32_t>> VulkanShaderCompiler::Compile(const std::filesystem::path& shaderSourcePath)
	{
		std::map<VkShaderStageFlagBits, std::vector<uint32_t>> shaderCode;
		std::ifstream shaderFile(shaderSourcePath);
		if (!shaderFile) {
			MADAM_CORE_ERROR("Unable to open shader file!");
			return shaderCode;
		}
		std::stringstream buffer;
		buffer << shaderFile.rdbuf();
		shaderFile.close();
		std::string fileContents = buffer.str();
		std::map<VkShaderStageFlagBits, std::string> shaderStages = VulkanPreprocessor::PreProcessShader(fileContents);
		
		for (const auto& pair : shaderStages)
		{
			switch (pair.first)
			{
			case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT:
				shaderCode.emplace(VK_SHADER_STAGE_VERTEX_BIT, CompileToSPV(std::string(shaderSourcePath.stem().string() + "-vert"), pair.second, shaderc_shader_kind::shaderc_glsl_vertex_shader));
				break;
			case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT:
				shaderCode.emplace(VK_SHADER_STAGE_FRAGMENT_BIT, CompileToSPV(std::string(shaderSourcePath.stem().string() + "-vert"), pair.second, shaderc_shader_kind::shaderc_glsl_fragment_shader));
				break;
			case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT:
				shaderCode.emplace(VK_SHADER_STAGE_COMPUTE_BIT, CompileToSPV(std::string(shaderSourcePath.stem().string() + "-vert"), pair.second, shaderc_shader_kind::shaderc_glsl_compute_shader));
				break;
			default:
				break;
			}
		}

		return shaderCode;
	}

	std::vector<uint32_t> VulkanShaderCompiler::CompileToSPV(const std::string& fileName, const std::string& source, shaderc_shader_kind kind)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options; 
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, fileName.c_str(), options);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			MADAM_CORE_ERROR("Shader Compilation Error: {0}", result.GetErrorMessage());
		}

		// Return SPIR-V binary
		return { result.cbegin(), result.cend() };
	}
}