#ifndef CLAYBORNE_CAMERA_HPP
#define CLAYBORNE_CAMERA_HPP

#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <entt/entt.hpp>

namespace clayborne {
    // A camera is both a view into the world and a microphone.
    struct camera {};
    entt::entity init_camera(entt::registry &registry);
    void update_camera(const entt::entity camera, const entt::entity player, entt::registry &registry);

    constexpr float mic_x(const float camera_x) noexcept { return camera_x + 160.0f; }
    constexpr float mic_y(const float camera_y) noexcept { return camera_y + 90.0f; }
    constexpr float mic_z() noexcept { return 0.0f; }
}

#endif // CLAYBORNE_CAMERA_HPP