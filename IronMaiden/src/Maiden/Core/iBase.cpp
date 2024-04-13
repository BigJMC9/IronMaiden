#include "maidenpch.hpp"
#include "Base.hpp"

namespace Madam {

	static std::random_device rd;
	static std::mt19937_64 gen(rd());
	static std::uniform_int_distribution<uint64_t> dis;

	UUID::UUID() {

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

	UUID::UUID(std::string uuid) : _UUID(uuid) {}
}