#ifndef CLAYBORNE_CAMERA_HPP
#define CLAYBORNE_CAMERA_HPP

#include <expected>
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include "resources.hpp"

namespace clayborne {
    struct camera {
        entt::entity entity;
    };

    clayborne::camera init_camera(entt::registry &registry);
    void deinit_camera(clayborne::camera &camera, entt::registry &registry);
    void render(const clayborne::camera &camera, const entt::registry &registry, const clayborne::resources &resources, SDL_Renderer *renderer, SDL_Texture *canvas);
}

#endif // CLAYBORNE_CAMERA_HPP