#include <SDL3_image/SDL_image.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include "level_loader.hpp"
#include "physics.hpp"
#include "clay.hpp"
#include "player.hpp"
#include "head.hpp"
#include "interactables.hpp"

namespace clayborne {
    std::vector<tile_group> merge_tiles_greedy(
        const std::array<std::array<std::uint8_t, tile_cols>, tile_rows> &tiles
    ) {
        auto tile_groups{ std::vector<tile_group>{} };
        auto visited_tiles{ std::array<std::array<bool, tile_cols>, tile_rows>{} };

        tile_groups.reserve(tile_rows * tile_cols);

        for (std::uint8_t y{ 0 }; y < tile_rows; y += 1) {
            for (std::uint8_t x{ 0 }; x < tile_cols; x += 1) {
                if (visited_tiles[y][x]) {
                    continue;
                }

                std::uint8_t tile{ tiles[y][x] };

                if (tile == 0) {
                    visited_tiles[y][x] = true;
                    continue;
                }

                std::uint8_t w{ 1 };
                while (x + w < tile_cols && tile == tiles[y][x + w] /* && !visited_tiles[y][x + w] */) {
                    w += 1;
                }

                std::uint8_t h{ 1 };
                bool can_expand{ true };
                while (y + h < tile_rows && can_expand) {
                    for (std::uint8_t dx{ 0 }; dx < w; dx += 1) {
                        if (tile != tiles[y + h][x + dx] || visited_tiles[y + h][x + dx]) {
                            can_expand = false;
                            break;
                        }
                    }

                    if (can_expand) {
                        h += 1;
                    }
                }

                for (std::uint8_t dy = 0; dy < h; dy += 1) {
                    for (std::uint8_t dx = 0; dx < w; dx += 1) {
                        visited_tiles[y + dy][x + dx] = true;
                    }
                }

                tile_groups.push_back({ tile, x, y, w, h });
            }
        }

        return tile_groups;
    }

