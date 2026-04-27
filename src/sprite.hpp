#ifndef CLAYBORNE_SPRITE_HPP
#define CLAYBORNE_SPRITE_HPP

#include <SDL3/SDL.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace clayborne {
    struct animation_resource { std::vector<SDL_FRect> frames; };
    using texture_resource = SDL_Texture;

    struct animation_loader {
        using result_type = std::shared_ptr<animation_resource>;
        result_type operator()(const std::filesystem::path &path) noexcept;
    };
    struct texture_loader {
        using result_type = std::shared_ptr<texture_resource>;
        result_type operator()(const std::filesystem::path &path, SDL_Renderer *renderer) noexcept;
    };

    using animation_cache = entt::resource_cache<animation_resource, animation_loader>;
    using texture_cache = entt::resource_cache<texture_resource, texture_loader>;

    struct sprite_animator {
        entt::id_type animation;
        std::size_t current_frame;
        bool is_looping;
    };
    struct sprite_renderer {
        entt::id_type texture{};
        SDL_FRect srcrect{};
        float x_offset{ 0.0f };
        float y_offset{ 0.0f };
        int z{ 0 };
    };

    void animate_sprites(
        entt::registry &registry,
        animation_cache &animations
    );
}

#endif // CLAYBORNE_SPRITE_HPP