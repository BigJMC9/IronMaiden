#pragma once

#include <spdlog/fmt/ostr.h>
#include <glm/glm.hpp>

namespace fmt {
    template<>
    struct fmt::formatter<glm::vec2> {

        constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::vec2& vec, FormatContext& ctx) const -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "vec2({}, {})", vec.x, vec.y);
        }
    };

    template<>
    struct fmt::formatter<glm::vec3> {

        constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::vec3& vec, FormatContext& ctx) const -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "vec3({}, {}, {})", vec.x, vec.y, vec.z);
        }
    };

    template<>
    struct fmt::formatter<glm::vec4> {

        constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::vec4& vec, FormatContext& ctx) const -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "vec4({}, {}, {}. {})", vec.x, vec.y, vec.z, vec.w);
        }
    };

    template<>
    struct fmt::formatter<glm::quat> {

        constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::quat& quat, FormatContext& ctx) const -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "quat({}, {}, {}. {})", quat.x, quat.y, quat.z, quat.w);
        }
    };

    template<>
    struct fmt::formatter<glm::mat4> {

        constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::mat4& mat4, FormatContext& ctx) const -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "Matrix 4x4:\n({}, {}, {}, {})\n({}, {}, {}, {})\n({}, {}, {}, {})\n({}, {}, {}, {})",
                mat4[0][0], mat4[1][0], mat4[2][0], mat4[3][0],
                mat4[0][1], mat4[1][1], mat4[2][1], mat4[3][1],
                mat4[0][2], mat4[1][2], mat4[2][2], mat4[3][2],
                mat4[0][3], mat4[1][3], mat4[2][3], mat4[3][3]);
        }
    };

    template<>
    struct fmt::formatter<glm::mat3> {

        constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::mat3& mat3, FormatContext& ctx) const -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "Matrix 3x3:\n({}, {}, {})\n({}, {}, {})\n({}, {}, {})",
                mat3[0][0], mat3[1][0], mat3[2][0],
                mat3[0][1], mat3[1][1], mat3[2][1],
                mat3[0][2], mat3[1][2], mat3[2][2]);
        }
    };

    template<>
    struct fmt::formatter<glm::mat2> {

        constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const glm::mat2& mat2, FormatContext& ctx) const -> decltype(ctx.out()) {
            return fmt::format_to(ctx.out(), "Matrix 2x2:\n({}, {})\n({}, {})",
                mat2[0][0], mat2[1][0],
                mat2[0][1], mat2[1][1]);
        }
    };
}