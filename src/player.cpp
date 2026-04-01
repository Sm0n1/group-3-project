#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include "player.hpp"
#include "physics.hpp"

namespace clayborne {
    clayborne::player init_player(entt::registry &registry) {
        clayborne::player player{ 
            .entity = registry.create()
        };

        registry.emplace<clayborne::position>(player.entity, 0.0f, 0.0f);
        registry.emplace<clayborne::velocity>(player.entity, 0.0f, 0.0f);
        registry.emplace<clayborne::collider>(player.entity, 32.0f, 32.0f);

        return player;
    }

    void deinit_player(clayborne::player &player, entt::registry &registry) {
        registry.destroy(player.entity);
    }

    void update_player(clayborne::player &player, entt::registry &registry) {
        auto &vel{ registry.get<velocity>(player.entity) };
        vel.x = 4.0f * (player.is_d_down - player.is_a_down);
        vel.y = 4.0f * (player.is_s_down - player.is_w_down);
    }
}