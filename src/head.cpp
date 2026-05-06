#include "head.hpp"
#include "player.hpp"
#include "physics.hpp"
#include "sprite.hpp"
#include "interactables.hpp"

using entt::literals::operator""_hs;

namespace clayborne {
    // TODO: make detonation continue over a period of time
    // TODO: trigger other things that can react to explosions
    static void detonate_head(
        const entt::entity entity,
        entt::registry &registry,
        const struct position &position,
        const struct collider &collider,
        // TODO: Replace with events
        audio_cache &sounds,
        // TODO: Replace with events
        MIX_Mixer *mixer
    ) {
        if (registry.any_of<struct sprite_animator>(entity)) {
            return;
        }
        else {
            auto &sprite_renderer{
                registry.get<struct sprite_renderer>(entity)
            };

            sprite_renderer.texture = "data/textures/player/head_explosion.png"_hs;
            sprite_renderer.x_offset = -8.0f;
            sprite_renderer.y_offset = -8.0f;
            sprite_renderer.z = 3;

            auto &sprite_animator{
                registry.emplace<struct sprite_animator>(entity)
            };

            sprite_animator.animation = "data/animations/player/head_explosion.json"_hs;
            sprite_animator.current_frame = 0;
            sprite_animator.is_looping = false;

            registry.remove<struct velocity>(entity);
            registry.remove<struct collider>(entity);
            registry.remove<struct activator>(entity);
        }

        const auto player_view{
            registry.view<struct player, struct position, struct velocity, struct collider>()
        };

        auto &player_player{ player_view.get<struct player>(player_view.front()) };
        auto &player_position{ player_view.get<struct position>(player_view.front()) };
        auto &player_velocity{ player_view.get<struct velocity>(player_view.front()) };
        auto &player_collider{ player_view.get<struct collider>(player_view.front()) };

        const float explosion_center_x{ position.x + collider.w * 0.5f };
        const float explosion_center_y{ position.y + collider.h * 0.5f };

        const clayborne::position explosion_position{
            .x = explosion_center_x - head::explosion_radius,
            .y = explosion_center_y - head::explosion_radius,
        };
        const clayborne::collider explosion_collider{
            .w = 2.0f * head::explosion_radius,
            .h = 2.0f * head::explosion_radius,
        };

        if (overlap(explosion_position, explosion_collider, player_position, player_collider)) {
            const float player_center_x{ player_position.x + player_collider.w * 0.5f };
            const float player_center_y{ player_position.y + player_collider.h * 0.5f };
            const float delta_x{ player_center_x - explosion_center_x };
            const float delta_y{ player_center_y - explosion_center_y };
            static const clayborne::velocity directions[8] = {
                { 1.0f,  0.0f },
                { inv_sqrt2,  inv_sqrt2 },
                { 0.0f,  1.0f },
                { -inv_sqrt2,  inv_sqrt2 },
                { -1.0f,  0.0f },
                { -inv_sqrt2, -inv_sqrt2 },
                { 0.0f, -1.0f },
                { inv_sqrt2, -inv_sqrt2 },
            };
            const float angle = std::atan2(delta_y, delta_x);
            const int octant = static_cast<int>(std::round(angle / (pi / 4.0f))) & 7;
            // TODO: use events or something
            player_velocity.x = directions[octant].x * player::head_launch_speed;
            player_velocity.y = directions[octant].y * player::head_launch_speed;
            player_player.state = player::state::launched;
            player_player.head_launch_timer = player::head_launch_duration;
            player_player.wall_speed_retention_timer = 0.0f;
        }

        // TODO: Replace with audio event sink
        (void)play_sound(registry, sounds, mixer, "data/explosion.wav"_hs, 1.0f, false);
    }

