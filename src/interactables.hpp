#ifndef CLAYBORNE_INTERACTABLES_HPP
#define CLAYBORNE_INTERACTABLES_HPP

#include <entt/entt.hpp>
#include "physics.hpp"
#include "camera.hpp"

namespace clayborne {
    struct sensor {
        float w{ 8.0f };
        float h{ 8.0f };
        bool is_sensing{ false };
    };

    struct activator {
        float w{ 8.0f };
        float h{ 8.0f };
    };

    [[nodiscard]] entt::entity create_sensor(entt::registry &registry, const float x, const float y) noexcept;
    void sense(entt::registry &registry) noexcept;

    struct door {
        entt::entity sensor{ entt::null };
        float w{ 8.0f };
        float h{ 8.0f };
        bool is_default_open{ false };
        bool is_open{ false };
    };

    [[nodiscard]] entt::entity create_door(entt::registry &registry, const float x, const float y, const bool is_default_open, const entt::entity toggle_sensor) noexcept;
    void toggle_doors(entt::registry &registry) noexcept;
};

#endif // CLAYBORNE_INTERACTABLES_HPP