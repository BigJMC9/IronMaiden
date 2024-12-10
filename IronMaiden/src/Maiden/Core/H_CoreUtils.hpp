#pragma once
#include "Main/Core.hpp"
#include "maidenpch.hpp"

namespace Madam {

	struct null_t final 
	{
		null_t() = default;
		null_t(const null_t&) = default;
		null_t& operator=(const null_t&) = default;

		template <typename T>
		null_t(const T&) = delete;

		template <typename T>
		null_t& operator=(const T&) = delete;

		constexpr inline bool operator==(const null_t&) const { return true; }
		constexpr inline bool operator!=(const null_t&) const { return false; }
		bool operator==(const std::string& s) const 
		{ 
			return s == "";
		}
		bool operator!=(const std::string& s) const { 
			return s != "";
		}

		friend bool operator==(const std::string& left, const null_t& right) {
			return left == "";
		}

		friend bool operator!=(const std::string& left, const null_t& right) {
			return left != "";
		}
	};

	constexpr inline null_t null{};

	// from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

	class Entity;

	using id_t = unsigned int;
	using Map = std::unordered_map<id_t, Entity>;

	/*
	The UUID struct will be changed to GUID and will be used only for assets.
	A new struct for UUID will use a uint64_t for ingame objects for efficiency purposes. - Jacob

	Why not just use a uint64_t for the UUID and have that be used for both assets and game objects? - Luis

	1. uint128 hex string is standard in the industry thus would be familar to developers.
	2. A Hex string is easier to read and serialize in comparison to a uint64_t.
	3. A uint128 hex string has enough data to contain certain metadata segments into the string which makes version control
	more robust and easier.
	- Jacob
	*/
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

		bool operator==(const null_t& other) const
		{
			if (_UUID == "")
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		bool operator!=(const null_t& other) const
		{
			if (_UUID != "")
			{
				return true;
			}
			else
			{
				return false;
			}
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