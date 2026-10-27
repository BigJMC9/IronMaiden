#include "maidenpch.hpp"
#include "H_Logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/hourly_file_sink.h>
#include <spdlog/sinks/base_sink.h>
#include <chrono>
#include <atomic>
#include <deque>
#include <iomanip>
#include <mutex>

namespace Madam {
	class ConsoleSink final : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		ConsoleSink() { set_pattern("[%T] %n: %v"); }

		std::vector<ConsoleMessage> GetMessages()
		{
			std::lock_guard<std::mutex> lock(messagesMutex);
			return { messages.begin(), messages.end() };
		}
		std::uint64_t GetRevision() const { return revision.load(std::memory_order_relaxed); }

		void Clear()
		{
			std::lock_guard<std::mutex> lock(messagesMutex);
			messages.clear();
			revision.fetch_add(1, std::memory_order_relaxed);
		}

	protected:
		void sink_it_(const spdlog::details::log_msg& message) override
		{
			spdlog::memory_buf_t formatted;
			formatter_->format(message, formatted);
			std::string text(formatted.data(), formatted.size());
			while (!text.empty() && (text.back() == '\n' || text.back() == '\r'))
				text.pop_back();
			std::lock_guard<std::mutex> lock(messagesMutex);
			if (messages.size() == 1000) messages.pop_front();
			messages.push_back({ std::move(text), message.level });
			revision.fetch_add(1, std::memory_order_relaxed);
		}

		void flush_() override {}

	private:
		std::mutex messagesMutex;
		std::deque<ConsoleMessage> messages;
		std::atomic<std::uint64_t> revision{ 0 };
	};

	Ref<spdlog::logger> Logger::s_CoreLogger;
	Ref<spdlog::logger> Logger::s_ClientLogger;
	Ref<spdlog::logger> Logger::s_FileLogger;
	Ref<ConsoleSink> Logger::s_ConsoleSink;

	void Logger::Init() {
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("MAIDEN");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ConsoleSink = CreateRef<ConsoleSink>();
		s_CoreLogger->sinks().push_back(s_ConsoleSink);
		s_ClientLogger->sinks().push_back(s_ConsoleSink);

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

	std::vector<ConsoleMessage> Logger::GetConsoleMessages()
	{
		return s_ConsoleSink ? s_ConsoleSink->GetMessages() : std::vector<ConsoleMessage>{};
	}

	std::uint64_t Logger::GetConsoleRevision()
	{
		return s_ConsoleSink ? s_ConsoleSink->GetRevision() : 0;
	}

	void Logger::ClearConsoleMessages()
	{
		if (s_ConsoleSink) s_ConsoleSink->Clear();
	}
}
