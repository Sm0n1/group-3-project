#ifndef CLAYBORNE_PLAYER_HPP
#define CLAYBORNE_PLAYER_HPP

#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <entt/entt.hpp>
#include "engine/input/manager.hpp"
#include "resources.hpp"

namespace clayborne {
    struct player {
        // ------------------------ //
        // Temporary Input Handling //
        // ------------------------ //
        bool jump_just_pressed{ false }; // Celeste clears its jump buffer when releasing the button
        bool jump_pressed{ false };
        bool head_just_pressed{ false };
        bool head_pressed{ false };
        bool left{ false };
        bool right{ false };
        bool up{ false };
        bool down{ false };
        // ------------------------ //

        enum class facing {
            left,
            right,
        };

        enum class state {
            start, // Running, jumping, and falling
            throwing, // Throwing head
            launched, // Launched by exploding head
            dead,
        };

        // ---------------------------- //
        // Running & Movement Constants //
        // ---------------------------- //

        static constexpr float run_speed{ 90.0f }; // Maximum run speed in pixels per second
        static constexpr float run_acceleration{ 1000.0f }; // Acceleration in pixels per seconds squared applied when below max speed
        static constexpr float run_deceleration{ 400.0f }; // Acceleration in pixels per seconds squared applied when above max speed
        static constexpr float air_multiplier{ 0.65f }; // Acceleration and deceleration multiplier applied whilst airborne
        static constexpr float wall_speed_retention_time{ 0.06f }; // Duration moving into a wall retains speed

        // ----------------- //
        // Jumping Constants //
        // ----------------- //

        static constexpr float jump_vertical_speed{ -150.0f }; // Vertical speed applied to the player when they jump
        static constexpr float jump_horizontal_speed{ 40.0f }; // Horizontal speed boost applied to the player when they jump whilst moving
        static constexpr float jump_buffer_duration{ 0.08f }; // Duration in seconds a held jump button input is buffered whilst falling
        static constexpr float jump_grace_duration{ 0.1f }; // Duration in seconds the player can jump after beginning to fall
        static constexpr float jump_boost_duration{ 0.2f }; // Duration in seconds the player can hold jump to increase height
        static constexpr float ceiling_jump_boost_grace{ 0.5f }; // Duration in seconds the player gains held jump boost whilst colliding with the ceiling
        static constexpr int   ceiling_corner_correction{ 4 }; // Number of pixels the player is shifted to jump past ceiling corners

        // ----------------- //
        // Falling Constants //
        // ----------------- //

        static constexpr float fall_speed{ 160.0f }; //
        static constexpr float gravity{ 900.0f }; //
        static constexpr float half_gravity_threshold{ 40.0f }; // The vertical speed 

        // ---------------- //
        // Hitbox Ccnstants //
        // ---------------- //

        static constexpr float hitbox_width{ 8.0f };
        static constexpr float hitbox_height{ 11.0f };
        static constexpr float headless_hitbox_height{ 8.0f };

        // -------------- //
        // Head Constants //
        // -------------- //

        static constexpr float head_throw_duration{ 0.20f }; // Time in seconds in the throwing state
        static constexpr float head_throw_speed{ 240.0f }; // Speed applied to the thrown head
        static constexpr float head_throw_end_speed{ 160.0f }; // Speed applied to the head when it exits the thrown state
        static constexpr float head_launch_duration{ 0.15f }; // Time in seconds in the launched state
        static constexpr float head_launch_speed{ 280.0f }; // Speed applied to the player when they are launched by head explosion
        static constexpr float head_launch_end_speed{ 160.0f }; // Speed applied to the player when they exit the launched state
        static constexpr float head_buffer_duration{ 0.08f }; // Duration in seconds a held head button input is buffered
        static constexpr int   head_launch_corner_correction{ 4 }; // Number of pixels the player is shifted to move past corners whilst launched

        // -------------- //
        // States & Flags //
        // -------------- //

        bool is_grounded{ false }; //
        bool is_on_clay{ false }; // False if not grounded
        bool is_head_attached{ true }; //
        facing facing{ facing::right }; //
        state state{ state::start }; //

        // ------------- //
        // Movement Data //
        // ------------- //

        float jump_buffer_timer{ 0.0f }; //
        float jump_grace_timer{ 0.0f }; //
        float jump_boost_timer{ 0.0f }; //
        float jump_boost_speed{ 0.0f }; //
        float wall_speed_retention_timer{ 0.0f }; //
        float wall_speed_retention{ 0.0f }; //

        // --------- //
        // Head Data //
        // --------- //

        entt::entity head{ entt::null };
        entt::entity buried_head{ entt::null };

        // Allows buffering a head throw/detonation to trigger when possible.
        float head_buffer_timer{ 0.0f };
        float head_throw_timer{ 0.0f };
        float head_launch_timer{ 0.0f };

        // ---------- //
        // Death Data //
        // ---------- //

        float respawn_x{ 0.0f };
        float respawn_y{ 0.0f };
    };

    struct head {
        static constexpr float hitbox_width{ 8.0f };
        static constexpr float hitbox_height{ 8.0f };
         
        static constexpr float throw_deceleration{ 500.0f };
        static constexpr int   throw_corner_correction{ 3 };

        static constexpr float explosion_duration{ 0.3f };
        static constexpr float explosion_radius{ 14.0f };

        enum class state {
            start,
            buried,
            thrown,
            detonated,
        };

        bool is_grounded{ false };
        state state{ state::start };

        float throw_timer{ 0.0f };
        float explosion_timer{ 0.0f };
    };
    
    entt::entity init_player(entt::registry &registry, clayborne::resources &resources, float x, float y) noexcept;
    void update_player(entt::entity player_entity, entt::registry &registry, const input::manager &inputs, Uint64 dt_ns) noexcept;
}

#endif // CLAYBORNE_PLAYER_HPP