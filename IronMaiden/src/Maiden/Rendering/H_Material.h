#pragma once
#include "../Core/H_CoreUtils.hpp"
#include "H_Shader.h"
namespace Madam
{
	enum class MaterialFlag
	{
		None = 1 << 0,
		DepthTest = 1 << 1,
		Blend = 1 << 2,
		TwoSided = 1 << 3,
		DisableShadowCasting = 1 << 4
	};

	class Material
	{
	public:
		static Ref<Material> Create(const Ref<Shader>& shader);
		virtual ~Material() {};

		virtual uint32_t GetFlags() const = 0;
		virtual void SetFlags(uint32_t flags) = 0;

		virtual bool GetFlag(MaterialFlag flag) const = 0;
		virtual void SetFlag(MaterialFlag flag, bool value = true) = 0;

		virtual Ref<Shader> GetShader() = 0;
	};
}