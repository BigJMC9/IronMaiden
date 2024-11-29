#pragma once
#include "Main/Core.hpp"
#include "maidenpch.hpp"

namespace Madam {

	// from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

	class Entity;

	using id_t = unsigned int;
	using Map = std::unordered_map<id_t, Entity>;

	struct UUID {

	public:
		UUID();

		UUID(std::string uuid);
		UUID(const UUID&) = default;

		operator std::string() const { return _UUID; }

		friend std::ostream& operator<<(std::ostream& os, const UUID& uuid) {
			os << "UUID: " << uuid._UUID;
			return os;
		}

		friend bool operator==(const UUID& left, const UUID& right) {
			return left._UUID == right._UUID;
		}

		friend bool operator!=(const UUID& left, const UUID& right) {
			return left._UUID != right._UUID;
		}
		
		friend bool operator==(const UUID& left, const std::string& right) {
			return left._UUID == right;
		}

		friend bool operator!=(const UUID& left, const std::string& right) {
			return left._UUID != right;
		}

	private:
		std::string _UUID;
	};

	//Add Custom Smart Pointer that is shared. When Owner is destroyed and reference is > 0, throw error

	//Add Custom Smart Pointer that is owned but can be shared. When Owner is destroyed and reference is > 0, Tell garbage collector to delete it and set the pointer to nullptr.
	//Maybe Garbage Collector use a tree to keep track of which memory address each pointer points to.

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

#define BIT(x) (1u << x)

	inline std::vector<std::string> SplitString(std::string str, bool keepDelimeters = false)
	{
		if (keepDelimeters)
		{
			const static std::regex re(R"((^\W|^\w+)|(\w+)|[:()])", std::regex_constants::optimize);

			std::regex_iterator<std::string::iterator> rit(str.begin(), str.end(), re);
			std::regex_iterator<std::string::iterator> rend;
			std::vector<std::string> result;

			while (rit != rend)
			{
				result.emplace_back(rit->str());
				++rit;
			}
			return result;
		}
		else
		{
			std::vector<std::string> str;
			return str;
		}
	}
}

namespace std {

	template <typename T> struct hash;

	template<>
	struct hash<Madam::UUID>
	{
		std::size_t operator()(const Madam::UUID& uuid) const
		{
			return std::hash<std::string>{}((std::string)(uuid));
		}
	};

}