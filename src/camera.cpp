#include <cstdio>
#include <entt/entt.hpp>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "camera.hpp"
#include "physics.hpp"

namespace clayborne {
    entt::entity init_camera(entt::registry &registry) {
        auto camera{ registry.create() };

        registry.emplace<struct camera>(camera);
        registry.emplace<struct position>(camera, 0.0f, 0.0f);

        return camera;
    }

    // Sets the camera position to the room the player is currently in.
    void update_camera(const entt::entity camera, const entt::entity player, entt::registry &registry) {
        auto &camera_position{ registry.get<position>(camera) };
        auto &player_position{ registry.get<const position>(player) };
        auto &player_collider{ registry.get<const collider>(player) };

        collider temporary_camera_collider{ .w = 320.0f, .h = 184.0f, .collide = nullptr };
        if (!overlap(camera_position, temporary_camera_collider, player_position, player_collider)) {
            camera_position.x = std::floor(player_position.x / 320.0f) * 320.0f;
            camera_position.y = std::floor(player_position.y / 184.0f) * 184.0f;
        }
    }
}