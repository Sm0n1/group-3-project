#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <cmath>
#include <cstdio>
// #include <chrono>
#include <entt/entt.hpp>
#include "physics.hpp"

namespace clayborne {
    void update_physics(entt::registry &registry, Uint64 dt_ns) {
        // const auto start = std::chrono::steady_clock::now();

        const float delta_time{ static_cast<float>(static_cast<double>(dt_ns) / SDL_NS_PER_SECOND )};

        // Move entities that cannot collide
        auto noncollidable_view{ registry.view<position, const velocity>(entt::exclude<collider>) };
        for (auto [entity, position, velocity]: noncollidable_view.each()) {
            position.x += velocity.x * delta_time;
            position.y += velocity.y * delta_time;
        }
        
        // Move entities that may collide
        auto collidable_view{ registry.view<position, velocity, const collider>() };
        for (auto [self, self_position, self_velocity, self_collider]: collidable_view.each()) {
            self_velocity.subpos_x += self_velocity.x * delta_time;
            self_velocity.subpos_y += self_velocity.y * delta_time;

            auto move{ round(vec2{ self_velocity.subpos_x, self_velocity.subpos_y }) };
            const auto sign{ sgn(move) };

            self_velocity.subpos_x -= move.x;
            self_velocity.subpos_y -= move.y;

            // Move one pixel at a time in the x axis
            while (move.x != 0) {
                auto self_new_position{ self_position };
                self_new_position.x += sign.x;
                move.x -= sign.x;

                bool is_collision{ false };
                auto view{ registry.view<const position, const collider>() };
                for (auto [other, other_position, other_collider]: view.each()) {
                    if (self == other) {
                        continue;
                    }

                    if (overlap(self_new_position, self_collider, other_position, other_collider)) {
                        is_collision = true;
                        if (self_collider.collide) {
                            (*self_collider.collide)(registry, {self, other, { -sign.x, 0.0f } });
                            const auto other_collider_opt{ registry.try_get<clayborne::collider>(other) };
                            if (other_collider_opt && (*other_collider_opt).collide) {
                                (*other_collider_opt->collide)(registry, { other, self, { sign.x, 0.0f } });
                            }
                        }
                        else if (other_collider.collide) {
                            (*other_collider.collide)(registry, { other, self, { sign.x, 0.0f } });
                        }
                    }
                }

                if (is_collision) {
                    break;
                } else {
                    self_position = self_new_position;
                }
            }

            // Move one pixel at a time in the y axis
            while (move.y != 0) {
                position self_new_position{ self_position };
                self_new_position.y += sign.y;
                move.y -= sign.y;

                bool is_collision{ false };
                auto view{ registry.view<const position, const collider>() };
                for (auto [other, other_position, other_collider]: view.each()) {
                    if (self == other) {
                        continue;
                    }
                    
                    if (overlap(self_new_position, self_collider, other_position, other_collider)) {
                        is_collision = true;
                        if (self_collider.collide) {
                            (*self_collider.collide)(registry, { self,  other, { 0.0f, -sign.y } });
                            const auto other_collider_opt{ registry.try_get<clayborne::collider>(other) };
                            if (other_collider_opt && (*other_collider_opt).collide) {
                                (*other_collider_opt->collide)(registry, { other, self, { 0.0f, sign.y } });
                            }
                        }
                        else if (other_collider.collide) {
                            (*other_collider.collide)(registry, { other, self, { 0.0f, sign.y } });
                        }
                    }
                }

                if (is_collision) {
                    break;
                } else {
                    self_position = self_new_position;
                }
            }
        }

        // const auto end = std::chrono::steady_clock::now();
        // const auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        // const auto percent_of_frame{ (static_cast<double>(total_ns) / (SDL_NS_PER_SECOND / 60.0)) * 100.0 };
        // printf("physics_time: %.2f%% of 60 FPS frame\n", percent_of_frame);
    }
}