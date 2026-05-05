#ifndef CLAYBORNE_LEVEL_LOADER_HPP
#define CLAYBORNE_LEVEL_LOADER_HPP

#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include <filesystem>
#include <expected>
#include "sprite.hpp"

namespace clayborne {
    struct tile_group {
        std::uint8_t tile;
        std::uint8_t x;
        std::uint8_t y;
        std::uint8_t w;
        std::uint8_t h;
    };

    constexpr int tile_rows{ 23 };
    constexpr int tile_cols{ 40 };

    std::vector<tile_group> merge_tiles_greedy(
        const std::array<std::array<std::uint8_t, tile_cols>, tile_rows> &tiles
    );

    std::expected<std::monostate, std::string> load_level(
        const std::filesystem::path &level,
        entt::registry &registry,
        texture_cache &textures,
        SDL_Renderer *renderer
    );

    std::expected<std::monostate, std::string> load_levels(
        const std::filesystem::path &levels,
        entt::registry &registry,
        texture_cache &textures,
        SDL_Renderer *renderer
    );
}

#endif // CLAYBORNE_LEVEL_LOADER_HPP