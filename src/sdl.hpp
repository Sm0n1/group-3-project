#ifndef CLAYBORNE_SDL_HPP
#define CLAYBORNE_SDL_HPP

#include <SDL3/SDL.h>

// We do this to get around Clang's preamble optimization
// incorrectly forgetting pushes and thus treating pops
// as compilation errors. Yes, really...
static_assert(true);

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

constexpr auto sdl_audio_device_default_playback{
    SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK
};

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif // CLAYBORNE_SDL_HPP