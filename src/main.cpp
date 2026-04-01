#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include <print>
#include "camera.hpp"
#include "player.hpp"
#include "physics.hpp"

struct gamestate {
    SDL_Window *window{ nullptr };
    SDL_Renderer *renderer{ nullptr };
    Uint64 current_time;
    Uint64 accumulated_time{ 0 };
    entt::registry registry;
    clayborne::player player;
    clayborne::camera camera;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    (void)argc;
    (void)argv;

    static gamestate gs;
    *appstate = &gs;

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Initialize window
    gs.window = SDL_CreateWindow("Clayborne", 640, 360, 0);
    if (!gs.window) {
        SDL_Log("SDL create window failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Initialize renderer
    gs.renderer = SDL_CreateRenderer(gs.window, nullptr);
    if (!gs.renderer) {
        SDL_Log("SDL create renderer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Initialize camera
    if (auto camera_opt{ clayborne::init_camera(gs.registry, gs.renderer) }) {
        gs.camera = *camera_opt;
    } else {
        return SDL_APP_FAILURE;
    }

    // Initialize player
    gs.player = clayborne::init_player(gs.registry);

    // Initialize timer
    gs.current_time = SDL_GetTicksNS();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    auto &gs{ *static_cast<gamestate*>(appstate) };

    switch (event->type) {
    case SDL_EVENT_QUIT: [[fallthrough]];
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        switch (event->key.scancode) {
        case SDL_SCANCODE_W: gs.player.is_w_down = true; break;
        case SDL_SCANCODE_A: gs.player.is_a_down = true; break;
        case SDL_SCANCODE_S: gs.player.is_s_down = true; break;
        case SDL_SCANCODE_D: gs.player.is_d_down = true; break;
        }
        break;
    case SDL_EVENT_KEY_UP:
        switch (event->key.scancode) {
        case SDL_SCANCODE_W: gs.player.is_w_down = false; break;
        case SDL_SCANCODE_A: gs.player.is_a_down = false; break;
        case SDL_SCANCODE_S: gs.player.is_s_down = false; break;
        case SDL_SCANCODE_D: gs.player.is_d_down = false; break;
        }
        break;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto &gs{ *static_cast<gamestate*>(appstate) };

    Uint64 frame_time = SDL_GetTicksNS() - gs.current_time;
    gs.current_time += frame_time;
    gs.accumulated_time += frame_time;

    while (gs.accumulated_time >= SDL_NS_PER_SECOND / 60) {
        clayborne::update_player(gs.player, gs.registry);
        clayborne::update_physics(gs.registry);
        gs.accumulated_time -= SDL_NS_PER_SECOND / 60;
    }

    clayborne::render(gs.camera, gs.registry, gs.renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    auto &gs{ *static_cast<gamestate*>(appstate) };

    clayborne::deinit_player(gs.player, gs.registry);
    clayborne::deinit_camera(gs.camera, gs.registry);

    if (gs.renderer) {
        SDL_DestroyRenderer(gs.renderer);
    }

    if (gs.window) {
        SDL_DestroyWindow(gs.window);
    }

    switch (result) {
    case SDL_APP_SUCCESS: std::println("App Success"); break;
    case SDL_APP_FAILURE: std::println("App Failure"); break;
    }
}