#include "SDL3/SDL_init.h"
#include "SDL3_mixer/SDL_mixer.h"
#include "audio.hpp"
#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <entt/entt.hpp>
#include <cstdio>
#include "engine/input/manager.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "physics.hpp"
#include "interactables.hpp"
#include "level_loader.hpp"
#include "sprite.hpp"
#include "rendering.hpp"
#include "sdl.hpp"

struct gamestate {
    SDL_Window *window{ nullptr };
    SDL_Renderer *renderer{ nullptr };
    SDL_Texture *canvas{ nullptr };
    MIX_Mixer *mixer{ nullptr };
    Uint64 current_time;
    Uint64 accumulated_time{ 0 };
    entt::registry registry;
    entt::entity player;
    entt::entity camera;
    bool is_fullscreen{ false };

    clayborne::texture_cache textures{};
    clayborne::animation_cache animations{};
    clayborne::audio_cache sounds{};

    clayborne::input::manager inputs;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
try {
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

    // Initialize SDL_mixer.
    if (!MIX_Init()) {
        SDL_Log("SDL_mixer init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Initialize audio mixer.
    gs.mixer = MIX_CreateMixerDevice(sdl_audio_device_default_playback, NULL);
    if (!gs.mixer) {
        SDL_Log("MIX create mixer with default device failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // TODO: remove
    if (!clayborne::load_debug_sounds(gs.sounds, gs.mixer)) {
        return SDL_APP_FAILURE;
    }

    // Initialize camera
    gs.camera = clayborne::init_camera(gs.registry);

    // Initialize levels
    auto level_load_result{
        clayborne::load_levels(
            "data/levels",
            gs.registry,
            gs.animations,
            gs.textures,
            gs.renderer
        )
    };
    if (!level_load_result) {
        SDL_Log("%s", level_load_result.error().c_str());
        return SDL_APP_FAILURE;
    }

    // Initialize player
    gs.player = gs.registry.view<clayborne::player>().front();
    if (gs.player == entt::null) {
        SDL_Log("Level contains no player");
        return SDL_APP_FAILURE;
    }
    
    // Initialize timer
    gs.current_time = SDL_GetTicksNS();

    // Initialize input manager
    gs.inputs = {};

    return SDL_APP_CONTINUE;

} catch (const std::exception& e) {
    SDL_Log("std::exception: %s", e.what());
    return SDL_APP_FAILURE;
} catch (const char* e) {
    SDL_Log("const char*: %s", e);
    return SDL_APP_FAILURE;
} catch (...) {
    SDL_Log("Unknown exception");
    return SDL_APP_FAILURE;
}
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    auto &gs{ *static_cast<gamestate*>(appstate) };
    auto &player{ gs.registry.get<clayborne::player>(gs.player) };

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
        clayborne::update_player(gs.player, gs.registry, gs.inputs, SDL_NS_PER_SECOND / 60, gs.sounds, gs.mixer);
        clayborne::update_physics(gs.registry, SDL_NS_PER_SECOND / 60);
        clayborne::sense(gs.registry);
        clayborne::toggle_doors(gs.registry);
        clayborne::update_camera(gs.camera, gs.player, gs.registry);
        clayborne::update_audio(gs.registry, gs.camera);
        clayborne::animate_sprites(gs.registry, gs.animations);
        gs.accumulated_time -= SDL_NS_PER_SECOND / 60;
    }

    // TODO: banish to the shadowrealm
    gs.registry.sort<clayborne::sprite_renderer>(
        [](
            const clayborne::sprite_renderer &lhs,
            const clayborne::sprite_renderer &rhs
        ) {
            return lhs.z < rhs.z;
        }
    );

    clayborne::render(gs.camera, gs.registry, gs.textures, gs.renderer, gs.canvas);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    auto &gs{ *static_cast<gamestate*>(appstate) };

    SDL_DestroyTexture(gs.canvas);
    SDL_DestroyRenderer(gs.renderer);
    SDL_DestroyWindow(gs.window);

    switch (result) {
    case SDL_APP_SUCCESS: SDL_Log("App Success"); break;
    case SDL_APP_FAILURE: SDL_Log("App Failure"); break;
    default:              std::unreachable();
    }
}