#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <entt/entt.hpp>
#include "camera.hpp"
#include "physics.hpp"
#include "resources.hpp"
#include <print>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <cstdio>
#include <entt/entt.hpp>
#include "camera.hpp"
#include "physics.hpp"

namespace clayborne {
    // TODO: match destination rectangle with window
    clayborne::camera init_camera(entt::registry &registry) {
        clayborne::camera camera{ 
            .entity = registry.create()
        };

        registry.emplace<clayborne::position>(camera.entity, 0.0f, 0.0f);

        return camera;
    }

    void deinit_camera(clayborne::camera &camera, entt::registry &registry) {
        registry.destroy(camera.entity);
    }

    void render(const clayborne::camera& camera, const entt::registry& registry, SDL_Renderer* renderer, SDL_Texture* canvas) {
        // Clear last frame
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, canvas);
        SDL_RenderClear(renderer);

        // Draw camera view
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        auto &camera_position = registry.get<const clayborne::position>(camera.entity);
        auto view = registry.view<const clayborne::position, const clayborne::renderer>();
        for (auto [entity, position, renderable]: view.each()) {
            const SDL_FRect dstrect{
                .x = renderable.dstrect.x + position.x - camera_position.x,
                .y = renderable.dstrect.y + position.y - camera_position.y,
                .w = renderable.dstrect.w,
                .h = renderable.dstrect.h,
            };
            if (renderable.texture) {
                // Debug stuff for showing drawbox
                //SDL_SetRenderDrawColor(renderer, 255, 155, 255, 255);
                //SDL_RenderRect(renderer, &dstrect);
                //SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

                // Render the sprite itself
                SDL_RenderTexture(renderer, renderable.texture, &renderable.srcrect, &dstrect);
            }
            else {
                SDL_RenderFillRect(renderer, &dstrect);
            }
        }

        // Render camera view
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderTexture(renderer, canvas, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
}