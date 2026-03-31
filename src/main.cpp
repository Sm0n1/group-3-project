#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <print>

namespace clayborne
{
    struct gamestate
    {
        SDL_Window *window{ nullptr };
        SDL_Renderer *renderer{ nullptr };
        Uint64 current_time;
        Uint64 accumulated_time{ 0 };
        // entt::registry
    };
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
    auto gamestate{ new clayborne::gamestate{ .current_time = SDL_GetTicksNS() } };
    *appstate = gamestate;

    (void)argc;
    (void)argv;

    SDL_Init(SDL_INIT_VIDEO);

    gamestate->window = SDL_CreateWindow("Clayborne", 512, 512, 0);
    if (gamestate->window == nullptr)
    {
        SDL_Log("SDL create window failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    gamestate->renderer = SDL_CreateRenderer(gamestate->window, nullptr);
    if (gamestate->renderer == nullptr)
    {
        SDL_Log("SDL create renderer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // init_game(gamestate)

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    (void)appstate;

    switch (event->type)
    {
    case SDL_EVENT_QUIT: [[fallthrough]];
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    auto gamestate{ static_cast<clayborne::gamestate*>(appstate) };

    Uint64 frame_time = SDL_GetTicksNS() - gamestate->current_time;
    gamestate->current_time += frame_time;
    gamestate->accumulated_time += frame_time;

    while (gamestate->accumulated_time >= SDL_NS_PER_SECOND / 60)
    {
        // update_game(gamestate)

        gamestate->accumulated_time -= SDL_NS_PER_SECOND / 60;
    }

    // render_game(gamestate)

    std::println("frame_time: {0:10.3f}", static_cast<double>(frame_time) / (SDL_NS_PER_SECOND / 60));

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    auto gamestate{ static_cast<clayborne::gamestate*>(appstate) };

    (void)result;

    // deinit_game(gamestate)

    if (gamestate->renderer != nullptr)
    {
        SDL_DestroyRenderer(gamestate->renderer);
    }

    if (gamestate->window != nullptr)
    {
        SDL_DestroyWindow(gamestate->window);
    }
}