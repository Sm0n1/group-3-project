#ifndef CLAYBORNE_INPUT_TRIGGER_HPP
#define CLAYBORNE_INPUT_TRIGGER_HPP

#include <SDL3/SDL.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace clayborne::input::trigger {
    struct id {
        static constexpr std::uint64_t invalid{ std::numeric_limits<std::uint64_t>::max() };

        std::uint64_t value = invalid;

        [[nodiscard]] constexpr bool valid() const noexcept {
            return value != invalid;
        }

        constexpr std::strong_ordering operator<=>(const id&) const noexcept = default;
    };

    enum class sdl_mouse_button : std::uint8_t  {
        left = SDL_BUTTON_LEFT,
        middle = SDL_BUTTON_MIDDLE,
        right = SDL_BUTTON_RIGHT,
        x1 = SDL_BUTTON_X1,
        x2 = SDL_BUTTON_X2,
    };

    enum class tag : std::uint8_t {
        key,
        mouse_button,
        mouse_motion,
        mouse_wheel,
        gamepad_button,
        gamepad_axis,
    };

    union data {
        SDL_Scancode key;
        sdl_mouse_button mouse_button;
        SDL_GamepadButton gamepad_button;
        SDL_GamepadAxis gamepad_axis;
    };

    struct descriptor {
        tag t;
        data d;

        static constexpr descriptor init_key(SDL_Scancode key) noexcept {
            return descriptor{
                .t = tag::key,
                .d{ .key = key }
            };
        }

        static constexpr descriptor init_mouse_button(trigger::sdl_mouse_button button) noexcept {
            return descriptor{
                .t = tag::mouse_button,
                .d{ .mouse_button = button }
            };
        }

        static constexpr descriptor init_mouse_motion() noexcept {
            return descriptor{
                .t = tag::mouse_motion,
                .d{}
            };
        }

        static constexpr descriptor init_mouse_wheel() noexcept {
            return descriptor{
                .t = tag::mouse_wheel,
                .d{}
            };
        }

        static constexpr descriptor init_gamepad_button(SDL_GamepadButton button) noexcept {
            return descriptor{
                .t = tag::gamepad_button,
                .d{ .gamepad_button = button }
            };
        }

        static constexpr descriptor init_gamepad_axis(SDL_GamepadAxis axis) noexcept {
            return descriptor{
                .t = tag::gamepad_axis,
                .d{ .gamepad_axis = axis }
            };
        }
    };

    inline constexpr std::size_t mouse_button_index(sdl_mouse_button button) noexcept {
        return static_cast<std::size_t>(button) - 1;
    }
}

#endif // CLAYBORNE_INPUT_TRIGGER_HPP