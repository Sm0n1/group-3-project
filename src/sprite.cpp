#include <SDL3/SDL_log.h>
#include <fstream>
#include <SDL3_image/SDL_image.h>
#include "interactables.hpp"
#include "sprite.hpp"

namespace clayborne {
    // Load animation
    animation_loader::result_type animation_loader::operator()(
        const std::filesystem::path &path
    ) noexcept {
        animation_resource resource{};

        std::ifstream file{ path };
        if (!file) {
            SDL_Log("Could not open file: %s", path.c_str());
            return nullptr;
        }

        auto data = nlohmann::json::parse(file, nullptr, false);
        if (data.is_discarded()) {
            SDL_Log("Could not parse file as json: %s", path.c_str());
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
        }

        return std::make_shared<animation_resource>(resource);
    }

    // Load texture
    texture_loader::result_type texture_loader::operator()(
        const std::filesystem::path &path,
        SDL_Renderer *renderer
    ) noexcept {
        auto texture{ IMG_LoadTexture(renderer, path.c_str()) };
        if (!texture) {
            SDL_Log("Could not load texture %s: %s", path.c_str(), SDL_GetError());
            return nullptr;
        }

        return std::shared_ptr<SDL_Texture>{
            texture,
            [](SDL_Texture *t) { SDL_DestroyTexture(t); }
        };
    }

    void animate_sprites(
        entt::registry &registry,
        animation_cache &animations
    ) {
        auto view{ registry.view<struct sprite_renderer, struct sprite_animator>() };
        for (auto [e, r, a] : view.each()) {
            auto animation{ animations[a.animation] };
            if(!animation || animation->frames.empty()) {
                continue;
            }

            const auto &frame{ animation->frames[a.current_frame] };

            r.srcrect.x = frame.x;
            r.srcrect.y = frame.y;
            r.srcrect.w = frame.w;
            r.srcrect.h = frame.h;

            a.current_frame += 1;

            if (a.current_frame >= animation->frames.size()) {
                if (a.is_looping) {
                    a.current_frame = 0;
                }
                else {
                    a.current_frame = animation->frames.size() - 1;
                }
            }
        }
    }
}