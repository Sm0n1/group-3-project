#ifndef CLAYBORNE_ANIMATION_HPP
#define CLAYBORNE_ANIMATION_HPP

#include <SDL3/SDL.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <fstream>

// The animation resources are implemented as per the following:
// https://github.com/skypjack/entt/wiki/Resource-management

namespace clayborne {
    struct animation_resource {
        std::vector<SDL_FRect> frames;
    };

    struct animation_loader {
        using result_type = std::shared_ptr<animation_resource>;

        result_type operator()(const std::filesystem::path& path) {
            animation_resource resource{};

            std::ifstream file{ path };
            if (!file) {
                return nullptr;
            }

            auto data = nlohmann::json::parse(file, nullptr, false);
            if (data.is_discarded()) {
                return nullptr;
            }

            auto frames = data["frames"];
            for (auto frame : frames) {
                resource.frames.push_back({
                    .x = frame["frame"]["x"],
                    .y = frame["frame"]["y"],
                    .w = frame["frame"]["w"],
                    .h = frame["frame"]["h"],
                });

                SDL_Log("%d, %d, %d, %d",
                static_cast<int>(frame["frame"]["x"]),
                static_cast<int>(frame["frame"]["y"]),
                static_cast<int>(frame["frame"]["w"]),
                static_cast<int>(frame["frame"]["h"])
                );
            }

            return std::make_shared<animation_resource>(resource);
        }
    };

    using animation_cache = entt::resource_cache<animation_resource, animation_loader>;

    struct animator {
        entt::resource<animation_resource> resource;
        std::size_t current_frame;
        bool is_looping;
    };

    void animate(entt::registry &registry);
}

#endif // CLAYBORNE_ANIMATION_HPP