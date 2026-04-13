#ifndef CLAYBORNE_PLAYER_HPP
#define CLAYBORNE_PLAYER_HPP

#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <entt/entt.hpp>
#include "engine/input/manager.hpp"
#include "resources.hpp"

// HEAD MECHANIC
//
// Edge case 1: 
//     If the player is partially under ceiling, then reattachment should not be possible.
//     If possible, the player and/or the head should be moved to the side.
//     If no movement is possible, then the head should be destroyed.
// 
// Edge case 2:
//     If the player throws the head against an surface they are close to,
//     then the player should be pushed away to accomodate the head collider.
//     If this is not possible, then the head should not be thrown.
//
// Edge case 3:
//     If there is clay in a one-block tall corridor, then the head should not be able to
//     regrow in order to not trap the player within the ceiling.
//
// Edge case 4:
//     ...
//
//
//
// Throwing the head should throw it at the height of the head.
// Throwing the head should stall the player in the air for a moment.

namespace clayborne {
    struct player {
        // ------------------------ //
        // Temporary Input Handling //
        // ------------------------ //
        bool jump_just_pressed{ false }; // Celeste clears its jump buffer when releasing the button
        bool jump_pressed{ false };
        bool head_just_pressed{ false };
        bool left{ false };
        bool right{ false };
        bool up{ false };
        bool down{ false };
        // ------------------------ //

        enum class facing {
            left,
            right,
        };

        static constexpr float run_speed{ 90.0f };
        static constexpr float run_acceleration{ 1000.0f };
        static constexpr float run_deceleration{ 400.0f };
        static constexpr float air_multiplier{ 0.65f };

        static constexpr float jump_vertical_speed{ -150.0f };
        static constexpr float jump_horizontal_speed{ 40.0f };
        static constexpr float jump_buffer_time{ 0.08f };
        static constexpr float jump_grace_time{ 0.1f };
        static constexpr float jump_boost_time{ 0.2f };

        static constexpr float fall_speed{ 160.0f };
        static constexpr float gravity{ 900.0f };
        static constexpr float half_gravity_threshold{ 40.0f };

        static constexpr float wall_speed_retention_time{ 0.6f };
        static constexpr float ceiling_jump_boost_grace{ 0.5f };
        static constexpr int ceiling_corner_correction{ 4 };

        static constexpr float hitbox_width{ 8.0f };
        static constexpr float hitbox_height{ 11.0f };
        static constexpr float headless_hitbox_height{ 8.0f };

        bool is_grounded{ true };
        facing facing{ facing::right };

        // Allows buffering a jump to trigger when landing.
        float jump_buffer_timer{ 0.0f };

        // Allows jumping a few moments after beginning to fall.
        float jump_grace_timer{ 0.0f };

        // Allows holding the jump button to gain increased height.
        float jump_boost_timer{ 0.0f };
        float jump_boost_speed{ 0.0f };

        // Moving into a wall retains the momentum for a short duration.
        float wall_speed_retention_timer{ 0.0f };
        float wall_speed_retention{ 0.0f };

        bool is_head_attached{ true };
        entt::entity head{ entt::null };
    };

    struct head {
        static constexpr float hitbox_width{ 8.0f };
        static constexpr float hitbox_height{ 8.0f };
        static constexpr float throw_speed{ 200.0f };
        static constexpr float throw_deceleration{ 500.0f };
        static constexpr float gravity{ player::gravity };
        static constexpr float fall_speed{ player::fall_speed };

        static constexpr float explosion_radius{ 14.0f };
        static constexpr float explosion_speed{ 400.0f };

        bool is_grounded{ true };
        bool is_thrown{ true };
        bool is_detonated{ false };
    };
    
    entt::entity init_player(entt::registry &registry, clayborne::resources &resources, float x, float y) noexcept;
    void update_player(entt::entity player_entity, entt::registry &registry, const input::manager &inputs, Uint64 dt_ns) noexcept;
}

#endif // CLAYBORNE_PLAYER_HPP