    std::expected<std::monostate, std::string> load_level(
        const std::filesystem::path& level,
        entt::registry &registry,
        texture_cache &textures,
        SDL_Renderer *renderer
    ) {
        const auto data_path{ level / "data.json" };
        const auto grid_path{ level / "IntGrid.csv" };
        const auto foreground_path{ level / "IntGrid.png" };
        const auto background_path{ level / "_bg.png" };
        
        std::ifstream data_file{ data_path };
        if (!data_file) {
            return std::unexpected("Failed to open " + data_path.string());
        }

        auto data = nlohmann::json::parse(data_file, nullptr, false);
        if (data.is_discarded()) {
            return std::unexpected("Failed to parse " + data_path.string());
        }

        std::ifstream grid_file{ grid_path };
        if (!grid_file) {
            return std::unexpected("Failed to open " + grid_path.string());
        }

        std::array<std::array<std::uint8_t, tile_cols>, tile_rows> tiles{};
        std::string line;

        for (size_t row{ 0 }; row < tile_rows; row += 1) {
            if (!std::getline(grid_file, line)) {
                return std::unexpected("Failed to parse " + grid_path.string() + ": not enough rows");
            }

            const char *start{ line.data() };
            const char *end{ start + line.length() };

            for (size_t col{ 0 }; col < tile_cols; col += 1) {
                if (start >= end) {
                    return std::unexpected("Failed to parse " + grid_path.string() + ": not enough cols");
                }

                const char *comma{ start };
                while (comma < end && *comma != ',') {
                    comma += 1;
                }

                while (start < comma && *start == ' ') {
                    start += 1;
                }
                while (comma > start && *(comma - 1) == ' ') {
                    comma -= 1;
                }

                // std::uint8_t tile{ 0 };
                // auto [ptr, ec] = std::from_chars(start, comma, tile);
                //
                // if (ec != std::errc{}) {
                //     return std::unexpected("Failed to parse " + grid_path.string() + ": invalid integer");
                // }
                std::uint8_t tile{ 0 };
                std::string temp_s(start, comma);
                char* temp_end = nullptr;
                unsigned long value = strtoul(temp_s.c_str(), &temp_end, 10);
                if (temp_end != temp_s.c_str() + temp_s.size() || value > 255) {
                    return std::unexpected("Failed to parse " + grid_path.string() + ": invalid integer");
                }
                tile = static_cast<std::uint8_t>(value);

                tiles[row][col] = tile;
                start = (comma < end) ? comma + 1 : comma;
            }
        }

        constexpr int grass_tile{ 1 };
        constexpr int clay_tile{ 2 };
        constexpr int lava_tile{ 3 };

        const float level_x{ data["x"] };
        const float level_y{ data["y"] };

        auto tile_groups{ merge_tiles_greedy(tiles) };
        for (auto tg : tile_groups) {
            switch (tg.tile) {
            case grass_tile: {
                auto tile{ registry.create() };
                registry.emplace<position>(tile, level_x + tg.x * 8.0f, level_y + tg.y * 8.0f);
                registry.emplace<collider>(tile, tg.w * 8.0f, tg.h * 8.0f);
                registry.emplace<light_blocker>(tile);
                break;
            }
            case clay_tile: {
                auto tile{ registry.create() };
                registry.emplace<position>(tile, level_x + tg.x * 8.0f, level_y + tg.y * 8.0f);
                registry.emplace<collider>(tile, tg.w * 8.0f, tg.h * 8.0f);
                registry.emplace<clay>(tile);
                registry.emplace<light_blocker>(tile);
                break;
            }
            case lava_tile: {
                auto tile{ registry.create() };
                registry.emplace<position>(tile, level_x + tg.x * 8.0f, level_y + tg.y * 8.0f);
                registry.emplace<collider>(tile, tg.w * 8.0f, tg.h * 8.0f, [](entt::registry &r, const collider::collision &c) {
                    auto player{ r.try_get<struct player>(c.other) };
                    if (player) {
                        player->state = player::state::dead;
                        return;
                    }
                    auto head{ r.try_get<struct head>(c.other) };
                    if (head) {
                        head->state = head::state::detonated;
                        return;
                    }
                });
                break;
            }
            default:
                return std::unexpected("Failed to load tile " + std::to_string(tg.tile) + ": invalid tile id");
            }
        }

        const auto entities = data["entities"];

        for (auto& [entity_name, entity_list] : entities.items()) {
            if (entity_name == "Player") {
                float x{ entity_list[0]["x"] };
                float y{ entity_list[0]["y"] };
                init_player(registry, level_x + x, level_y + y);
            }
            else if (entity_name == "Door") {
                for (auto entity : entity_list) {
                    float x{ entity["x"] };
                    float y{ entity["y"] };
                    float w{ entity["width"] };
                    float h{ entity["height"] };
                    (void)create_door(registry, textures, renderer, level_x + x, level_y + y, w, h, false, entt::null);
                }
            }
            else if (entity_name == "Sensor") {
                for (auto entity : entity_list) {
                    float x{ entity["x"] };
                    float y{ entity["y"] };
                    float w{ entity["width"] };
                    float h{ entity["height"] };
                    (void)create_sensor(registry, textures, renderer, level_x + x, level_y + y, w, h);
                }
            }
            else {
                return std::unexpected("Failed to load entity " + entity_name + ": invalid entity id");
            }
        }

        const entt::hashed_string foreground_hs{ foreground_path.c_str() };
        const auto foreground_texture{
            textures.load(
                foreground_hs,
                foreground_path.c_str(),
                renderer
            )
        };
        if (!foreground_texture.first->second) {
            return std::unexpected("IMG load texture failed: " + std::string(SDL_GetError()));
        }

        auto foreground_entity{ registry.create() };
        auto &foreground_sprite_position{
            registry.emplace<position>(foreground_entity)
        };
        foreground_sprite_position.x = level_x;
        foreground_sprite_position.y = level_y;
        auto &foreground_sprite_renderer{
            registry.emplace<struct sprite_renderer>(foreground_entity)
        };
        foreground_sprite_renderer.texture = foreground_hs;
        foreground_sprite_renderer.srcrect = SDL_FRect{ 0.0f, 0.0f, 320.0f, 184.0f };
        foreground_sprite_renderer.z = 2;

        const entt::hashed_string background_hs{ background_path.c_str() };
        const auto background_texture{
            textures.load(
                background_hs,
                background_path.c_str(),
                renderer
            )
        };
        if (!background_texture.first->second) {
            return std::unexpected("IMG load texture failed: " + std::string(SDL_GetError()));
        }

        auto background_entity{ registry.create() };
        auto &background_sprite_position{
            registry.emplace<position>(background_entity)
        };
        background_sprite_position.x = level_x;
        background_sprite_position.y = level_y;
        auto &background_sprite_renderer{
            registry.emplace<struct sprite_renderer>(background_entity)
        };
        background_sprite_renderer.texture = background_hs;
        background_sprite_renderer.srcrect = SDL_FRect{ 0.0f, 0.0f, 320.0f, 184.0f };
        background_sprite_renderer.z = -1;

        return {};
    }

    std::expected<std::monostate, std::string> load_levels(
        const std::filesystem::path& levels,
        entt::registry &registry,
        texture_cache &textures,
        SDL_Renderer *renderer
    ) {
        for (auto level : std::filesystem::directory_iterator(levels)) {
            if (!std::filesystem::is_directory(level)) {
                continue;
            }

            if (!level.path().filename().string().starts_with("Level_")) {
                continue;
            }

            SDL_Log("Load level...(%s)", level.path().string().c_str());

            const auto result{ load_level(level.path(), registry, textures, renderer) };
            if (!result) {
                return result;
            }
        }

        return {};
    }
}