#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include "player.hpp"
#include "engine/input.hpp"
#include "physics.hpp"

namespace clayborne {
    player init_player(entt::registry &registry) {
        player player{ 
            .entity = registry.create()
        };

        registry.emplace<position>(player.entity, 0.0f, 0.0f);
        registry.emplace<velocity>(player.entity, 0.0f, 0.0f);
        registry.emplace<collider>(player.entity, 32.0f, 32.0f);

        return player;
    }

    void deinit_player(player &player, entt::registry &registry) {
        registry.destroy(player.entity);
    }

    void update_player(player &player, entt::registry &registry, const input::gameplay_context &inputs) {
        using gameplay = input::gameplay_context;

        bool left{ gameplay::is_move_left(inputs) };
        bool right{ gameplay::is_move_right(inputs) };
        auto &vel{ registry.get<velocity>(player.entity) };
        vel.y = 4.0f * (right - left);

        for (auto event : inputs.events) {
            // Handle jump and use_head events
            (void)event;
        }
    }
}