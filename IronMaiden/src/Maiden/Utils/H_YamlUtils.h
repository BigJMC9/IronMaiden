#ifndef MADAM_DYNAMIC_LINK
#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#endif
#include <yaml-cpp/yaml.h>

#ifndef H_YAMLUTILS_INCLUDE_UTILS_GUARD
#ifdef INCLUDE_UTILS

#include "H_EngineUtils.h"

namespace YAML {
	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<>
	struct convert<std::filesystem::path>
	{
		static Node encode(const std::filesystem::path& rhs)
		{
			Node node;
			node.push_back(rhs.string());
			return node;
		}

		static bool decode(const Node& node, std::filesystem::path& rhs)
		{
			rhs = std::filesystem::u8path(node.as<std::string>());
			return true;
		}
	};

	template<>
	struct convert<Madam::UUID>
	{
		static Node encode(const Madam::UUID& uuid)
		{
			Node node;
			node.push_back((std::string)uuid);
			return node;
		}

		static bool decode(const Node& node, Madam::UUID& uuid)
		{
			uuid = Madam::UUID(node.as<std::string>());
			return true;
		}
	};
}
#define H_YAMLUTILS_INCLUDE_UTILS_GUARD
#endif // INCLUDE_UTILS
#endif // !H_YAMLUTILS_INCLUDE_UTILS_GUARD

#ifndef H_YAMLUTILS_INCLUDE_GLM_GUARD
#if defined(INCLUDE_GLM) 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace YAML {
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::quat& rhs)
		{
			if (!node.IsSequence() || node.size() > 4 || node.size() < 3)
			{
				return false;
			}

			if (node.size() == 3)
			{
				glm::vec3 angles;
				angles.x = node[0].as<float>();
				angles.y = node[1].as<float>();
				angles.z = node[2].as<float>();
				rhs = glm::quat(angles);
				return true;
			}
			else
			{
				rhs.x = node[0].as<float>();
				rhs.y = node[1].as<float>();
				rhs.z = node[2].as<float>();
				rhs.w = node[3].as<float>();
				return true;
			}
		}
	};
}
#define H_YAMLUTILS_INCLUDE_GLM_GUARD
#endif // INCLUDE_GLM
#endif // !H_YAMLUTILS_INCLUDE_GLM_GUARD