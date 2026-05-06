#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <entt/entt.hpp>
#include <optional>
#include "player.hpp"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_surface.h"
#include "physics.hpp"
#include "clay.hpp"
#include "sprite.hpp"
#include "utils.hpp"
#include "interactables.hpp"
#include "head.hpp"

using entt::literals::operator""_hs;

namespace clayborne {
    // TODO perhaps change magic numbers to header consts or something.
    static void set_player_tall(bool tall, struct sprite_renderer &renderer) {
        if (tall) {
            renderer.srcrect.w = player::hitbox_width;
            renderer.srcrect.h = player::hitbox_height + 5.0f;
            renderer.srcrect.x = 4.0f;
            renderer.srcrect.y = 0.0f;
            renderer.y_offset = -5.0f;
        }
        else {
            renderer.srcrect.w = player::hitbox_width;
            renderer.srcrect.h = player::headless_hitbox_height;
            renderer.srcrect.x = 4.0f;
            renderer.srcrect.y = 8.0f;
            renderer.y_offset = -2.0f;
        }
    }

    static void player_collision_handler(entt::registry &registry, const collider::collision &collision) noexcept {
        auto &player{ registry.get<clayborne::player>(collision.self) };
        auto &position{ registry.get<clayborne::position>(collision.self) };
        auto &velocity{ registry.get<clayborne::velocity>(collision.self) };
        auto &collider{ registry.get<clayborne::collider>(collision.self) };
        auto &sprite_renderer{ registry.get<struct sprite_renderer>(collision.self) };
        auto &activator{ registry.get<clayborne::activator>(collision.self) };

        // --------------------- //
        // Horizontal Collisions //
        // --------------------- //

        if (collision.normal.x != 0.0f) {
            if (player.state == player::state::launched) {
                if (corner_correction<true, player::head_launch_corner_correction>(
                    registry,
                    collision.self,
                    position,
                    velocity,
                    collider
                )) {
                    return;
                }
            }

            if (player.wall_speed_retention_timer <= 0.0f) {
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
        if (registry.any_of<struct head>(collision.other) && collision.normal.y > 0.0f) {
            player.is_head_attached = true;
            player.is_head_detonated = true;

            const auto head_position{ registry.get<const clayborne::position>(collision.other) };

            registry.destroy(collision.other);

            auto above{ position };
            above.y -= 1.0f;
            while (overlap_any(registry, collision.self, above, collider)) {
                above.x += sgn(head_position.x - position.x);
            }
            position.x = above.x;

            // Update player shape
            position.y -= player::hitbox_height - player::headless_hitbox_height;
            collider.h = player::hitbox_height;
            activator.h = player::hitbox_height;
            set_player_tall(true, sprite_renderer);
        }

        if (player.state == player::state::launched) {
            if (corner_correction<false, player::head_launch_corner_correction>(
                registry,
                collision.self,
                position,
                velocity,
                collider
            )) {
                return;
            }
        }

        if (velocity.y < 0.0f) {
            // Left corner correction
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

            // Right corner correction
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
            
            // Head bonk jump boost grace period
            if (player.jump_boost_timer < player::jump_boost_duration - player::ceiling_jump_boost_grace) {
                player.jump_boost_timer = 0.0f;
            }
        }

        velocity.y = 0.0f;
    }

    static clayborne::velocity compute_throw_velocity(const clayborne::player &player) {
        const float x{ static_cast<float>(player.right - player.left) };
        const float y{ static_cast<float>(player.down - player.up) };
        const float len2{ x * x + y * y };

        clayborne::velocity result;
        result.subpos_x = 0.0f;
        result.subpos_y = 0.0f;

        if (len2 == 0.0f) {
            const int direction{ (player.facing == player::facing::right) - (player.facing == player::facing::left) };
            result.x = static_cast<float>(direction) * player::head_throw_speed;
            result.y = 0.0f;
        }
        else {
            const float invlen{ (len2 == 2.0f) ? inv_sqrt2 : 1 };
            result.x = x * invlen * player::head_throw_speed;
            result.y = y * invlen * player::head_throw_speed;
        }

        return result;
    }

    template<typename F>
    static void pay_debt(int &debt, clayborne::position &p, const float dx, const float dy, F &&f) {
        while (debt > 0) {
            auto next{ p };
            next.x += dx;
            next.y += dy;
            if (f(next)) {
                return;
            }
            p = next;
            debt -= 1;
        }
    }

    static void respawn_player(entt::registry &registry, player &p, position &pos, velocity &vel) noexcept {
        p.state = player::state::start;
        vel.x = 0.0f;
        vel.y = 0.0f;

        // Respawn on head if it is on clay
        const auto head_entity{ registry.view<struct head>().front() };
        if (head_entity != entt::null) {
            auto head_position{ registry.get<const position>(head_entity) };
            auto head_collider{ registry.get<const collider>(head_entity) };
            auto below{ head_position };
            below.y += 1.0f;
            if (overlap_any<clay>(registry, head_entity, below, head_collider)) {
                registry.destroy(head_entity);
                pos.x = head_position.x;
                pos.y = head_position.y;
                return;
            }
        }
        
        pos.x = p.respawn_x;
        pos.y = p.respawn_y;
    }

    entt::entity init_player(
        entt::registry &registry,
        float x,
        float y
    ) noexcept {
        auto player_entity{ registry.create() };

        auto &player_player{ registry.emplace<player>(player_entity) };
        player_player.respawn_x = x;
        player_player.respawn_y = y;

        registry.emplace<position>(player_entity, x, y);
        registry.emplace<velocity>(player_entity);
        registry.emplace<activator>(player_entity, player::hitbox_width, player::hitbox_height);

        auto &collider{ registry.emplace<struct collider>(player_entity) };
        collider.w = player::hitbox_width;
        collider.h = player::headless_hitbox_height;;
        collider.collide = player_collision_handler;

        auto &sprite_renderer{ registry.emplace<struct sprite_renderer>(player_entity) };
        sprite_renderer.z = 1;

        registry.emplace<struct sprite_animator>(player_entity);

        set_player_tall(false, sprite_renderer);
        
        return player_entity;
    }

    void update_player(
        entt::entity player_entity,
        entt::registry &registry,
        const input::manager &inputs,
        Uint64 dt_ns,
        // TODO: Replace with events
        audio_cache &sounds,
        // TODO: Replace with events
        MIX_Mixer *mixer
    ) noexcept {
        const float delta_time{ static_cast<float>(static_cast<double>(dt_ns) / SDL_NS_PER_SECOND) };

        for (auto event : inputs.get_events()) {
            // use event information???
            (void)event;
        }

        auto &player{ registry.get<clayborne::player>(player_entity) };
        auto &velocity{ registry.get<clayborne::velocity>(player_entity) };
        auto &position{ registry.get<clayborne::position>(player_entity) };
        auto &collider{ registry.get<clayborne::collider>(player_entity) };
        auto &renderer{ registry.get<struct sprite_renderer>(player_entity) };
        auto &activator{ registry.get<clayborne::activator>(player_entity) };

        // ----------------------------- //
        // Update States, Flags & Timers //
        // ----------------------------- //

        if (player.state == player::state::dead) {
            respawn_player(registry, player, position, velocity);
        }

        // Check if grounded
        const bool player_was_grounded = player.is_grounded;
        player.is_grounded = false;
        if (velocity.y >= 0.0f) {
            auto below{ position };
            below.y += 1.0f;
            if (overlap_any(registry, player_entity, below, collider)) {
                player.is_grounded = true;
            }
        }
        player.is_landing = !player_was_grounded && player.is_grounded;

        // Check if standing on clay
        player.is_on_clay = false;
        if (player.is_grounded) {
            auto below{ position };
            below.y += 1.0f;
            if (overlap_any<clayborne::clay>(registry, player_entity , below, collider)) {
                player.is_on_clay = true;
                player.respawn_x = position.x;
                player.respawn_y = position.y;
            }
        }

        // Check if head has been destroyed
        if (registry.view<struct head>().front() == entt::null) {
            player.is_head_detonated = true;
        }

        // Update jump buffer timer
        if (player.jump_just_pressed) {
            player.jump_buffer_timer = player::jump_buffer_duration;
        }
        if (player.jump_buffer_timer > 0.0f) {
            player.jump_buffer_timer -= delta_time;
        }

        // Update jump grace timer
        if (player.is_grounded) {
            player.jump_grace_timer = player::jump_grace_duration;
        }
        else if (player.jump_grace_timer > 0.0f) {
            player.jump_grace_timer -= delta_time;
        }

        // Update jump boost timer
        if (player.jump_boost_timer > 0.0f) {
            player.jump_boost_timer -= delta_time;
        }
                    
        // Update facing
        if (velocity.x > 0.0f && player.right) {
            player.facing = player::facing::right;
        }
        else if (velocity.x < 0.0f && player.left) {
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

        // Update head buffer timer
        if (player.head_just_pressed) {
            player.head_buffer_timer = player::head_buffer_duration;
        }
        if (player.head_buffer_timer > 0.0f) {
            player.head_buffer_timer -= delta_time;
        }

        // Update throwing state
        if (player.state == player::state::throwing) {
            if (player.head_throw_timer > 0.0f) {
                player.head_throw_timer -= delta_time;
            }
            else {
                player.state = player::state::start;
            }
        }

        // Update launched state
        if (player.state == player::state::launched) {
            if (player.head_launch_timer > 0.0f) {
                player.head_launch_timer -= delta_time;
            }
            else {
                player.state = player::state::start;
                auto new_velocity = normalize({ velocity.x, velocity.y }) * player::head_launch_end_speed;
                velocity.x = new_velocity.x;
                velocity.y = new_velocity.y;
            }
        }

        // ------------------- //
        // Horizontal Movement //
        // ------------------- //

        if (player.state == player::state::start || player.state == player::state::throwing) {
            const float move_input{ static_cast<float>(player.right - player.left) };
            const float velocity_sign{ sgn(velocity.x) };
            const float multiplier{ player.is_grounded ? 1.0f : player.air_multiplier };

            if (std::abs(velocity.x) > player::run_speed && velocity_sign == move_input) {
                velocity.x = approach(velocity.x, move_input * player::run_speed, player::run_deceleration * multiplier * delta_time);
            }
            else {
                velocity.x = approach(velocity.x, move_input * player::run_speed, player::run_acceleration * multiplier * delta_time);
            }
        }

        // -------------- //
        // Jump Mechanics //
        // -------------- //

        if (player.state == player::state::start) {
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
                player.is_grounded = false;
                player.jump_buffer_timer = 0.0f;
                player.jump_grace_timer = 0.0f;
                player.jump_boost_timer = player::jump_boost_duration;

                const float move_input{ static_cast<float>(player.right - player.left) };

                velocity.x += player::jump_horizontal_speed * move_input;
                velocity.y = player::jump_vertical_speed;
                player.jump_boost_speed = velocity.y;

                // TODO: Replace with audio event sink
                (void)play_sound(registry, sounds, mixer, "data/jump.wav"_hs, 1.0f, false);
            }

            // Reset jump buffer timer on ground
            if (player.is_grounded) {
                player.jump_buffer_timer = 0.0f;
            }
        }

        // -------------- //
        // Head Mechanics //
        // -------------- //

        // Regrow head
        if (player.state == player::state::start && !player.is_head_attached && player.is_head_detonated && player.is_on_clay) {
            auto above{ position };
            above.y -= 1.0f;
            if (!overlap_any(registry, player_entity , above, collider)) {
                player.is_head_attached = true;
                position.y -= player::hitbox_height - player::headless_hitbox_height;
                collider.h = player::hitbox_height;
                set_player_tall(true, renderer);
                SDL_Log("Player regrew head");
            }
        }

        // Throw/detonate head
        if (player.state == player::state::start || player.state == player::state::launched) {
            bool is_buffered_head_action{ player.head_pressed && player.head_buffer_timer > 0.0f };
            if (is_buffered_head_action) {
                player.head_buffer_timer = 0.0f;

                // Throw head
                if (player.is_head_attached) {
                    auto head_velocity{ compute_throw_velocity(player) };

                    clayborne::position new_position{ position };
                    new_position.y += player::hitbox_height - player::headless_hitbox_height;
                    clayborne::collider new_collider{ collider };
                    new_collider.h = player::headless_hitbox_height;

                    clayborne::position head_position{ new_position };
                    clayborne::collider head_collider{ head::hitbox_width, head::hitbox_height, head_collision_handler };
                    const float dir_x{ sgn(head_velocity.x) };
                    const float dir_y{ sgn(head_velocity.y) };
                    int debt_x = (dir_x != 0.0f) ? static_cast<int>(player::hitbox_width) : 0;
                    int debt_y = (dir_y != 0.0f) ? static_cast<int>(player::headless_hitbox_height) : 0;

                    // Throwing diagonally upwards prioritizes the y-axis
                    if (dir_y < 0.0f) {
                        // Move head out of player along the y-axis
                        pay_debt(debt_y, head_position, 0.0f, dir_y, [&](const clayborne::position next) {
                            return overlap_any(registry, entt::null, next, head_collider, entt::exclude<clayborne::player>);
                        });
                        // Move player out of head along the y-axis
                        pay_debt(debt_y, new_position, 0.0f, -dir_y, [&](const clayborne::position next) {
                            return overlap_any(registry, player_entity, next, new_collider, entt::exclude<clayborne::head>);
                        });

                        // Skip the x-axis movement if the y-axis movement was sufficient
                        if (overlap(new_position, new_collider, head_position, head_collider)) {
                            // Move head out of player along the x-axis
                            pay_debt(debt_x, head_position, dir_x, 0.0f, [&](const clayborne::position next) {
                                return overlap_any(registry, entt::null, next, head_collider, entt::exclude<clayborne::player>);
                            });

                            // Move player out of head along the x-axis
                            pay_debt(debt_x, new_position, -dir_x, 0.0f, [&](const clayborne::position next) {
                                return overlap_any(registry, player_entity, next, new_collider, entt::exclude<clayborne::head>);
                            });
                        }
                    }
                    // Throwing diagonally downards prioritizes the x-axis
                    else {
                        // Move head out of player along the x-axis
                        pay_debt(debt_x, head_position, dir_x, 0.0f, [&](const clayborne::position next) {
                            return overlap_any(registry, entt::null, next, head_collider, entt::exclude<clayborne::player>);
                        });

                        // Move player out of head along the x-axis
                        pay_debt(debt_x, new_position, -dir_x, 0.0f, [&](const clayborne::position next) {
                            return overlap_any(registry, player_entity, next, new_collider, entt::exclude<clayborne::head>);
                        });

                        // Skip the y-axis movement if the x-axis movement was sufficient
                        if (overlap(new_position, new_collider, head_position, head_collider)) {
                            // Move head out of player along the y-axis
                            pay_debt(debt_y, head_position, 0.0f, dir_y, [&](const clayborne::position next) {
                                return overlap_any(registry, entt::null, next, head_collider, entt::exclude<clayborne::player>);
                            });
                            // Move player out of head along the y-axis
                            pay_debt(debt_y, new_position, 0.0f, -dir_y, [&](const clayborne::position next) {
                                return overlap_any(registry, player_entity, next, new_collider, entt::exclude<clayborne::head>);
                            });
                        }
                    }

                    // Throw succeeded
                    if (!overlap(new_position, new_collider, head_position, head_collider)) {
                        player.is_head_attached = false;
                        player.is_head_detonated = false;
                        player.state = player::state::throwing;
                        player.head_throw_timer = player::head_throw_duration;
                        player.wall_speed_retention_timer = 0.0f;
                        position = new_position;
                        velocity.y = 0.0f;
                        collider.h = player::headless_hitbox_height;
                        activator.h = player::headless_hitbox_height;
                        set_player_tall(false, renderer);

                        const auto head_entity = registry.create();
                        auto &head{ registry.emplace<clayborne::head>(head_entity) };
                        head.state = head::state::thrown;
                        head.throw_timer = player::head_throw_duration;
                        registry.emplace<clayborne::position>(head_entity, head_position);
                        registry.emplace<clayborne::velocity>(head_entity, head_velocity);
                        registry.emplace<clayborne::collider>(head_entity, head_collider);
                        registry.emplace<clayborne::activator>(head_entity, head::hitbox_width, head::hitbox_height);
                        
                        auto &head_renderer{ registry.emplace<struct sprite_renderer>(head_entity) };
                        head_renderer.texture = "data/textures/player/head.png"_hs;
                        head_renderer.srcrect.w = 8.0f;
                        head_renderer.srcrect.h = 8.0f;
                        head_renderer.flip = player.facing == player::facing::left ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
                        head_renderer.z = 1;
                    }
                    // Throw failed
                    // We still enter the throw state, but the player keeps their head and aren't moved
                    else {
                        player.is_head_attached = true;
                        player.is_head_detonated = true;
                        player.state = player::state::throwing;
                        player.head_throw_timer = player::head_throw_duration;
                        velocity.y = 0.0f;
                    }
                }
                // Detonate head
                else {
                    player.is_head_detonated = true;
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

    void animate_player(
        entt::entity player_entity,
        entt::registry &registry,
        animation_cache &animations
    ) noexcept {
        auto player{ registry.get<struct player>(player_entity) };

        const auto velocity{ registry.get<const struct velocity>(player_entity) };
        auto &sprite_renderer{ registry.get<struct sprite_renderer>(player_entity) };
        auto &sprite_animator{ registry.get<struct sprite_animator>(player_entity) };

        // TODO: something that is not this ugly mess
        auto play{
            [&](std::string filename){
                const entt::hashed_string texture{ 
                    ("data/textures/player/" + filename + ".png").c_str()
                };

                const entt::hashed_string animation{
                    ("data/animations/player/" + filename + ".json").c_str()
                };

                if (sprite_animator.animation == animation) {
                    // SDL_Log(
                    //     "Animation %s continued: frame %d, x: %f, y: %f",
                    //     filename.c_str(),
                    //     static_cast<int>(sprite_animator.current_frame),
                    //     static_cast<double>(animations[sprite_animator.animation]->frames[sprite_animator.current_frame - 1].x),
                    //     static_cast<double>(animations[sprite_animator.animation]->frames[sprite_animator.current_frame - 1].y)
                    // );
                    return;
                }

                SDL_Log("Animation changed to %s", filename.c_str());
                
                sprite_renderer.texture = texture;
                sprite_animator.animation = animation;
                sprite_animator.current_frame = 0;
            }
        };

        switch (player.state) {
        case player::state::start:
            if (player.is_grounded) {
                if (
                    player.is_landing || (
                        (
                            sprite_animator.animation == "data/animations/player/land.json"_hs ||
                            sprite_animator.animation == "data/animations/player/land_headless.json"_hs
                        ) && 
                        sprite_animator.current_frame < animations[sprite_animator.animation]->frames.size()
                    )
                ) {
                    if (player.is_head_attached) play("land");
                    else                         play("land_headless");
                    sprite_animator.is_looping = false;
                }
                else if (player.left == player.right) {
                    if (player.is_head_attached) play("idle");
                    else                         play("idle_headless");
                    sprite_animator.is_looping = true;
                }
                else {
                    if (player.is_head_attached) play("run");
                    else                         play("run_headless");
                    sprite_animator.is_looping = true;
                }
            }
            else if (velocity.y < 0.0f) {
                if (player.is_head_attached) play("jump");
                else                         play("jump_headless");
                sprite_animator.is_looping = false;
            }
            else {
                if (player.is_head_attached) play("fall");
                else                         play("fall_headless");
                sprite_animator.is_looping = true;
            }
            break;
        case player::state::throwing:
            play("throw_head");
            sprite_animator.is_looping = false;
            break;
        default:
            break;
        }

        if (player.facing == player::facing::left) {
            sprite_renderer.flip = SDL_FLIP_HORIZONTAL;
        }
        else {
            sprite_renderer.flip = SDL_FLIP_NONE;
        }
    } 
}