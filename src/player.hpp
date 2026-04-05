#ifndef CLAYBORNE_PLAYER_HPP
#define CLAYBORNE_PLAYER_HPP

#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include "engine/input.hpp"

namespace clayborne {
    struct player {
        static constexpr float run_speed{ 9.0f };
        static constexpr float run_acceleration{ 100.0f };
        static constexpr float run_deceleration{ 40.0f };
        static constexpr float air_multiplier{ 0.65f };

        bool is_jumping{ false };
        bool is_airborne{ false };

        entt::entity entity{ entt::null };
    };

    player init_player(entt::registry &registry);
    void deinit_player(player &player, entt::registry &registry);
    void update_player(player &player, entt::registry &registry, const input::gameplay_context &inputs);
}

#endif // CLAYBORNE_PLAYER_HPP