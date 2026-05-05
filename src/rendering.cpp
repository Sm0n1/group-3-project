#include "SDL3/SDL_render.h"
#include "interactables.hpp"
#include "physics.hpp"
#include "sprite.hpp"
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
        for (auto entity: view) {
            const auto &sprite_renderer{
                view.get<const struct sprite_renderer>(entity)
            };

            auto texture{ textures[sprite_renderer.texture] };
            if (!texture) {
                continue;
            }

            SDL_SetTextureAlphaMod(texture.handle().get(), sprite_renderer.alpha);

            const auto &position{
                view.get<const struct position>(entity)
            };

            if (sprite_renderer.is_tiled) {
                const SDL_FRect dstrect{
                    .x = sprite_renderer.x_offset + position.x - camera_position.x,
                    .y = sprite_renderer.y_offset + position.y - camera_position.y,
                    .w = sprite_renderer.w_tiled,
                    .h = sprite_renderer.h_tiled,
                };

                SDL_RenderTextureTiled(
                    renderer,
                    texture.handle().get(),
                    &sprite_renderer.srcrect,
                    1.0f,
                    &dstrect
                );
                continue;
            }

            const SDL_FRect dstrect{
                .x = sprite_renderer.x_offset + position.x - camera_position.x,
                .y = sprite_renderer.y_offset + position.y - camera_position.y,
                .w = sprite_renderer.srcrect.w,
                .h = sprite_renderer.srcrect.h,
            };

            SDL_RenderTextureRotated(
                renderer,
                texture.handle().get(),
                &sprite_renderer.srcrect,
                &dstrect,
                0.0,
                sprite_renderer.center ? &*sprite_renderer.center : nullptr,
                sprite_renderer.flip
            );
        }

        // Render camera view
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderTexture(renderer, canvas, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
}