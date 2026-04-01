#ifndef CLAYBORNE_PLAYER_HPP
#define CLAYBORNE_PLAYER_HPP

#include <SDL3/SDL.h>
#include <entt/entt.hpp>

namespace clayborne {
    struct player {
        entt::entity entity{ entt::null };

        // This should be replaced by an actual input mapper
        bool is_w_down{ false };
        bool is_a_down{ false };
        bool is_s_down{ false };
        bool is_d_down{ false };
    };

    clayborne::player init_player(entt::registry &registry);
    void deinit_player(clayborne::player &player, entt::registry &registry);
    void update_player(clayborne::player &player, entt::registry &registry);
}

#endif // CLAYBORNE_PLAYER_HPP