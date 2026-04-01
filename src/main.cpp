#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include <print>

namespace clayborne {
    struct gamestate {
        SDL_Window *window{ nullptr };
        SDL_Renderer *renderer{ nullptr };
        Uint64 current_time;
        Uint64 accumulated_time{ 0 };
        entt::registry registry;

        // This  should be replaced by an actual input mapper
        bool is_w_down{ false };
        bool is_a_down{ false };
        bool is_s_down{ false };
        bool is_d_down{ false };
    };

    struct player {};

    struct position {
        float x;
        float y;
    };

    struct velocity {
        float x;
        float y;
    };

    struct collider {
        float w;
        float h;
    };

    void init_game(gamestate &gs) {
        const auto entity{ gs.registry.create() };
        gs.registry.emplace<player>(entity);
        gs.registry.emplace<position>(entity, 256.0f, 256.0f);
        gs.registry.emplace<velocity>(entity, 0.0f, 0.0f);
        gs.registry.emplace<collider>(entity, 32.0f, 32.0f);
    }

    void apply_forces(gamestate &gs) {
        auto view = gs.registry.view<const player, velocity>();

        for (auto [entity, vel]: view.each()) {
            vel.x = 4.0f * (gs.is_d_down - gs.is_a_down);
            vel.y = 4.0f * (gs.is_s_down - gs.is_w_down);
        }
    }

    void move(gamestate &gs) {
        auto view = gs.registry.view<position, const velocity>();

        for (auto [entity, pos, vel]: view.each()) {
            pos.x += vel.x;
            pos.y += vel.y;
        }
    }

    void update_game(gamestate &gs) {
        apply_forces(gs);
        move(gs);
    }

    void render_game(gamestate &gs) {
        auto view = gs.registry.view<const position, const collider>();

        SDL_SetRenderDrawColor(gs.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gs.renderer);

        SDL_SetRenderDrawColor(gs.renderer, 255, 255, 255, 255);
        for (auto [entity, pos, col]: view.each()) {
            const SDL_FRect rect{ .x = pos.x, .y = pos.y, .w = col.w, .h = col.h };
            SDL_RenderFillRect(gs.renderer, &rect);
        }

        SDL_RenderPresent(gs.renderer);
    }

    void deinit_game(gamestate &gs) {
        (void)gs;
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    auto gamestate{ new clayborne::gamestate{ .current_time = SDL_GetTicksNS() } };
    *appstate = gamestate;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    gamestate->window = SDL_CreateWindow("Clayborne", 512, 512, 0);
    if (!gamestate->window) {
        SDL_Log("SDL create window failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    gamestate->renderer = SDL_CreateRenderer(gamestate->window, nullptr);
    if (!gamestate->renderer) {
        SDL_Log("SDL create renderer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    clayborne::init_game(*gamestate);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    auto gamestate{ static_cast<clayborne::gamestate*>(appstate) };

    switch (event->type) {
    case SDL_EVENT_QUIT: [[fallthrough]];
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        switch (event->key.scancode) {
        case SDL_SCANCODE_W: gamestate->is_w_down = true; break;
        case SDL_SCANCODE_A: gamestate->is_a_down = true; break;
        case SDL_SCANCODE_S: gamestate->is_s_down = true; break;
        case SDL_SCANCODE_D: gamestate->is_d_down = true; break;
        }
        break;
    case SDL_EVENT_KEY_UP:
        switch (event->key.scancode) {
        case SDL_SCANCODE_W: gamestate->is_w_down = false; break;
        case SDL_SCANCODE_A: gamestate->is_a_down = false; break;
        case SDL_SCANCODE_S: gamestate->is_s_down = false; break;
        case SDL_SCANCODE_D: gamestate->is_d_down = false; break;
        }
        break;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto gamestate{ static_cast<clayborne::gamestate*>(appstate) };

    Uint64 frame_time = SDL_GetTicksNS() - gamestate->current_time;
    gamestate->current_time += frame_time;
    gamestate->accumulated_time += frame_time;

    while (gamestate->accumulated_time >= SDL_NS_PER_SECOND / 60) {
        clayborne::update_game(*gamestate);
        gamestate->accumulated_time -= SDL_NS_PER_SECOND / 60;
    }

    clayborne::render_game(*gamestate);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    auto gamestate{ static_cast<clayborne::gamestate*>(appstate) };

    (void)result;

    clayborne::deinit_game(*gamestate);

    if (gamestate->renderer) {
        SDL_DestroyRenderer(gamestate->renderer);
    }

    if (gamestate->window) {
        SDL_DestroyWindow(gamestate->window);
    }

    delete gamestate;
}