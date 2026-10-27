#pragma once

#include "maidenpch.hpp"
#include <entt/entt.hpp>
#include <vector>

namespace Madam::Rendering
{
	struct RenderScene
	{
		entt::registry* registry = nullptr;
		std::vector<entt::entity> meshEntities;
		std::vector<entt::entity> pointLightEntities;
		std::vector<entt::entity> directionalLightEntities;
		std::vector<entt::entity> spotLightEntities;

		void Clear()
		{
			registry = nullptr;
			meshEntities.clear();
			pointLightEntities.clear();
			directionalLightEntities.clear();
			spotLightEntities.clear();
		}
	};
}