    static inline void update_head_in_start_state(
        const entt::entity entity,
        entt::registry &registry,
        const float delta_time,
        struct head &head,
        const struct position &position,
        struct velocity &velocity,
        const struct collider &collider,
        // TODO: Replace with events
        audio_cache &sounds,
        // TODO: Replace with events
        MIX_Mixer *mixer
    ) {
        // TODO: Remove dependency once input is no longer tied to player
        const auto player_view{ registry.view<struct player>() };
        auto &player_player{ player_view.get<struct player>(player_view.front()) };
        if (player_player.head_pressed) {
            detonate_head(entity, registry, position, collider, sounds, mixer);
        }

        // Check if grounded
        head.is_grounded = false;
        if (velocity.y >= 0) {
            auto below{ position };
            below.y += 1.0f;
            if (overlap_any(registry, entity, below, collider)) {
                head.is_grounded = true;
            }
        }

        // Apply gravity if airborne
        if (!head.is_grounded) {
            velocity.y = approach(
                velocity.y,
                player::fall_speed,
                player::gravity * delta_time
            );
        }

        // Deceleration
        velocity.x = approach(
            velocity.x,
            0.0f,
            head::throw_deceleration * delta_time
        );
    }

    static inline void update_head_in_thrown_state(
        const float delta_time,
        struct head &head,
        struct velocity &velocity
    ) {
        if (head.throw_timer > 0.0f) {
            head.throw_timer -= delta_time;
        }
        else {
            head.state = head::state::start;
            const auto new_velocity{
                normalize({ .x = velocity.x, .y = velocity.y }) * player::head_throw_end_speed
            };
            velocity.x = new_velocity.x;
            velocity.y = new_velocity.y;
        }
    }

    void head_collision_handler(
        entt::registry &registry,
        const collider::collision &collision
    ) noexcept {
        auto &head{ registry.get<clayborne::head>(collision.self) };
        auto &position{ registry.get<clayborne::position>(collision.self) };
        auto &velocity{ registry.get<clayborne::velocity>(collision.self) };
        auto &collider{ registry.get<clayborne::collider>(collision.self) };

        if (collision.normal.x != 0.0f) {
            if (head.state == head::state::thrown) {
                corner_correction<true, head::throw_corner_correction>(
                    registry,
                    collision.self,
                    position,
                    velocity,
                    collider
                );
            }

            velocity.x = 0.0f;
        }
        else if (collision.normal.y != 0.0f) {
            if (head.state == head::state::thrown) {
                corner_correction<false, head::throw_corner_correction>(
                    registry,
                    collision.self,
                    position,
                    velocity,
                    collider
                );
            }

            velocity.y = 0.0f;
        }
    }

    void update_heads(
        entt::registry &registry,
        const Uint64 dt_ns,
        animation_cache &animations,
        // TODO: Replace with events
        audio_cache &sounds,
        // TODO: Replace with events
        MIX_Mixer *mixer
    ) {
        const float delta_time{
            static_cast<float>(static_cast<double>(dt_ns) / SDL_NS_PER_SECOND)
        };

        auto view{
            registry.view<const struct position, struct velocity, const struct collider, struct head>()
        };

        for (auto entity : view) {
            auto &head{
                view.get<struct head>(entity)
            };
            const auto &position{
                view.get<const struct position>(entity)
            };
            auto &velocity{
                view.get<struct velocity>(entity)
            };
            const auto &collider{
                view.get<const struct collider>(entity)
            };

            switch (head.state) {
            case head::state::start:
                update_head_in_start_state(entity, registry, delta_time, head, position, velocity, collider, sounds, mixer);
                break;
            case head::state::thrown:
                update_head_in_thrown_state(delta_time, head, velocity);
                break;
            case head::state::detonated:
                detonate_head(entity, registry, position, collider, sounds, mixer);
                break;
            default:
                break;
            }
        }

        auto detonated_view{
            registry.view<struct head, struct sprite_animator>()
        };
        for (auto entity : detonated_view) {
            auto sprite_animator{
                detonated_view.get<struct sprite_animator>(entity)
            };
            if (sprite_animator.current_frame >= animations[sprite_animator.animation]->frames.size()) {
                registry.destroy(entity);
            }
        }
    }
}