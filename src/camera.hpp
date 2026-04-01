#ifndef CLAYBORNE_CAMERA_HPP
#define CLAYBORNE_CAMERA_HPP

#include <expected>
#include <SDL3/SDL.h>
#include <entt/entt.hpp>

namespace clayborne {
    struct camera {
        entt::entity entity; 
        SDL_Texture *canvas;
        SDL_FRect srcrect;
        SDL_FRect dstrect;
    };

    std::optional<clayborne::camera> init_camera(entt::registry &registry, SDL_Renderer *renderer);
    void deinit_camera(clayborne::camera &camera, entt::registry &registry);
    void render(const clayborne::camera &camera, const entt::registry &registry, SDL_Renderer *renderer);
}

#endif // CLAYBORNE_CAMERA_HPP