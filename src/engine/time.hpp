#ifndef CLAYBORNE_TIME_HPP
#define CLAYBORNE_TIME_HPP

#include <SDL3/SDL.h>
#include <compare>

namespace clayborne {
    struct time {
        Uint64 nanoseconds;

        time(Uint64 ns) noexcept : nanoseconds(ns) {}

        [[nodiscard]] inline constexpr Uint64 seconds() const noexcept {
            return SDL_NS_TO_SECONDS(nanoseconds);
        }

        [[nodiscard]] inline constexpr float seconds_float() const noexcept {
            return static_cast<float>(SDL_NS_TO_SECONDS(static_cast<double>(nanoseconds)));
        }

        constexpr std::strong_ordering operator<=>(const time &) const noexcept = default;
    };
}

#endif // CLAYBORNE_TIME_HPP