#include <SDL3_image/SDL_image.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include "level_loader.hpp"
#include "camera.hpp"
#include "physics.hpp"
#include "clay.hpp"
#include "player.hpp"
#include "resources.hpp"
#include "interactables.hpp"

namespace clayborne {
    std::vector<tile_group> merge_tiles_greedy(const std::array<std::array<std::uint8_t, tile_cols>, tile_rows> &tiles) {
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
                    for (int dx{ 0 }; dx < w; dx += 1) {
                        if (tile != tiles[y + h][x + dx] || visited_tiles[y + h][x + dx]) {
                            can_expand = false;
                            break;
                        }
                    }

                    if (can_expand) {
                        h += 1;
                    }
                }

                for (int dy = 0; dy < h; dy += 1) {
                    for (int dx = 0; dx < w; dx += 1) {
                        visited_tiles[y + dy][x + dx] = true;
                    }
                }

                tile_groups.push_back({ tile, x, y, w, h });
            }
        }

        return tile_groups;
    }

    std::expected<std::monostate, std::string> load_level(const std::filesystem::path& level, entt::registry &registry, SDL_Renderer *renderer) {
        const auto data_path{ level / "data.json" };
        const auto grid_path{ level / "IntGrid.csv" };
        const auto image_path{ level / "_composite.png" };
        
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

        const int level_x{ data["x"] };
        const int level_y{ data["y"] };

        auto tile_groups{ merge_tiles_greedy(tiles) };
        for (auto tg : tile_groups) {
            switch (tg.tile) {
            case grass_tile: {
                auto tile{ registry.create() };
                registry.emplace<position>(tile, level_x + tg.x * 8.0f, level_y + tg.y * 8.0f);
                registry.emplace<collider>(tile, tg.w * 8.0f, tg.h * 8.0f);
                break;
            }
            case clay_tile: {
                auto tile{ registry.create() };
                registry.emplace<position>(tile, level_x + tg.x * 8.0f, level_y + tg.y * 8.0f);
                registry.emplace<collider>(tile, tg.w * 8.0f, tg.h * 8.0f);
                registry.emplace<clay>(tile);
                break;
            }
            case lava_tile: {
                auto tile{ registry.create() };
                registry.emplace<position>(tile, level_x + tg.x * 8.0f, level_y + tg.y * 8.0f);
                registry.emplace<collider>(tile, tg.w * 8.0f, tg.h * 8.0f, [](entt::registry &r, const collider::collision &c) {
                    auto p{ r.try_get<player>(c.other) };
                    if (p) {
                        p->state = player::state::dead;
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
                int x{ entity_list[0]["x"] };
                int y{ entity_list[0]["y"] };
                // TODO: use an actual resource loader
                auto r{ resources{ .spritesheet = nullptr } };
                init_player(registry, r, static_cast<float>(x + level_x), static_cast<float>(y +  level_y));
            }
            else if (entity_name == "Door") {
                int x{ entity_list[0]["x"] };
                int y{ entity_list[0]["y"] };
                (void)create_door(registry, static_cast<float>(x + level_x), static_cast<float>(y + level_y), false, entt::null);
            }
            else if (entity_name == "Sensor") {
                int x{ entity_list[0]["x"] };
                int y{ entity_list[0]["y"] };
                (void)create_sensor(registry, static_cast<float>(x + level_x), static_cast<float>(y + level_y));
            }
            else {
                return std::unexpected("Failed to load entity " + entity_name + ": invalid entity id");
            }
        }

        const auto level_sprite{ IMG_LoadTexture(renderer, image_path.string().c_str()) };
        if (!level_sprite) {
            return std::unexpected("IMG load texture failed: " + std::string(SDL_GetError()));
        }

        auto level_entity{ registry.create() };
        registry.emplace<position>(level_entity, static_cast<float>(level_x), static_cast<float>(level_y));
        registry.emplace<clayborne::renderer>(level_entity, level_sprite, SDL_FRect{ 0.0f, 0.0f, 320.0f, 184.0f }, SDL_FRect{ 0.0f, 0.0f, 320.0f, 184.0f }, -1);

        return {};
    }

    std::expected<std::monostate, std::string> load_levels(const std::filesystem::path& levels, entt::registry &registry, SDL_Renderer *renderer) {
        for (auto level : std::filesystem::directory_iterator(levels)) {
            if (!std::filesystem::is_directory(level)) {
                continue;
            }

            if (!level.path().filename().string().starts_with("Level_")) {
                continue;
            }

            SDL_Log("Load level...(%s)", level.path().string().c_str());

            const auto result{ load_level(level.path(), registry, renderer) };
            if (!result) {
                return result;
            }
        }

        return {};
    }
}