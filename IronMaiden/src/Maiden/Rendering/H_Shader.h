#pragma once
#include "../Core/H_Utils.hpp"
#include <filesystem>
namespace Madam
{
	enum class PipelineStage
	{
		Vert,
		TessCS,
		TessES,
		Geo,
		Frag,
		Comp
	};
	class Shader
	{
	public:
		virtual ~Shader() {}

		static Ref<Shader> Create(const std::filesystem::path& filepath);
		static Ref<Shader> Create(const std::vector<uint32_t> _code);

		virtual void Load() = 0;
	protected:
		std::vector<uint32_t> _code;
		std::filesystem::path _filepath;
	};
}
