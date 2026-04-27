#include "interactables.hpp"
#include "physics.hpp"
#include "rendering.hpp"

namespace clayborne {
    void render(
        const entt::entity camera,
        const entt::registry &registry,
        texture_cache &textures,
        SDL_Renderer *renderer,
        SDL_Texture *canvas
    ) {
        // Clear last frame
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, canvas);
        SDL_RenderClear(renderer);

        // Draw camera view
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        auto &camera_position = registry.get<const struct position>(camera);
        auto view = registry.view<const struct position, const struct sprite_renderer>();
        for (auto [entity, position, sprite_renderer]: view.each()) {
            const SDL_FRect dstrect{
                .x = sprite_renderer.x_offset + position.x - camera_position.x,
                .y = sprite_renderer.y_offset + position.y - camera_position.y,
                .w = sprite_renderer.srcrect.w,
                .h = sprite_renderer.srcrect.h,
            };
            auto texture{
                textures[sprite_renderer.texture]
            };

            if (texture) {
                auto door = registry.try_get<const struct door>(entity);
                if (door && door->is_open) {
                    SDL_SetTextureAlphaMod(texture.handle().get(), 128);
                    SDL_RenderTexture(renderer, texture.handle().get(), &sprite_renderer.srcrect, &dstrect);
                    SDL_SetTextureAlphaMod(texture.handle().get(), 255);
                    continue;
                }
                SDL_RenderTexture(renderer, texture.handle().get(), &sprite_renderer.srcrect, &dstrect);
            }
        }

        // Render camera view
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderTexture(renderer, canvas, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
}