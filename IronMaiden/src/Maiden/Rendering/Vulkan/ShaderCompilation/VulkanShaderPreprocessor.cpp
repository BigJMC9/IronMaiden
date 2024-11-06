#include "maidenpch.hpp"
#include "H_VulkanShaderPreprocessor.h"

#include <filesystem>
#include <format>
#include <sstream>

namespace Madam
{
	std::map<VkShaderStageFlagBits, std::string> VulkanPreprocessor::PreProcessShader(std::string& contents)
	{
		std::stringstream ss;
		PreprocessUtils::CopyWithoutComments(contents.begin(), contents.end(), std::ostream_iterator<char>(ss));
		std::string source = ss.str();

		std::map<VkShaderStageFlagBits, std::string> shaderSources;
		std::vector<std::pair<VkShaderStageFlagBits, size_t>> stagePositions;

		size_t startOfStage = 0;
		size_t pos = source.find('#', 0);

		while (pos != std::string::npos) {
			const size_t endOfLine = source.find_first_of("\r\n", pos) + 1;

			auto tokens = SplitString(source.substr(pos, endOfLine - pos));

			uint32_t index = 1;

			if (tokens[index] == "pragma") {
				++index;
				if (tokens[index] == "stage") {
					++index;

					const std::string_view stage = tokens[index];
					MADAM_CORE_ASSERT(stage == "vert" || stage == "frag" || stage == "comp", "Invalid shader type specified!");

					VkShaderStageFlagBits shaderStage;
					if (stage == "vert")
					{
						shaderStage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
					}
					else if (stage == "frag")
					{
						shaderStage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
					}
					else if (stage == "comp")
					{
						shaderStage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
					}
					stagePositions.emplace_back(shaderStage, startOfStage);
				}
			}
			startOfStage = pos;
			pos = source.find('#', pos + 1);
		}

		MADAM_CORE_ASSERT(stagePositions.size(), "Could not preprocess stages!");
		auto& [firstStage, firstStagePos] = stagePositions[0];
		if (stagePositions.size() > 1)
		{
			//Get first stage
			const std::string firstStageStr = source.substr(0, stagePositions[1].second);
			shaderSources[firstStage] = firstStageStr;


			//Get stages in the middle
			for (size_t i = 1; i < stagePositions.size() - 1; ++i)
			{
				auto& [stage, stagePos] = stagePositions[i];
				std::string stageStr = source.substr(stagePos, stagePositions[i + 1].second - stagePos);
				shaderSources[stage] = stageStr;
			}

			//Get last stage
			auto& [stage, stagePos] = stagePositions[stagePositions.size() - 1];
			std::string lastStageStr = source.substr(stagePos);
			shaderSources[stage] = lastStageStr;
		}
		else
		{
			shaderSources[firstStage] = source;
		}

		return shaderSources;
	}
}