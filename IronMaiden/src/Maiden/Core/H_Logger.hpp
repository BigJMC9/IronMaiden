#include "maidenpch.hpp"
#include "Main/Core.hpp"
#include "H_CoreUtils.hpp"

#ifdef INCLUDE_GLM_UTILS
#include "../Utils/H_Logger_Utils.h"
#undef INCLUDE_GLM_UTILS
#endif

#ifndef H_LOGGER_GUARD
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <filesystem>

namespace fmt {

    template<>
    struct fmt::formatter<std::filesystem::path> {

        constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const std::filesystem::path& path, FormatContext& ctx) const -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "File Directory: {}", path.u8string());
        }
    };

    template<>
    struct fmt::formatter<Madam::UUID> {

        constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const Madam::UUID& uuid, FormatContext& ctx) const -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "UUID: {}", uuid);
        }
    };

    template <typename T>
    struct fmt::formatter<T, std::enable_if_t<std::is_enum_v<T>, char>> {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const T& value, FormatContext& ctx) const -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "{}", static_cast<int>(value));
        }
    };

}

namespace Madam
{
	class MADAM_API Logger 
	{
	public:
		static void Init();

		inline static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		inline static Ref<spdlog::logger>& GetFileLogger() { return s_FileLogger; }
	private:

		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
		static Ref<spdlog::logger> s_FileLogger;
	};
};

//Quiet log macros
#define MADAM_QUIET_TRACE(...)         ::Madam::Logger::GetFileLogger()->trace("LOG: Trace: {}", fmt::format(__VA_ARGS__))
#define MADAM_QUIET_INFO(...)          ::Madam::Logger::GetFileLogger()->info("LOG: Info: {}", fmt::format(__VA_ARGS__))
#define MADAM_QUIET_NOT_IMPL(...)      ::Madam::Logger::GetFileLogger()->info("LOG: Not Implemented: {}", fmt::format(__VA_ARGS__))
#define MADAM_QUIET_WARN(...)          ::Madam::Logger::GetFileLogger()->warn("LOG: Warn: {}", fmt::format(__VA_ARGS__))
#define MADAM_QUIET_ERROR(...)         ::Madam::Logger::GetFileLogger()->error("LOG: Error: {}", fmt::format(__VA_ARGS__))
#define MADAM_QUIET_FATAL(...)         ::Madam::Logger::GetFileLogger()->critical("LOG: Fatal Error: {}", fmt::format(__VA_ARGS__))
								       
//Core log macros				       
#define MADAM_CORE_TRACE(...)          ::Madam::Logger::GetCoreLogger()->trace(__VA_ARGS__); \
                                       ::Madam::Logger::GetFileLogger()->trace("MAIDEN: Trace: {}", fmt::format(__VA_ARGS__))
#define MADAM_CORE_INFO(...)           ::Madam::Logger::GetCoreLogger()->info(__VA_ARGS__); \
                                       ::Madam::Logger::GetFileLogger()->info("MAIDEN: Info: {}", fmt::format(__VA_ARGS__))
#define MADAM_CORE_NOT_IMPL(...)       ::Madam::Logger::GetCoreLogger()->info(__VA_ARGS__); \
                                       ::Madam::Logger::GetFileLogger()->info("MAIDEN: Not Implemented: {}", fmt::format(__VA_ARGS__))
#define MADAM_CORE_WARN(...)           ::Madam::Logger::GetCoreLogger()->warn(__VA_ARGS__); \
                                       ::Madam::Logger::GetFileLogger()->warn("MAIDEN: Warn: {}", fmt::format(__VA_ARGS__))
#define MADAM_CORE_ERROR(...)          ::Madam::Logger::GetCoreLogger()->error(__VA_ARGS__); \
                                       ::Madam::Logger::GetFileLogger()->error("MAIDEN: Error: {}", fmt::format(__VA_ARGS__))
#define MADAM_CORE_FATAL(...)          ::Madam::Logger::GetCoreLogger()->critical(__VA_ARGS__); \
                                       ::Madam::Logger::GetFileLogger()->critical("MAIDEN: Fatal Error: {}",fmt::format( __VA_ARGS__))
#define MADAM_CORE_ASSERT(x, ...)      { if(!(x)) { MADAM_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define MADAM_CORE_POINTER(...)        ::fmt::ptr(__VA_ARGS__)
#define MADAM_CORE_VALID_VERBOSE(...)  ::Madam::Logger::GetCoreLogger()->trace("\033[38;5;22m{}\033[0m", fmt::format(__VA_ARGS__)); \
                                       ::Madam::Logger::GetFileLogger()->trace("MAIDEN: {}", fmt::format(__VA_ARGS__))
#define MADAM_CORE_VALID_INFO(...)     ::Madam::Logger::GetCoreLogger()->info("\033[38;5;22m{}\033[0m", fmt::format(__VA_ARGS__)); \
                                       ::Madam::Logger::GetFileLogger()->info("MAIDEN: {}", fmt::format(__VA_ARGS__))
#define MADAM_CORE_VALID_WARN(...)     ::Madam::Logger::GetCoreLogger()->warn("\033[38;2;255;165;0m{}\033[0m", fmt::format(__VA_ARGS__)); \
                                       ::Madam::Logger::GetFileLogger()->warn("MAIDEN: {}", fmt::format(__VA_ARGS__))
#define MADAM_CORE_VALID_ERROR(...)    ::Madam::Logger::GetCoreLogger()->error("\033[38;2;139;0;0m{}\033[0m", fmt::format(__VA_ARGS__)); \
                                       ::Madam::Logger::GetFileLogger()->error("MAIDEN: {}", fmt::format(__VA_ARGS__))

//Client log macros
#define MADAM_TRACE(...)      ::Madam::Logger::GetClientLogger()->trace(__VA_ARGS__); \
                              ::Madam::Logger::GetFileLogger()->trace("APP: Trace: {}", fmt::format(__VA_ARGS__))
#define MADAM_INFO(...)       ::Madam::Logger::GetClientLogger()->info(__VA_ARGS__); \
                              ::Madam::Logger::GetFileLogger()->info("APP: Info: {}", fmt::format(__VA_ARGS__))
#define MADAM_WARN(...)       ::Madam::Logger::GetClientLogger()->warn(__VA_ARGS__); \
                              ::Madam::Logger::GetFileLogger()->warn("APP: Warn: {}", fmt::format(__VA_ARGS__))
#define MADAM_ERROR(...)      ::Madam::Logger::GetClientLogger()->error(__VA_ARGS__); \
                              ::Madam::Logger::GetFileLogger()->error("APP: Error: {}", fmt::format(__VA_ARGS__))
#define MADAM_FATAL(...)      ::Madam::Logger::GetClientLogger()->critical(__VA_ARGS__); \
                              ::Madam::Logger::GetFileLogger()->fatal("APP: Fatal Error: {}", fmt::format(__VA_ARGS__))

#define H_LOGGER_GUARD
#endif