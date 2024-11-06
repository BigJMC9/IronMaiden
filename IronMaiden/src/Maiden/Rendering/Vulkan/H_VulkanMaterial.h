#pragma once
#include "../H_Material.h"
#include "H_VulkanShader.h"
#include "../H_Pipeline.hpp"

namespace Madam
{
	class VulkanMaterial : public Material
	{
	public:
		VulkanMaterial(const Ref<Shader>& shader);
		~VulkanMaterial() override;

		Ref<Shader> GetShader() override;

		uint32_t GetFlags() const override { return 1; }
		void SetFlags(uint32_t flags) override {}

		bool GetFlag(MaterialFlag flag) const override { return true;}
		void SetFlag(MaterialFlag flag, bool value = true) override {}

	private:
		Ref<VulkanShader> _shader;
		Pipeline* pipeline = nullptr;
	};
}