#ifndef CLAYBORNE_INPUT_MAP_HPP
#define CLAYBORNE_INPUT_MAP_HPP

#include <array>
#include <cassert>
#include <compare>
#include <limits>
#include <memory>
#include "binding.hpp"
#include "trigger.hpp"

namespace clayborne::input::map {
    struct id {
        static constexpr std::uint64_t invalid{ std::numeric_limits<std::uint64_t>::max() };

        std::uint64_t value = invalid;

        [[nodiscard]] constexpr bool valid() const noexcept {
            return value != invalid;
        }

        constexpr std::strong_ordering operator<=>(const id&) const noexcept = default;
    };

    struct map {
        std::array<std::unique_ptr<binding::key>, SDL_SCANCODE_COUNT> keyboard;
        std::array<std::unique_ptr<binding::mouse_button>, 5> mouse_buttons;
        std::unique_ptr<binding::mouse_motion> mouse_motion;
        std::unique_ptr<binding::mouse_wheel> mouse_wheel;
        std::array<std::unique_ptr<binding::gamepad_button>, SDL_GAMEPAD_BUTTON_COUNT> gamepad_buttons;
        std::array<std::unique_ptr<binding::gamepad_axis>, SDL_GAMEPAD_AXIS_COUNT> gamepad_axes;

        void bind_key(const SDL_Scancode scancode, std::unique_ptr<binding::key> binding);
        void bind_mouse_button(const input::trigger::sdl_mouse_button button, std::unique_ptr<binding::mouse_button> binding);
        void bind_mouse_motion(std::unique_ptr<binding::mouse_motion> binding);
        void bind_mouse_wheel(std::unique_ptr<binding::mouse_wheel> binding);
        void bind_gamepad_button(const SDL_GamepadButton button, std::unique_ptr<binding::gamepad_button> binding);
        void bind_gamepad_axis(const SDL_GamepadAxis axis, std::unique_ptr<binding::gamepad_axis> binding);
    };
}

#endif // CLAYBORNE_INPUT_MAP_HPP