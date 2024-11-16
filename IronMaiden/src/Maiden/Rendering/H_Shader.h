#pragma once

#define INCLUDE_UTILS
#include "../Utils/H_Utils.h"

namespace Madam
{
	enum class PipelineStage
	{
		None = -1,
		Vert,
		TessCS,
		TessES,
		Geo,
		Frag,
		Comp
	};

	template<typename T>
	PipelineStage StringToPipelineStage(T str)
	{
		if (str == "vert")
		{
			return PipelineStage::Vert;
		}
		else if (str == "frag")
		{
			return PipelineStage::Frag;
		}
		else if (str == "comp")
		{
			return PipelineStage::Comp;
		}
		else if (str == "tess")
		{
			return PipelineStage::TessES;
		}
		else if (str == "geo")
		{
			return PipelineStage::Geo;
		}
		else
		{
			return PipelineStage::None;
		}
	}

	class Shader
	{
	public:
		virtual ~Shader() {}

		static Ref<Shader> Create(const std::filesystem::path& filepath);
		static Ref<Shader> Create(const std::vector<uint32_t> _code);

		virtual void Load() = 0;
	protected:
		std::filesystem::path _filepath;
	};
}
