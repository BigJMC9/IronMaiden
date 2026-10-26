#include "maidenpch.hpp"
#include "H_ShaderDefinition.h"

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>

namespace Madam
{
	ShaderDefinition ReadMaterialShader(const std::filesystem::path& fragmentPath,
		const std::filesystem::path& defaultVertexPath)
	{
		ShaderDefinition result;
		if (fragmentPath.extension() != ".frag")
		{
			result.error = "Select a fragment shader (.frag).";
			return result;
		}
		std::ifstream file(fragmentPath);
		if (!file)
		{
			result.error = "The fragment shader could not be opened.";
			return result;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string source = buffer.str();
		source = std::regex_replace(source, std::regex(R"(/\*[\s\S]*?\*/|//[^\n]*)"), "");

		const std::regex declaration(R"(layout\s*\(([^)]*)\)\s*uniform\s+([A-Za-z_][A-Za-z_0-9]*)\s+([A-Za-z_][A-Za-z_0-9]*)\s*;)");
		const std::regex setPattern(R"(\bset\s*=\s*([0-9]+))");
		const std::regex bindingPattern(R"(\bbinding\s*=\s*([0-9]+))");
		for (auto it = std::sregex_iterator(source.begin(), source.end(), declaration);
			it != std::sregex_iterator(); ++it)
		{
			const std::string layout = (*it)[1].str();
			std::smatch setMatch;
			std::smatch bindingMatch;
			if (!std::regex_search(layout, setMatch, setPattern) || setMatch[1] != "1")
				continue;
			if (!std::regex_search(layout, bindingMatch, bindingPattern))
			{
				result.error = "A material input is missing its binding number.";
				return result;
			}
			if ((*it)[2] != "sampler2D")
			{
				result.error = "Material set 1 currently supports sampler2D inputs only.";
				return result;
			}
			uint32_t binding = 0;
			try { binding = static_cast<uint32_t>(std::stoul(bindingMatch[1].str())); }
			catch (const std::exception&)
			{
				result.error = "A material input has an invalid binding number.";
				return result;
			}
			ShaderInput input{ (*it)[3].str(), binding };
			if (std::any_of(result.textures.begin(), result.textures.end(), [&](const ShaderInput& existing)
				{ return existing.binding == input.binding || existing.name == input.name; }))
			{
				result.error = "Material shader inputs must have unique names and bindings.";
				return result;
			}
			result.textures.push_back(std::move(input));
		}
		const std::regex anyUniform(R"(layout\s*\(([^)]*)\)\s*uniform\b)");
		size_t materialUniformCount = 0;
		for (auto it = std::sregex_iterator(source.begin(), source.end(), anyUniform);
			it != std::sregex_iterator(); ++it)
		{
			std::smatch setMatch;
			const std::string layout = (*it)[1].str();
			if (std::regex_search(layout, setMatch, setPattern) && setMatch[1] == "1")
				++materialUniformCount;
		}
		if (materialUniformCount != result.textures.size())
		{
			result.error = "Material set 1 currently supports individual sampler2D inputs only.";
			return result;
		}
		std::sort(result.textures.begin(), result.textures.end(),
			[](const ShaderInput& a, const ShaderInput& b) { return a.binding < b.binding; });
		result.fragmentPath = fragmentPath;
		result.vertexPath = fragmentPath;
		result.vertexPath.replace_extension(".vert");
		if (!std::filesystem::is_regular_file(result.vertexPath))
			result.vertexPath = defaultVertexPath;
		if (!std::filesystem::is_regular_file(result.vertexPath))
			result.error = "The vertex shader could not be found.";
		return result;
	}
}
