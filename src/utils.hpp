#ifndef CLAYBORNE_UTILS_HPP
#define CLAYBORNE_UTILS_HPP

#include <cassert>
#include <cmath>
#include <iostream>
#include <source_location>

namespace clayborne {
    // Calls a function and returns std::nullopt if it throws an exception.
    // Functions that return void return std::monostate instead.
    template<typename F, typename... A>
    std::optional<std::conditional_t<
        std::is_void_v<std::invoke_result_t<F, A...>>,
        std::monostate,
        std::invoke_result_t<F, A...>
    >> call(F &&f, A &&...a) {
        try {
            if constexpr (std::is_void_v<std::invoke_result_t<F, A...>>) {
                std::forward<F>(f)(std::forward<A>(a)...);
                return std::monostate{};
            }
            else {
                return std::forward<F>(f)(std::forward<A>(a)...);
            }
        }
        catch (...) {
            return std::nullopt;
        }
    }

    constexpr float inv_sqrt2{ 0.70710678f };
    constexpr float pi{ 3.14159265358979323846f };

    inline void log(const char* msg, const std::source_location loc = std::source_location::current()) {
        std::cout << loc.line() << " " << msg << std::endl;
    }

    constexpr float sgn(const float x) {
        return static_cast<float>(x > 0.0f) - static_cast<float>(x < 0.0f);
    }

    [[nodiscard]] constexpr float approach(const float from, const float to, const float amount) noexcept {
        const float delta{ to - from };
        const float sign{ static_cast<float>((delta > 0.0f) - (delta < 0.0f)) };
        const float change{ std::clamp(amount * sign, -std::abs(delta), std::abs(delta)) };
        return from + change;
    }

    struct vec2 {
        float x{ 0.0f };
        float y{ 0.0f };

        [[nodiscard]] constexpr vec2 operator-() const noexcept {
            return { -x, -y };
        }

        [[nodiscard]] constexpr vec2 operator+(const vec2 &other) const noexcept {
            return { x + other.x, y + other.y };
        }

        [[nodiscard]] constexpr vec2 operator-(const vec2 &other) const noexcept {
            return { x - other.x, y - other.y };
        }

        [[nodiscard]] constexpr vec2 operator*(const float scalar) const noexcept {
            return { x * scalar, y * scalar };
        }

        [[nodiscard]] constexpr vec2 operator/(const float scalar) const noexcept {
            return { x / scalar, y / scalar };
        }

        constexpr vec2 &operator+=(const vec2 &other) noexcept {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr vec2 &operator-=(const vec2 &other) noexcept  {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr vec2 &operator*=(const float scalar) noexcept {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        constexpr vec2 &operator/=(const float scalar) noexcept {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        friend constexpr vec2 operator*(const float scalar, const vec2 &self) noexcept {
            return {self.x * scalar, self.y * scalar};
        }

        friend std::ostream &operator<<(std::ostream &os, const vec2 &v) {
            return os << '(' << v.x << ", " << v.y << ')';
        }
    };

    [[nodiscard]] inline float length(const vec2 &a) noexcept {
        return std::sqrt(a.x * a.x + a.y * a.y);
    }

    [[nodiscard]] constexpr float length_squared(const vec2 &a) noexcept {
        return a.x * a.x + a.y * a.y;
    }

    [[nodiscard]] inline vec2 normalize(const vec2 &a) noexcept {
        const float len{ length(a) };

        if (len == 0.0f) {
            return {};
        }

        return { a.x / len, a.y / len };
    }

    [[nodiscard]] inline vec2 round(const vec2 &a) noexcept {
        return { std::round(a.x), std::round(a.y) };
    }

    [[nodiscard]] constexpr vec2 sgn(const vec2 &a) noexcept {
        return {
            static_cast<float>(a.x > 0.0f) - static_cast<float>(a.x < 0.0f), 
            static_cast<float>(a.y > 0.0f) - static_cast<float>(a.y < 0.0f),
        };
    }

    [[nodiscard]] inline vec2 abs(const vec2 &a) noexcept {
        return { std::fabs(a.x), std::fabs(a.y) };
    }

    [[nodiscard]] constexpr vec2 min(const vec2 &a, const vec2 &b) noexcept {
        return { std::min(a.x, b.x), std::min(a.y, b.y) };
    }

    [[nodiscard]] constexpr vec2 max(const vec2 &a, const vec2 &b) noexcept {
        return { std::max(a.x, b.x), std::max(a.y, b.y) };
    }

    [[nodiscard]] constexpr vec2 hadamard(const vec2 &a, const vec2 &b) noexcept {
        return { a.x * b.x, a.y * b.y };
    }

    [[nodiscard]] constexpr float dot(const vec2 &a, const vec2 &b) noexcept {
        return a.x * b.x + a.y * b.y;
    }

    [[nodiscard]] constexpr float cross(const vec2 &a, const vec2 &b) noexcept {
        return a.x * b.y - a.y * b.x;
    }

    [[nodiscard]] constexpr float distance_squared(const vec2 &a, const vec2 &b) noexcept {
        return length_squared(b - a);
    }

    [[nodiscard]] inline float distance(const vec2 &a, const vec2 &b) noexcept {
        return length(b - a);
    }

    [[nodiscard]] inline vec2 direction(const vec2 &a, const vec2 &b) noexcept {
        return normalize(b - a);
    }

    [[nodiscard]] constexpr vec2 project(const vec2 &a, const vec2 &b) noexcept {
        const float denominator{ length_squared(b) };

        if (denominator == 0.0f) {
            return {};
        }

        return b * (dot(a, b) / denominator);
    }

    [[nodiscard]] constexpr vec2 reflect(const vec2 &a, const vec2 &unit_normal) noexcept {
        return a - 2.0f * dot(a, unit_normal) * unit_normal;
    }

    [[nodiscard]] constexpr vec2 perpendicular(const vec2 &a) noexcept {
        return { -a.y, a.x };
    }

    [[nodiscard]] inline float angle(const vec2 &a) noexcept {
        return std::atan2(a.y, a.x);
    }

    [[nodiscard]] inline float angle(const vec2 &a, const vec2 &b) noexcept {
        return std::atan2(cross(a, b), dot(a, b));
    }

    [[nodiscard]] inline vec2 rotate(const vec2 &a, const float radians) noexcept {
        const float c{ std::cos(radians) };
        const float s{ std::sin(radians) };
        return { a.x * c - a.y * s, a.x * s + a.y * c };
    }

    [[nodiscard]] constexpr vec2 lerp(const vec2 &a, const vec2 &b, const float time) noexcept {
        return a + (b - a) * time;
    }
}

#endif // CLAYBORNE_UTILS_HPP