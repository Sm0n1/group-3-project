#ifndef CLAYBORNE_INTERACTABLES_HPP
#define CLAYBORNE_INTERACTABLES_HPP

#include <entt/entt.hpp>
#include "sprite.hpp"

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

    struct door {
        entt::entity sensor{ entt::null }; // If null, doors listen to all sensors
        float w{ 8.0f };
        float h{ 8.0f };
        bool is_default_open{ false };
        bool is_open{ false };
    };

    [[nodiscard]] entt::entity create_sensor(
        entt::registry &registry,
        texture_cache &textures,
        SDL_Renderer *renderer,
        const float x,
        const float y
    ) noexcept;

    [[nodiscard]] entt::entity create_door(
        entt::registry &registry,
        texture_cache &textures,
        SDL_Renderer *renderer,
        const float x,
        const float y,
        const bool is_default_open,
        const entt::entity toggle_sensor
    ) noexcept;
    
    void sense(entt::registry &registry) noexcept;
    void toggle_doors(entt::registry &registry) noexcept;
};

#endif // CLAYBORNE_INTERACTABLES_HPP