#ifndef CLAYBORNE_CAMERA_HPP
#define CLAYBORNE_CAMERA_HPP

#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <entt/entt.hpp>

namespace clayborne {
    struct camera {};
    entt::entity init_camera(entt::registry &registry);
    void update_camera(const entt::entity camera, const entt::entity player, entt::registry &registry);
}

#endif // CLAYBORNE_CAMERA_HPP