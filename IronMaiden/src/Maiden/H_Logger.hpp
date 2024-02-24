#pragma once

#include "maidenpch.hpp"
#include "Core.hpp"

#include <spdlog/spdlog.h>

namespace Madam
{
	class MADAM_API Logger 
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:

		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
};

//Core log macros
#define MADAM_CORE_TRACE(...) ::Madam::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define MADAM_CORE_INFO(...)  ::Madam::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define MADAM_CORE_WARN(...)  ::Madam::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define MADAM_CORE_ERROR(...) ::Madam::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define MADAM_CORE_FATAL(...) ::Madam::Logger::GetCoreLogger()->fatal(__VA_ARGS__)

//Client log macros
#define MADAM_TRACE(...)      ::Madam::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define MADAM_INFO(...)       ::Madam::Logger::GetClientLogger()->info(__VA_ARGS__)
#define MADAM_WARN(...)       ::Madam::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define MADAM_ERROR(...)      ::Madam::Logger::GetClientLogger()->error(__VA_ARGS__)
#define MADAM_FATAL(...)      ::Madam::Logger::GetClientLogger()->fatal(__VA_ARGS__)

