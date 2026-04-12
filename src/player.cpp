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
#include "utils.hpp"

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

        if (collision.normal.x != 0.0f) {
            if (player.wall_speed_retention_timer <= 0) {
                player.wall_speed_retention = velocity.x;
                player.wall_speed_retention_timer = player::wall_speed_retention_time;
            }

            velocity.x = 0.0f;

            return;
        }

        // ------------------- //
        // Vertical Collisions //
        // ------------------- //

        // Reattach head if it falls on player
        if (player.head == collision.other && collision.normal.y > 0.0f) {
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
                    new_position.y -= 1;
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
                    new_position.y -= 1;
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

    static void head_collision_handler(entt::registry &registry, const collider::collision &collision) {
        auto &head{ registry.get<clayborne::head>(collision.self) };
        auto &velocity{ registry.get<clayborne::velocity>(collision.self) };

        head.is_thrown = false;
        velocity.x = 0.0f;
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
                auto new_position{ position };
                new_position.x += sgn(player.wall_speed_retention);
                if (!overlap_any(registry, player_entity, new_position, collider)) {
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
                
                auto &head_velocity{ registry.emplace<clayborne::velocity>(player.head) };
                if (len2 == 0.0f) {
                    const int direction{ (player.facing == player::facing::right) - (player.facing == player::facing::left) };
                    head_velocity.x = static_cast<float>(direction) * head::throw_speed;
                    head_velocity.y = 0.0f;
                    head_position.x += static_cast<float>(direction) * player::hitbox_width;
                }
                else {
                    // const float invlen{ (len2 == 2.0f) ? 0.70710678f : 1 };
                    head_velocity.x = x * 1 * head::throw_speed;
                    head_velocity.y = y * 1 * head::throw_speed;
                    head_position.x += x * player::hitbox_width;
                    head_position.y += y * player::headless_hitbox_height;
                }

                auto &head_collider{ registry.emplace<clayborne::collider>(player.head) };
                head_collider.w = head::hitbox_width;
                head_collider.h = head::hitbox_height;
                head_collider.collide = head_collision_handler;

                auto &head_renderer{ registry.emplace<clayborne::renderer>(player.head) };
                head_renderer.dstrect.w = head::hitbox_width;
                head_renderer.dstrect.h = head::hitbox_height;
            }
            // Detonate head
            // TODO: make detonation continue over a period of time
            // TODO: trigger other things that can react to explosions
            // TODO: explosion should move the player in a fixed trajectory, rather than physics-based
            else if (player.head != entt::null) {
                auto &head_position{ registry.get<clayborne::position>(player.head) };
                auto &head_collider{ registry.get<clayborne::collider>(player.head) };

                const float explosion_center_x{ head_position.x + head_collider.w * 0.5f };
                const float explosion_center_y{ head_position.y + head_collider.h * 0.5f };

                const clayborne::position explosion_position{
                    .x = explosion_center_x - head::explosion_radius,
                    .y = explosion_center_y - head::explosion_radius,
                };

                const clayborne::collider explosion_collider{
                    .w = 2.0f * head::explosion_radius,
                    .h = 2.0f * head::explosion_radius,
                };

                if (overlap(explosion_position, explosion_collider, position, collider)) {
                    const float center_x{ position.x + collider.w * 0.5f };
                    const float center_y{ position.y + collider.h * 0.5f };
                    const float delta_x{ center_x - explosion_center_x };
                    const float delta_y{ center_y - explosion_center_y };
                    static const float inv_sqrt2{ 0.70710678f };
                    static const float pi{ 3.14159265358979323846f };
                    static const clayborne::velocity directions[8] = {
                        { 1.0f,  0.0f },
                        { inv_sqrt2,  inv_sqrt2 },
                        { 0.0f,  1.0f },
                        { -inv_sqrt2,  inv_sqrt2 },
                        { -1.0f,  0.0f },
                        { -inv_sqrt2, -inv_sqrt2 },
                        { 0.0f, -1.0f },
                        { inv_sqrt2, -inv_sqrt2 }
                    };
                    const float angle = std::atan2(delta_y, delta_x);
                    const int octant = static_cast<int>(std::round(angle / (pi / 4.0f))) & 7;
                    velocity.x = directions[octant].x * head::explosion_speed;
                    velocity.y = directions[octant].y * head::explosion_speed;
                }

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

            // Update throw movement
            if (head.is_thrown) {
                head_velocity.x = approach(head_velocity.x, 0.0f, head::throw_deceleration * delta_time);
                head_velocity.y = approach(head_velocity.y, 0.0f, head::throw_deceleration * delta_time);

                constexpr float tolerance{ 0.0001f };
                if (std::abs(head_velocity.x) <= tolerance && std::abs(head_velocity.y) <= tolerance) {
                    head.is_thrown = false;
                    head_velocity.x = 0.0f;
                    head_velocity.y = 0.0f;
                }
            }
            else {
                if (!head.is_grounded) {
                    head_velocity.y = approach(head_velocity.y, head::fall_speed, head::gravity * delta_time);
                }

                // Check if grounded
                head.is_grounded = false;
                if (head_velocity.y >= 0) {
                    auto below{ head_position };
                    below.y += 1.0f;
                    if (overlap_any(registry, player.head , below, head_collider)) {
                        head.is_grounded = true;
                    }
                }
            }
        }
        // Regrow head
        else if (!player.is_head_attached) {
            if (player.is_grounded) {
                auto below{ position };
                below.y += 1.0f;
                if (overlap_any<clayborne::clay>(registry, player_entity , below, collider)) {
                    auto above{ position };
                    above.y -= 1.0f;
                    if (!overlap_any(registry, player_entity , above, collider)) {
                        player.is_head_attached = true;
                        position.y -= player::hitbox_height - player::headless_hitbox_height;
                        collider.h = player::hitbox_height;
                        renderer.dstrect.h = player::hitbox_height;
                    }
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