#ifndef CLAYBORNE_CAMERA_HPP
#define CLAYBORNE_CAMERA_HPP

#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <entt/entt.hpp>

namespace clayborne {
    struct camera {
        entt::entity entity;
    };

    struct renderer {
        SDL_Texture *texture{ nullptr };
        SDL_FRect srcrect{};
        SDL_FRect dstrect{};
        int z{ 0 };
    };

    clayborne::camera init_camera(entt::registry &registry);
    void deinit_camera(clayborne::camera &camera, entt::registry &registry);
    void render(const clayborne::camera &camera, const entt::registry &registry, SDL_Renderer *renderer, SDL_Texture *canvas);
}

#endif // CLAYBORNE_CAMERA_HPP