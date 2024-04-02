#pragma once

#include "maidenpch.hpp"

namespace Madam {

	class Entity;

	using id_t = unsigned int;
	using Map = std::unordered_map<id_t, Entity>;

	struct UUID {

	public:
		UUID() {
			std::random_device rd;
			std::mt19937_64 gen(rd());
			std::uniform_int_distribution<uint64_t> dis;

			//For 128bits (16 bytes)
			uint64_t data[2];
			for (int i = 0; i < 2; i++)
			{
				data[i] = dis(gen);
			}

			std::stringstream ss;
			ss << std::hex << std::setfill('0');
			for (int i = 0; i < 2; i++)
			{
				ss << std::setw(16) << data[i];
				if (i < 1) ss << "-";
			}

			_UUID = ss.str();
		}

		UUID(std::string uuid) {
			_UUID = uuid;
		}
		//UUID(const UUID&) = delete;

		operator std::string() const { return _UUID; }
		friend std::ostream& operator<<(std::ostream& os, const UUID& uuid) {
			os << "UUID: " << uuid._UUID;
			return os;
		}

	private:
		std::string _UUID;
	};

	//Add Custom Smart Pointer that is shared. When Owner is destroyed and reference is > 0, throw error

	//Add Custom Smart Pointer that is owned but can be shared. When Owner is destroyed and reference is > 0, Tell garbage collector to delete it and set the pointer to nullptr.
	//Maybe Garbage Collector use a tree to keep track of which memory address each pointer points to.

	/*template<typename T>
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
	}*/
}