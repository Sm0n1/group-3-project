#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <entt/entt.hpp>
#include <optional>
#include "player.hpp"
#include "physics.hpp"
#include "camera.hpp"
#include "clay.hpp"

[[nodiscard]] static inline constexpr float approach(const float from, const float to, const float amount) noexcept {
    const float delta{ to - from };
    const float sign{ static_cast<float>((delta > 0.0f) - (delta < 0.0f)) };
    const float change{ std::clamp(amount * sign, -std::abs(delta), std::abs(delta)) };
    return from + change;
}

namespace clayborne {
    static void player_collision_handler(entt::registry &registry, const collider::collision &collision) {
        auto &player{ registry.get<clayborne::player>(collision.self) };
        auto &position{ registry.get<clayborne::position>(collision.self) };
        auto &velocity{ registry.get<clayborne::velocity>(collision.self) };
        auto &collider{ registry.get<clayborne::collider>(collision.self) };
        auto &renderer{ registry.get<clayborne::renderer>(collision.self) };

        // --------------------- //
        // Horizontal Collisions //
        // --------------------- //

        if (collision.normal_x != 0.0f) {
            if (player.wall_speed_retention_timer <= 0) {
                player.wall_speed_retention = velocity.x;
                player.wall_speed_retention_timer = player::wall_speed_retention_time;
            }

            velocity.x = 0;

            return;
        }

        // ------------------- //
        // Vertical Collisions //
        // ------------------- //

        // Reattach head if it falls on player
        if (player.head == collision.other && collision.normal_y > 0.0f) {
            player.is_head_attached = true;

            registry.destroy(player.head);
            player.head = entt::null;

            // Update player shape
            position.y -= player::hitbox_height - player::headless_hitbox_height;
            collider.h = player::hitbox_height;
            renderer.dstrect.h = player::hitbox_height;
        }

        if (velocity.y < 0.0f) {
            if (velocity.x <= 0.0f) {
                for (int i{ 1 }; i <= player::ceiling_corner_correction; i += 1) {
                    auto new_position{ position };
                    new_position.x -= static_cast<float>(i);
                    if (!overlap_any(registry, collision.self, new_position, collider)) {
                        position = new_position;
                        return;
                    }
                }
            }

            if (velocity.x >= 0.0f) {
                for (int i{ 1 }; i <= player::ceiling_corner_correction; i += 1) {
                    auto new_position{ position };
                    new_position.x += static_cast<float>(i);
                    if (!overlap_any(registry, collision.self, new_position, collider)) {
                        position = new_position;
                        return;
                    }
                }
            }
                
            if (player.jump_boost_timer < player::jump_boost_time - player::ceiling_jump_boost_grace) {
                player.jump_boost_timer = 0.0f;
            }
        }

        velocity.y = 0.0f;
    }

    entt::entity init_player(entt::registry &registry, float x, float y) noexcept {
        auto player_entity{ registry.create() };

        registry.emplace<player>(player_entity);
        registry.emplace<position>(player_entity, x, y);
        registry.emplace<velocity>(player_entity);

        auto &collider{ registry.emplace<clayborne::collider>(player_entity) };
        collider.w = player::hitbox_width;
        collider.h = player::hitbox_height;
        collider.collide = player_collision_handler;

        auto &renderer{ registry.emplace<clayborne::renderer>(player_entity) };
        renderer.dstrect.w = player::hitbox_width;
        renderer.dstrect.h = player::hitbox_height;
        
        return player_entity;
    }

