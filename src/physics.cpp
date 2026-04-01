#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include "physics.hpp"

namespace clayborne {
    void update_physics(entt::registry &registry) {
        auto view = registry.view<clayborne::position, const clayborne::velocity>();

        for (auto [entity, pos, vel]: view.each()) {
            pos.x += vel.x;
            pos.y += vel.y;
        }
    }
}