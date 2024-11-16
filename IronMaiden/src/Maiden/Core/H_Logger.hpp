#pragma once

#include "maidenpch.hpp"
#include "Main/Core.hpp"
#include "../Utils/H_EngineUtils.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

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

#ifndef H_LOGGER_INCLUDE_GLM_GUARD
#ifdef INCLUDE_GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace fmt {
    template<>
    struct fmt::formatter<glm::vec2> {

        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::vec2& vec, FormatContext& ctx) -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "vec2({}, {})", vec.x, vec.y);
        }
    };

    template<>
    struct fmt::formatter<glm::vec3> {

        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::vec3& vec, FormatContext& ctx) -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "vec3({}, {}, {})", vec.x, vec.y, vec.z);
        }
    };

    template<>
    struct fmt::formatter<glm::vec4> {

        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::vec4& vec, FormatContext& ctx) -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "vec4({}, {}, {}. {})", vec.x, vec.y, vec.z, vec.w);
        }
    };

    template<>
    struct fmt::formatter<glm::quat> {

        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::quat& quat, FormatContext& ctx) -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "quat({}, {}, {}. {})", quat.x, quat.y, quat.z, quat.w);
        }
    };

    template<>
    struct fmt::formatter<glm::mat4> {

        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::mat4& mat4, FormatContext& ctx) -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "Matrix 4x4:\n({}, {}, {}, {})\n({}, {}, {}, {})\n({}, {}, {}, {})\n({}, {}, {}, {})",
                mat4[0][0], mat4[1][0], mat4[2][0], mat4[3][0],
                mat4[0][1], mat4[1][1], mat4[2][1], mat4[3][1],
                mat4[0][2], mat4[1][2], mat4[2][2], mat4[3][2],
                mat4[0][3], mat4[1][3], mat4[2][3], mat4[3][3]);
        }
    };

    template<>
    struct fmt::formatter<glm::mat3> {

        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::mat3& mat3, FormatContext& ctx) -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "Matrix 3x3:\n({}, {}, {})\n({}, {}, {})\n({}, {}, {})",
                mat3[0][0], mat3[1][0], mat3[2][0],
                mat3[0][1], mat3[1][1], mat3[2][1],
                mat3[0][2], mat3[1][2], mat3[2][2]);
        }
    };

    template<>
    struct fmt::formatter<glm::mat2> {

        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::mat2& mat2, FormatContext& ctx) -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "Matrix 2x2:\n({}, {})\n({}, {})",
                mat2[0][0], mat2[1][0],
                mat2[0][1], mat2[1][1]);
        }
    };
}
#define H_LOGGER_INCLUDE_GLM_GUARD
#endif
#endif // !H_LOGGER_INCLUDE_GLM_GUARD