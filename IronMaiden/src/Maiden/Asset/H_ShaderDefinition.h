#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace Madam
{
	struct ShaderInput
	{
		std::string name;
		uint32_t binding = 0;
	};

	struct ShaderDefinition
	{
		std::filesystem::path fragmentPath;
		std::filesystem::path vertexPath;
		std::vector<ShaderInput> textures;
		std::string error;
		bool IsValid() const { return error.empty() && !fragmentPath.empty(); }
	};

	// Material inputs are set 1 sampler2D declarations in the fragment shader.
	ShaderDefinition ReadMaterialShader(const std::filesystem::path& fragmentPath,
		const std::filesystem::path& defaultVertexPath);
}
