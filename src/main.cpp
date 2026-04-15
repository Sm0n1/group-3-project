#define SDL_MAIN_USE_CALLBACKS

// #include <utility>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <entt/entt.hpp>
#include <cstdio>
#include <fstream>
#include <string>
#include <print>
#include "engine/input/manager.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "physics.hpp"
#include "resources.hpp"
#include "clay.hpp"
#include "interactables.hpp"
#include "level_loader.hpp"

struct gamestate {
    SDL_Window *window{ nullptr };
    SDL_Renderer *renderer{ nullptr };
    SDL_Texture *canvas{ nullptr };
    Uint64 current_time;
    Uint64 accumulated_time{ 0 };
    entt::registry registry;
    entt::entity player;
    entt::entity camera;
    clayborne::resources resources;
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
    gs.window = SDL_CreateWindow("Clayborne", 1280, 720, SDL_WINDOW_RESIZABLE);
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

    // Initialize resources
    gs.resources = clayborne::init_resources(gs.renderer);

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

    // Initialize levels
    auto level_load_result{ clayborne::load_levels("data/levels", gs.registry, gs.renderer) };
    if (!level_load_result) {
        std::println("{}", level_load_result.error());
        return SDL_APP_FAILURE;
    }

    // Initialize player
    gs.player = gs.registry.view<clayborne::player>().front();
    if (gs.player == entt::null) {
        std::println("Level contains no player");
        return SDL_APP_FAILURE;
    }
    
    // Initialize timer
    gs.current_time = SDL_GetTicksNS();

    // Initialize input manager
    gs.inputs = {};

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    auto &gs{ *static_cast<gamestate*>(appstate) };
    auto &player{ gs.registry.get<clayborne::player>(gs.player) };
    auto &campos{ gs.registry.get<clayborne::position>(gs.camera) };

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
        case SDL_SCANCODE_K: player.head_just_pressed = true; player.head_pressed = true; break;
        case SDL_SCANCODE_W: player.up = true; break;
        case SDL_SCANCODE_A: player.left = true; break;
        case SDL_SCANCODE_S: player.down = true; break;
        case SDL_SCANCODE_D: player.right = true; break;
        case SDL_SCANCODE_UP: campos.y -= 184.0f; break;
        case SDL_SCANCODE_DOWN: campos.y += 184.0f; break;
        case SDL_SCANCODE_LEFT: campos.x -= 320.0f; break;
        case SDL_SCANCODE_RIGHT: campos.x += 320.0f; break;
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
        case SDL_SCANCODE_K: player.head_pressed = false; break; 
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
        clayborne::sense(gs.registry);
        clayborne::call(clayborne::toggle_doors, gs.registry);
        gs.accumulated_time -= SDL_NS_PER_SECOND / 60;
    }

    // TODO: banish to the shadowrealm
    gs.registry.sort<clayborne::renderer>([](const clayborne::renderer &lhs, const clayborne::renderer &rhs) {
        return lhs.z < rhs.z;
    });

    clayborne::render(gs.camera, gs.registry, gs.renderer, gs.canvas);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    auto &gs{ *static_cast<gamestate*>(appstate) };

    clayborne::deinit_camera(gs.camera, gs.registry);
    //clayborne::deinit_resources(gs.resources); //TODO implement

    SDL_DestroyTexture(gs.canvas);
    SDL_DestroyRenderer(gs.renderer);
    SDL_DestroyWindow(gs.window);

    switch (result) {
    case SDL_APP_SUCCESS: std::println("App Success"); break;
    case SDL_APP_FAILURE: std::println("App Failure"); break;
    case SDL_APP_CONTINUE: std::unreachable();
    default:
        break;
    }
}