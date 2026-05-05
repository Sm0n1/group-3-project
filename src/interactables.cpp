#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include "interactables.hpp"
#include "physics.hpp"

namespace clayborne {
    [[nodiscard]] entt::entity create_sensor(
        entt::registry &registry,
        texture_cache &textures,
        SDL_Renderer *renderer,
        const float x,
        const float y
    ) noexcept {
        auto entity{ registry.create() };

        registry.emplace<sensor>(entity, 8.0f, 8.0f, false);
        registry.emplace<position>(entity, x, y);
        auto &sr{ registry.emplace<sprite_renderer>(entity) };
        const entt::hashed_string hash{ "data/obejcts.png" };
        sr.texture = hash;
        sr.srcrect.x = 8.0f;
        sr.srcrect.w = 8.0f;
        sr.srcrect.h = 8.0f;
        if (!textures.load(hash, "data/objects.png", renderer).first->second) {
            SDL_Log("Could not load door texture");
            // TODO: error handling
        }

        return entity;
    }

    void sense(entt::registry &registry) noexcept {
        constexpr auto is_sensing{ [&](
            const position &sp,
            const sensor &s,
            const position &ap,
            const activator &a
        ) {
            return
                (sp.x + s.w > ap.x) &&
                (sp.y + s.h > ap.y) &&
                (ap.x + a.w > sp.x) &&
                (ap.y + a.h > sp.y);
        }};

        auto sensors{ registry.view<const position, sensor>() };
        auto activators{ registry.view<const position, const activator>() };

        for (auto [se, sp, s] : sensors.each()) {
            s.is_sensing = false;
            for (auto [ae, ap, a] : activators.each()) {
                if (is_sensing(sp, s, ap, a)) {
                    s.is_sensing = true;
                    break;
                }
            }
        }
    }

    [[nodiscard]] entt::entity create_door(
        entt::registry &registry,
        texture_cache &textures,
        SDL_Renderer *renderer,
        const float x,
        const float y,
        const bool is_default_open,
        const entt::entity toggle_sensor
    ) noexcept {
        auto entity{ registry.create() };

        registry.emplace<door>(entity, toggle_sensor, 8.0f, 16.0f, is_default_open, false);
        registry.emplace<position>(entity, x, y);
        auto &sr{ registry.emplace<sprite_renderer>(entity) };
        const entt::hashed_string hash{ "data/obejcts.png" };
        sr.texture = hash;
        sr.srcrect.w = 8.0f;
        sr.srcrect.h = 16.0f;
        if (!textures.load(hash, "data/objects.png", renderer).first->second) {
            SDL_Log("Could not load door texture");
            // TODO: error handling
        }

        if (!is_default_open) {
            registry.emplace<collider>(entity, 8.0f, 16.0f, std::nullopt);
        }
        
        return entity;
    }

    void toggle_doors(entt::registry &registry) noexcept {
        auto sensors{ registry.view<const sensor>() };
        auto doors{ registry.view<position, sprite_renderer, door>() };

        for (auto [de, dp, dsr, d] : doors.each()) {
            d.is_open = d.is_default_open;
            for (auto [se, s] : sensors.each()) {
                if ((d.sensor == entt::null || d.sensor == se) && s.is_sensing) {
                    d.is_open = !d.is_default_open;
                    break;
                }
            }

            if (d.is_open) {
                registry.remove<collider>(de);
                dsr.z = 0;
                continue;
            }

            // Only add the collider if the coast is clear!
            collider dc{ d.w, d.h, std::nullopt };
            if (!overlap_any(registry, de, dp, dc)) {
                registry.emplace_or_replace<collider>(de, dc);
                dsr.z = 1;
            }
        }
    }
};