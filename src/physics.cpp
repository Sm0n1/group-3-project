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

        float delta_time{ static_cast<float>(static_cast<double>(dt_ns) / SDL_NS_PER_SECOND )};

        // Move entities that cannot collide
        auto noncollidable_view{ registry.view<position, const velocity>(entt::exclude<collider>) };
        for (auto [entity, pos, vel]: noncollidable_view.each()) {
            pos.x += vel.x * delta_time;
            pos.y += vel.y * delta_time;
        }
        
        // Move entities that may collide
        auto collidable_view{ registry.view<position, velocity, const collider>() };
        for (auto [self, self_position, self_velocity, self_collider]: collidable_view.each()) {
            self_velocity.x_subpos += self_velocity.x * delta_time;
            self_velocity.y_subpos += self_velocity.y * delta_time;

            auto x_move{ std::round(self_velocity.x_subpos) };
            auto y_move{ std::round(self_velocity.y_subpos) };
            auto x_sgn{ static_cast<float>(x_move > 0.0f) - static_cast<float>(x_move < 0.0f) };
            auto y_sgn{ static_cast<float>(y_move > 0.0f) - static_cast<float>(y_move < 0.0f) };

            self_velocity.x_subpos -= x_move;
            self_velocity.y_subpos -= y_move;

            // Move one pixel at a time in the x axis
            while (x_move != 0) {
                x_move -= x_sgn;
                self_position.x += x_sgn;

                bool is_collision = false;
                auto view{ registry.view<const position, const collider>() };
                for (auto [other, other_position, other_collider]: view.each()) {
                    if (self == other) {
                        continue;
                    }

                    if (overlap(self_position, self_collider, other_position, other_collider)) {
                        is_collision = true;
                        if (self_collider.collide) {
                            self_collider.collide.value()(registry, {self, other, -x_sgn, 0.0f});
                        }
                        if (other_collider.collide) {
                            other_collider.collide.value()(registry, {other, self, x_sgn, 0.0f});
                        }
                    }
                }

                // Undo move if it caused a collision
                if (is_collision) {
                    self_position.x -= x_sgn; // undo move
                    break;
                }
            }

            // Move one pixel at a time in the y axis
            while (y_move != 0) {
                y_move -= y_sgn;
                self_position.y += y_sgn;

                bool is_collision = false;
                auto view{ registry.view<const position, const collider>() };
                for (auto [other, other_position, other_collider]: view.each()) {
                    if (self == other) {
                        continue;
                    }
                    
                    if (overlap(self_position, self_collider, other_position, other_collider)) {
                        is_collision = true;
                        if (self_collider.collide) {
                            self_collider.collide.value()(registry, {self,  other, 0.0f, -y_sgn });
                        }
                        if (other_collider.collide) {
                            other_collider.collide.value()(registry, {other, self, 0.0f, y_sgn });
                        }
                    }
                }

                // Undo move if it caused a collision 
                if (is_collision) {
                    self_position.y -= y_sgn; // undo move
                    break;
                }
            }
        }

        // const auto end = std::chrono::steady_clock::now();
        // const auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        // const auto percent_of_frame{ (static_cast<double>(total_ns) / (SDL_NS_PER_SECOND / 60.0)) * 100.0 };
        // printf("physics_time: %.2f%% of 60 FPS frame\n", percent_of_frame);
    }
}