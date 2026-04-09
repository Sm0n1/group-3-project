#ifndef CLAYBORNE_INPUT_BINDING_HPP
#define CLAYBORNE_INPUT_BINDING_HPP

#include <SDL3/SDL.h>
#include <optional>
#include "action.hpp"
#include "trigger.hpp"

namespace clayborne::input::binding {
    struct id {
        static constexpr std::uint64_t invalid{ std::numeric_limits<std::uint64_t>::max() };

        std::uint64_t value = invalid;

        [[nodiscard]] constexpr bool valid() const noexcept {
            return value != invalid;
        }

        constexpr std::strong_ordering operator<=>(const id&) const noexcept = default;
    };

    struct key {
        virtual ~key() = default;
        virtual SDL_Scancode scancode() const noexcept = 0;
        virtual action::id action() const noexcept = 0;
        virtual std::optional<action::event> apply(const SDL_KeyboardEvent& event) const = 0;
    };

    struct mouse_button {
        virtual ~mouse_button() = default;
        virtual input::trigger::sdl_mouse_button button() const noexcept = 0;
        virtual action::id action() const noexcept = 0;
        virtual std::optional<action::event> apply(const SDL_MouseButtonEvent& event) const = 0;
    };

    struct mouse_motion {
        virtual ~mouse_motion() = default;
        virtual action::id action() const noexcept = 0;
        virtual std::optional<action::event> apply(const SDL_MouseMotionEvent& event) const = 0;
    };

    struct mouse_wheel {
        virtual ~mouse_wheel() = default;
        virtual action::id action() const noexcept = 0;
        virtual std::optional<action::event> apply(const SDL_MouseWheelEvent& event) const = 0;
    };

    struct gamepad_button {
        virtual ~gamepad_button() = default;
        virtual SDL_GamepadButton button() const noexcept = 0;
        virtual action::id action() const noexcept = 0;
        virtual std::optional<action::event> apply(const SDL_GamepadButtonEvent& event) const = 0;
    };

    struct gamepad_axis {
        virtual ~gamepad_axis() = default;
        virtual SDL_GamepadAxis axis() const noexcept = 0;
        virtual action::id action() const noexcept = 0;
        virtual std::optional<action::event> apply(const SDL_GamepadAxisEvent& event) const = 0;
    };

    struct descriptor {
        trigger::descriptor trigger;
        action::descriptor action;
    };
}

#endif // CLAYBORNE_INPUT_BINDING_HPP