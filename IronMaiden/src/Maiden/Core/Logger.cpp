#include "maidenpch.hpp"
#include "H_Logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/hourly_file_sink.h>
#include <chrono>
#include <iomanip>

namespace Madam {

	Ref<spdlog::logger> Logger::s_CoreLogger;
	Ref<spdlog::logger> Logger::s_ClientLogger;
	Ref<spdlog::logger> Logger::s_FileLogger;

	void Logger::Init() {
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("MAIDEN");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);

		auto now = std::chrono::system_clock::now();
		std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
		std::tm nowTm = *std::localtime(&nowTime);
		std::stringstream ss;
		ss << std::put_time(&nowTm, "%Y-%m-%d %H-%M");
		std::string file = "logs/" + ss.str() + ".log";

		s_FileLogger = spdlog::basic_logger_mt("File", file);
		s_FileLogger->set_pattern("%^[%T] %v%$");
		s_FileLogger->set_level(spdlog::level::trace);
	}
}