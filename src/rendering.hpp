#ifndef CLAYBORNE_RENDERING_HPP
#define CLAYBORNE_RENDERING_HPP

#include <entt/entt.hpp>
#include <SDL3/SDL.h>
#include "sprite.hpp"

namespace clayborne {
    void render(
        const entt::entity camera,
        const entt::registry &registry,
        texture_cache &textures,
        SDL_Renderer *renderer,
        SDL_Texture *canvas
    );
}

#endif // CLAYBORNE_RENDERING_HPP