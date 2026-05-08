#ifndef CLAYBORNE_RENDERING_HPP
#define CLAYBORNE_RENDERING_HPP

#include <entt/entt.hpp>
#include <SDL3/SDL.h>
#include "sprite.hpp"

namespace clayborne {
    constexpr int canvas_width{ 320 };
    constexpr int canvas_height{ 180 };

    struct sdl_circle {
        double radius{ 1.0 };
        std::vector<SDL_Vertex> vertices{};
        std::vector<int> indices{};
    };

    void sdl_circle_set_position(
        sdl_circle &circle,
        const double center_x,
        const double center_y
    );

    constexpr sdl_circle sdl_circle_init(
        const std::size_t vertex_count,
        const double radius,
        const double center_x,
        const double center_y,
        const SDL_FColor center_color,
        const SDL_FColor perimiter_color
    );

    SDL_Texture *init_vignette(
        SDL_Renderer *renderer
    );

    void render(
        const entt::entity camera,
        const entt::registry &registry,
        texture_cache &textures,
        SDL_Renderer *renderer,
        SDL_Texture *canvas,
        SDL_Texture *vignette
    );
}

#endif // CLAYBORNE_RENDERING_HPP