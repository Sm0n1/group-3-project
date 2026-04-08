#include <optional>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <entt/entt.hpp>
#include "camera.hpp"
#include "physics.hpp"
#include "resources.hpp"
#include <print>

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

    void render(const clayborne::camera& camera, const entt::registry& registry, const clayborne::resources& resources, SDL_Renderer* renderer, SDL_Texture* canvas) {
        // Clear last frame
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, canvas);
        SDL_RenderClear(renderer);

        // Load image TODO: Move into seperate entity, don't reload every frame lol
        auto texture = resources.spritesheet;

        // Draw camera view
        SDL_SetRenderDrawColor(renderer, 255, 55, 255, 255);
        auto &cpos = registry.get<const clayborne::position>(camera.entity);
        auto view = registry.view<const clayborne::position, const clayborne::collider>();
        for (auto [entity, pos, col]: view.each()) {
            const SDL_FRect rect{
                .x = pos.x - cpos.x, // camera relative
                .y = pos.y - cpos.y, // camera relative
                .w = col.w,
                .h = col.h
            };
            SDL_RenderFillRect(renderer, &rect);
            SDL_RenderTexture(renderer, texture, nullptr, &rect);
        }

        // Render camera view
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderTexture(renderer, canvas, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
}