#include "SDL3/SDL_events.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_surface.h>
#include <cstdio>
#include <fstream>
#include <string>
#define SDL_MAIN_USE_CALLBACKS

// #include <utility>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <entt/entt.hpp>
// #include <print>
#include "engine/input/manager.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "physics.hpp"

struct gamestate {
    SDL_Window *window{ nullptr };
    SDL_Renderer *renderer{ nullptr };
    SDL_Texture *canvas{ nullptr };
    Uint64 current_time;
    Uint64 accumulated_time{ 0 };
    entt::registry registry;
    entt::entity player;
    clayborne::camera camera;
    bool is_fullscreen{ false };

    clayborne::input::manager inputs;
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
    gs.window = SDL_CreateWindow("Clayborne", 640, 360, SDL_WINDOW_RESIZABLE);
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

    // Enable automatic scaling
    SDL_SetRenderLogicalPresentation(gs.renderer, 320, 180, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
    

    // Initialize canvas
    gs.canvas = SDL_CreateTexture(gs.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, 320, 180);
    if (!gs.canvas) {
        SDL_Log("SDL create texture failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Scale the canvas with sharp edges
    SDL_SetTextureScaleMode(gs.canvas, SDL_SCALEMODE_NEAREST);

    // Initialize camera
    gs.camera = clayborne::init_camera(gs.registry);

    // Initialize player
    gs.player = clayborne::init_player(gs.registry, 180.0f, 90.0f);

    // Initialize play area

    // Quick and dirty ldtk super simple level format reader
    // move this to its own file
    SDL_Texture * level_sprite = IMG_LoadTexture(gs.renderer, "../data/levels/sprite.png");
    auto level{ gs.registry.create() };
    gs.registry.emplace<clayborne::position>(level, 0.0f, 0.0f);
    gs.registry.emplace<clayborne::renderer>(level, level_sprite, SDL_FRect{ .x = 0.0f, .y = 0.0f, .w = 320.0f, .h = 180.0f }, SDL_FRect{ .x = 0.0f, .y = 0.0f, .w = 320.0f, .h = 180.0f });

    std::ifstream file("../data/levels/tiles.csv");

    std::string line;

    float x{0};
    float y{0};

    float tile_size{ 8.0f };

    while (getline (file, line, ',')) {
        if (line == "1") {
            auto tile{ gs.registry.create() };
            gs.registry.emplace<clayborne::position>(tile, x * tile_size, y * tile_size);
            gs.registry.emplace<clayborne::collider>(tile, tile_size, tile_size);
            //gs.registry.emplace<clayborne::renderer>(tile, nullptr, SDL_FRect{}, SDL_FRect{ .x = 0.0f, .y = 0.0f, .w = tile_size, .h = tile_size });
        }

        if (line == "0" ||  line == "1") {
            x = x + 1;
            if (x>=39) {
                x = 0;
                y = y + 1;
            }
        }
    }
    // end of level loader

    // Initialize timer
    gs.current_time = SDL_GetTicksNS();

    // Initialize input manager
    gs.inputs = {};

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    auto &gs{ *static_cast<gamestate*>(appstate) };
    clayborne::player &player{ gs.registry.get<clayborne::player>(gs.player) };

    switch (event->type) {
    case SDL_EVENT_QUIT: [[fallthrough]];
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        if (event->key.repeat) {
            break;
        }
        switch (event->key.scancode) {
        case SDL_SCANCODE_F11:
            gs.is_fullscreen = !gs.is_fullscreen;
            SDL_SetWindowFullscreen(gs.window, gs.is_fullscreen);
            break;
        // ------------------------ //
        // Temporary Input Handling //
        // ------------------------ //
        case SDL_SCANCODE_J: player.jump_just_pressed = true; player.jump_pressed = true; break;
        case SDL_SCANCODE_K: player.head_just_pressed = true; break;
        case SDL_SCANCODE_W: player.up = true; break;
        case SDL_SCANCODE_A: player.left = true; break;
        case SDL_SCANCODE_S: player.down = true; break;
        case SDL_SCANCODE_D: player.right = true; break;
        // ------------------------ //
        default:
            break;
        }
        break;
    case SDL_EVENT_KEY_UP:
        // ------------------------ //
        // Temporary Input Handling //
        // ------------------------ //
        switch (event->key.scancode) {
        case SDL_SCANCODE_J: player.jump_pressed = false; break;
        case SDL_SCANCODE_W: player.up = false; break;
        case SDL_SCANCODE_A: player.left = false; break;
        case SDL_SCANCODE_S: player.down = false; break;
        case SDL_SCANCODE_D: player.right = false; break;
        default: break;
        }
        break;
        // ------------------------ //
    // case SDL_EVENT_GAMEPAD_ADDED:
    // case SDL_EVENT_GAMEPAD_REMOVED:
    default:
        // gs.inputs.process_event(*event);
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
        clayborne::update_player(gs.player, gs.registry, gs.inputs, SDL_NS_PER_SECOND / 60);
        clayborne::update_physics(gs.registry, SDL_NS_PER_SECOND / 60);
        gs.accumulated_time -= SDL_NS_PER_SECOND / 60;
    }

    clayborne::render(gs.camera, gs.registry, gs.renderer, gs.canvas);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    auto &gs{ *static_cast<gamestate*>(appstate) };

    clayborne::deinit_camera(gs.camera, gs.registry);

    SDL_DestroyTexture(gs.canvas);
    SDL_DestroyRenderer(gs.renderer);
    SDL_DestroyWindow(gs.window);

    switch (result) {
    // case SDL_APP_SUCCESS: std::println("App Success"); break;
    // case SDL_APP_FAILURE: std::println("App Failure"); break;
    // case SDL_APP_CONTINUE: std::unreachable();
    default:
        break;
    }
}