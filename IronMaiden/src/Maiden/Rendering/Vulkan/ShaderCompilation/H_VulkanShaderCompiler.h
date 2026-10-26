#pragma once
#include "shaderc/shaderc.hpp"
#include "../H_VulkanDevice.hpp"
#include "../../H_Shader.h"

namespace Madam
{
	class VulkanShaderCompiler
	{
	public:
		static std::map<VkShaderStageFlagBits, std::vector<uint32_t>> Compile(const std::filesystem::path& shaderSourcePath);
		static std::vector<uint32_t> CompileStage(const std::filesystem::path& path, VkShaderStageFlagBits stage,
			std::string* diagnostic = nullptr);

	private:
		static std::vector<uint32_t> CompileToSPV(const std::string& fileName, const std::string& source,
			shaderc_shader_kind kind, std::string* diagnostic = nullptr);
	};
}
