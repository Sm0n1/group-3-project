#include <optional>
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include "camera.hpp"
#include "physics.hpp"
#include <print>

namespace clayborne {
    // TODO: match destination rectangle with window
    std::optional<clayborne::camera> init_camera(entt::registry &registry, SDL_Renderer *renderer) {
        constexpr int width = 320;
        constexpr int height = 180;

        clayborne::camera camera;

        // Initialize camera entity
        camera.entity = registry.create();
        registry.emplace<clayborne::position>(camera.entity, 0.0f, 0.0f);

        // Initialize canvas
        camera.canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, width, height);
        if (!camera.canvas) {
            SDL_Log("SDL create texture failed: %s", SDL_GetError());
            return std::nullopt;
        }

        // Initialize source rectangle
        camera.srcrect = SDL_FRect{
            .x = 0.0f,
            .y = 0.0f,
            .w = static_cast<float>(width),
            .h = static_cast<float>(height),
        };

        // Initialize destination rectangle
        camera.dstrect = SDL_FRect{
            .x = 0.0f,
            .y = 0.0f,
            .w = 640.0f,
            .h = 360.0f,
        };

        return camera;
    }

    void deinit_camera(clayborne::camera &camera, entt::registry &registry) {
        SDL_DestroyTexture(camera.canvas);
        registry.destroy(camera.entity);
    }

    void render(const clayborne::camera &camera, const entt::registry &registry, SDL_Renderer *renderer) {
        // Clear last frame
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, camera.canvas);
        SDL_RenderClear(renderer);

        // Draw camera view
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
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
        }

        // Render camera view
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderTexture(renderer, camera.canvas, &camera.srcrect, &camera.dstrect);
        SDL_RenderPresent(renderer);
    }
}