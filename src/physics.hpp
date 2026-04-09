#ifndef CLAYBORNE_PHYSICS_HPP
#define CLAYBORNE_PHYSICS_HPP

#include <SDL3/SDL.h>
#include <cmath>
#include <entt/entt.hpp>
#include <optional>

namespace clayborne {
    struct vector {
        float x{ 0.0f };
        float y{ 0.0f };

        constexpr vector() noexcept = default;
        constexpr vector(float x_, float y_) noexcept : x(x_), y(y_) {}

        constexpr vector operator+(const vector& other) const noexcept {
            return {x + other.x, y + other.y};
        }

        constexpr vector operator-(const vector& other) const noexcept {
            return {x - other.x, y - other.y};
        }

        constexpr vector operator*(float scalar) const noexcept {
            return {x * scalar, y * scalar};
        }

        constexpr vector operator/(float scalar) const noexcept {
            return {x / scalar, y / scalar};
        }

        vector& operator+=(const vector& other) noexcept {
            x += other.x;
            y += other.y;
            return static_cast<vector&>(*this);
        }

        vector& operator-=(const vector& other) noexcept  {
            x -= other.x;
            y -= other.y;
            return static_cast<vector&>(*this);
        }

        vector& operator*=(float scalar) noexcept {
            x *= scalar;
            y *= scalar;
            return static_cast<vector&>(*this);
        }

        vector& operator/=(float scalar) noexcept {
            x /= scalar;
            y /= scalar;
            return static_cast<vector&>(*this);
        }

        [[nodiscard]] float length() const noexcept {
            return std::sqrt(x * x + y * y);
        }

        [[nodiscard]] float length_squared() const noexcept {
            return x * x + y * y;
        }

        [[nodiscard]] vector normalized() const noexcept {
            float len = length();
            return vector{ x / len, y / len };
        }

        [[nodiscard]] float dot(const vector &other) const noexcept {
            return x * other.x + y * other.y;
        }

        [[nodiscard]] float cross(const vector &other) const noexcept {
            return x * other.y - y * other.x;
        }
    };

    struct position {
        float x{ 0.0f };
        float y{ 0.0f };
    };

    struct velocity {
        float x{ 0.0f };
        float y{ 0.0f };
        float x_subpos{ 0.0f };
        float y_subpos{ 0.0f };
    };

    struct collider {
        float w{ 0.0f };
        float h{ 0.0f };

        struct collision {
            entt::entity self{ entt::null };
            entt::entity other{ entt::null };
            float normal_x{ 0.0f };
            float normal_y{ 0.0f };
        };

        // TODO: make an actual collision handler
        std::optional<std::function<void (entt::registry &, const collision &)>> collide{ std::nullopt };
    };

    // Important: if an entity moves, then its collision handler will be called at the point of
    //            intersection, only being moved back after all collisions are handled.
    void update_physics(entt::registry &registry, Uint64 dt_ns);

    [[nodiscard]] constexpr bool overlap(
        const position &position_1,
        const collider &collider_1,
        const position &position_2,
        const collider &collider_2
    ) noexcept {
        return 
            (position_1.x + collider_1.w > position_2.x) && 
            (position_1.y + collider_1.h > position_2.y) && 
            (position_2.x + collider_2.w > position_1.x) && 
            (position_2.y + collider_2.h > position_1.y );
    }

    template<typename... Includes, typename...Excludes>
    [[nodiscard]] bool overlap_any(
        const entt::registry &registry,
        const entt::entity self,
        const position &self_position,
        const collider &self_collider,
        entt::exclude_t<Excludes...> excludes = entt::exclude_t{}
    ) noexcept {
        auto view{ registry.view<const clayborne::position, const clayborne::collider, Includes...>(excludes) };
        for (auto [other, other_position, other_collider]: view.each()) {
            if (self == other) {
                continue;
            }

            if (overlap(self_position, self_collider, other_position, other_collider)) {
                return true;
            }
        }
        return false;
    }
}

#endif // CLAYBORNE_PHYSICS_HPP