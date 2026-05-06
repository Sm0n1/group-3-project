#ifndef CLAYBORNE_HEAD_HPP
#define CLAYBORNE_HEAD_HPP

#include <entt/entt.hpp>
#include <SDL3/SDL.h>
#include "audio.hpp"
#include "physics.hpp"
#include "sprite.hpp"

namespace clayborne {
    struct head {
        static constexpr float hitbox_width{ 8.0f };
        static constexpr float hitbox_height{ 8.0f };
         
        static constexpr float throw_deceleration{ 500.0f };
        static constexpr int   throw_corner_correction{ 3 };

        static constexpr float explosion_duration{ 0.3f };
        static constexpr float explosion_radius{ 14.0f };

        enum class state {
            start,
            thrown,
            detonated,
        };

        bool is_grounded{ false };
        state state{ state::start };

        float throw_timer{ 0.0f };
        float explosion_timer{ 0.0f };
    };

    void head_collision_handler(
        entt::registry &registry,
        const collider::collision &collision
    ) noexcept;

    void update_heads(
        entt::registry &registry,
        const Uint64 dt_ns,
        animation_cache &animations,
        // TODO: Replace with events
        audio_cache &sounds,
        // TODO: Replace with events
        MIX_Mixer *mixer
    );
}

#endif // CLAYBORNE_HEAD_HPP