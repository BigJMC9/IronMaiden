#pragma once
#include "Main/Core.hpp"
#include "maidenpch.hpp"

namespace Madam {

	enum IrmResult
	{
		IRM_SUCCESS = 0,
		IRM_FAILURE = -1,
		IRM_ERROR_UNKNOWN = -2,
		IRM_INVALID_VERSION = -3,
		IRM_VERSION_MISMATCH = -4,
		IRM_STD_EXCEPTION = -5,
		IRM_ERROR_INVALID_PARAM = -6,
		IRM_ERROR_OUT_OF_MEMORY = -7,
		// File System Specific
		IRM_INVALID_FILE_TYPE = -101,
		IRM_FILE_NOT_FOUND = -102,
		IRM_READ_FAILURE = -103,
		IRM_WRITE_FAILURE = -104,
		IRM_FILE_DIALOG_CANCELLED = -105,
		// YAML Specific
		IRM_YAML_EXCEPTION = -200,
		IRM_YAML_READ_FAILURE = -201,
		IRM_YAML_WRITE_FAILURE = -202,
		IRM_YAML_INVALID_NODE = -203,
		IRM_YAML_BAD_CONVERSION = -204,
		// Serialization Specific
		IRM_INVALID_SCENE_NODE = -301,
		IRM_INVALID_VERSION_NODE = -302,
		IRM_INVALID_ENTITIES_NODE = -302,
		// Scene Specific
		IRM_INVALID_SCENE_NAME = -401,
		// Entity Specific
		IRM_INVALID_ENTITY = -501,

	};

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
	*/
	struct UUID {

	public:
		UUID();

		UUID(std::string uuid);
		UUID(const UUID&) = default;

		operator std::string() const { return m_uuid; }

		friend std::ostream& operator<<(std::ostream& os, const UUID& uuid) {
			os << "UUID: " << uuid.m_uuid;
			return os;
		}

		friend bool operator==(const UUID& left, const UUID& right) {
			return left.m_uuid == right.m_uuid;
		}

		friend bool operator!=(const UUID& left, const UUID& right) {
			return left.m_uuid != right.m_uuid;
		}
		
		friend bool operator==(const UUID& left, const std::string& right) {
			return left.m_uuid == right;
		}

		friend bool operator!=(const UUID& left, const std::string& right) {
			return left.m_uuid != right;
		}

		bool operator==(const null_t& other) const
		{
			if (m_uuid == "")
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
			if (m_uuid != "")
			{
				return true;
			}
			else
			{
				return false;
			}
		}

	private:
		std::string m_uuid;
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