    void update_player(entt::entity player_entity, entt::registry &registry, const input::manager &inputs, Uint64 dt_ns) noexcept {
        const float delta_time{ static_cast<float>(static_cast<double>(dt_ns) / SDL_NS_PER_SECOND) };

        for (auto event : inputs.get_events()) {
            // use event information???
            (void)event;
        }

        auto &player{ registry.get<clayborne::player>(player_entity)};
        auto &velocity{ registry.get<clayborne::velocity>(player_entity) };
        auto &position{ registry.get<clayborne::position>(player_entity) };
        auto &collider{ registry.get<clayborne::collider>(player_entity) };
        auto &renderer{ registry.get<clayborne::renderer>(player_entity) };

        // Check if grounded
        player.is_grounded = false;
        if (velocity.y >= 0) {
            auto below{ position };
            below.y += 1.0f;
            if (overlap_any(registry, player_entity, below, collider)) {
                player.is_grounded = true;
            }
            // auto view{ registry.view<const clayborne::position, const clayborne::collider>() };
            // for (auto [e, p, c] : view.each()) {
            //     if (player_entity == e) {
            //         continue;
            //     }
                
            //     if (clayborne::overlap(below, collider, p, c)) {
            //         player.is_grounded = true;
            //         break;
            //     }
            // }
        }

        // Update jump buffer timer
        if (player.jump_just_pressed) {
            player.jump_buffer_timer = player::jump_buffer_time;
        }
        if (player.jump_buffer_timer > 0.0f) {
            player.jump_buffer_timer -= delta_time;
        }

        // Update jump grace timer
        if (player.is_grounded) {
            player.jump_grace_timer = player::jump_grace_time;
        }
        else if (player.jump_grace_timer > 0.0f) {
            player.jump_grace_timer -= delta_time;
        }

        // Update jump boost timer
        if (player.jump_boost_timer > 0.0f) {
            player.jump_boost_timer -= delta_time;
        }
                    
        // Update facing
        if (velocity.x > 0.0f) {
            player.facing = player::facing::right;
        }
        else if (velocity.x < 0.0f) {
            player.facing = player::facing::left;
        }

        // Moving into a wall retains speed for a short duration
        if (player.wall_speed_retention_timer > 0.0f) {
            if (velocity.x * player.wall_speed_retention < 0.0f) {
                player.wall_speed_retention_timer = 0.0f;
            }
            else {
                auto p{ position };
                p.x += static_cast<float>(player.wall_speed_retention > 0.0f) + static_cast<float>(player.wall_speed_retention < 0.0f);
                
                bool is_wall{ overlap_any(registry, player_entity, p, collider) };
                // auto view{ registry.view<const clayborne::position, const clayborne::collider>() };
                // for (auto [e2, p2, c2] : view.each()) {
                //     (void)e2;
                //     if (overlap(position, collider, p2, c2)) {
                //         is_wall = true;
                //         break;
                //     }
                // }

                if (!is_wall) {
                    velocity.x = player.wall_speed_retention;
                    player.wall_speed_retention_timer = 0.0f;
                }
                else {
                    player.wall_speed_retention_timer -= delta_time;
                }
            }
        }

        // ------------------- //
        // Horizontal Movement //
        // ------------------- //

        const float move_input{ static_cast<float>(player.right - player.left) };
        const float velocity_sign{ static_cast<float>((velocity.x > 0.0f) - (velocity.x < 0.0f)) };
        const float multiplier{ player.is_grounded ? 1.0f : player.air_multiplier };

        if (std::abs(velocity.x) > player::run_speed && velocity_sign == move_input) {
            velocity.x = approach(velocity.x, move_input * player::run_speed, player::run_deceleration * multiplier * delta_time);
        }
        else {
            velocity.x = approach(velocity.x, move_input * player::run_speed, player::run_acceleration * multiplier * delta_time);
        }

        // ----------------- //
        // Vertical Movement //
        // ----------------- //

        // The apex of the jump has lower gravity if holding the jump button
        if (!player.is_grounded) {
            bool is_half_gravity_applicable{ std::abs(velocity.y) < player::half_gravity_threshold };
            float mult = (is_half_gravity_applicable && player.jump_pressed) ? 0.5f : 1.0f;
            velocity.y = approach(velocity.y, player::fall_speed, player::gravity * mult * delta_time);
        }

        // Holding the jump button from the start gives higher jumps
        if (player.jump_boost_timer > 0.0f) {
            if (player.jump_pressed) {
                velocity.y = std::min(velocity.y, player.jump_boost_speed);
            }
            else {
                player.jump_boost_timer = 0.0f;
            }
        }

        bool is_buffered_jump{ player.jump_pressed && player.is_grounded && player.jump_buffer_timer > 0.0f };
        bool is_coyote_jump{ player.jump_just_pressed && player.jump_grace_timer > 0.0f };
        if (is_buffered_jump || is_coyote_jump) {
            player.jump_buffer_timer = 0.0f;
            player.jump_grace_timer = 0.0f;
            player.jump_boost_timer = player::jump_boost_time;

            velocity.x += player::jump_horizontal_speed * move_input;
            velocity.y = player::jump_vertical_speed;
            // jump pad boost goes here
            player.jump_boost_speed = velocity.y;
        }

        // Reset jump buffer timer on ground
        if (player.is_grounded) {
            player.jump_buffer_timer = 0.0f;
        }

        // ---- //
        // Head //
        // ---- //

        if (player.head_just_pressed) {
            // Throw head
            if (player.is_head_attached) {
                printf("throw\n");
                // Update player shape
                position.y += player::hitbox_height - player::headless_hitbox_height;
                collider.h = player::headless_hitbox_height;
                renderer.dstrect.h = player::headless_hitbox_height;

                const float x{ static_cast<float>(player.right - player.left) };
                const float y{ static_cast<float>(player.down - player.up) };
                const float len2{ x * x + y * y };

                player.is_head_attached = false;
                player.head = registry.create();
                registry.emplace<clayborne::head>(player.head);

                auto &head_position{ registry.emplace<clayborne::position>(player.head) };
                head_position = position;
                head_position.x += x * player::hitbox_width;
                head_position.y += y * player::headless_hitbox_height;
                
                auto &head_velocity{ registry.emplace<clayborne::velocity>(player.head) };
                if (len2 == 0.0f) {
                    const int direction{ (player.facing == player::facing::right) - (player.facing == player::facing::left) };
                    head_velocity.x = static_cast<float>(direction) * head::throw_speed;
                    head_velocity.y = 0.0f;
                }
                else {
                    const float invlen{ (len2 == 2.0f) ? 0.70710678f : 1 };
                    head_velocity.x = x * invlen * head::throw_speed;
                    head_velocity.y = y * invlen * head::throw_speed;
                }

                auto &head_collider{ registry.emplace<clayborne::collider>(player.head) };
                head_collider.w = head::hitbox_width;
                head_collider.h = head::hitbox_height;

                auto &head_renderer{ registry.emplace<clayborne::renderer>(player.head) };
                head_renderer.dstrect.w = head::hitbox_width;
                head_renderer.dstrect.h = head::hitbox_height;
            }
            // Detonate head
            else if (player.head != entt::null) {
                printf("detonate\n");
                registry.destroy(player.head);
                player.head = entt::null;
            }
        }

        // Update head
        // TODO: maybe move to separate system
        if (player.head != entt::null) {
            auto &head{ registry.get<clayborne::head>(player.head) };
            auto &head_position{ registry.get<clayborne::position>(player.head) };
            auto &head_velocity{ registry.get<clayborne::velocity>(player.head) };
            auto &head_collider{ registry.get<clayborne::collider>(player.head) };

            // Apply gravity
            if (!head.is_grounded) {
                head_velocity.y = approach(head_velocity.y, head::fall_speed, head::gravity * delta_time);
            }

            // Check if grounded
            head.is_grounded = false;
            if (head_velocity.y >= 0) {
                auto below{ head_position };
                below.y += 1.0f;
                if (overlap_any<clayborne::clay>(registry, player.head , below, head_collider)) {
                    head.is_grounded = true;
                }
            }
        }

        // ------------------------ //
        // Temporary Input Handling //
        // ------------------------ //
        player.jump_just_pressed = false;
        player.head_just_pressed = false;
        // ------------------------ //
    }
}