#include "map.hpp"
#include "trigger.hpp"

namespace clayborne::input::map {
    void map::bind_key(const SDL_Scancode scancode, std::unique_ptr<binding::key> binding) {
        keyboard[static_cast<std::size_t>(scancode)] = std::move(binding);
    }

    void map::bind_mouse_button(const trigger::sdl_mouse_button button, std::unique_ptr<binding::mouse_button> binding) {
        mouse_buttons[trigger::mouse_button_index(button)] = std::move(binding);
    }

    void map::bind_mouse_motion(std::unique_ptr<binding::mouse_motion> binding) {
        mouse_motion = std::move(binding);
    }

    void map::bind_mouse_wheel(std::unique_ptr<binding::mouse_wheel> binding) {
        mouse_wheel = std::move(binding);
    }

    void map::bind_gamepad_button(const SDL_GamepadButton button, std::unique_ptr<binding::gamepad_button> binding) {
        gamepad_buttons[static_cast<std::size_t>(button)]  = std::move(binding);
    }

    void map::bind_gamepad_axis(const SDL_GamepadAxis axis, std::unique_ptr<binding::gamepad_axis> binding) {
        gamepad_axes[static_cast<std::size_t>(axis)] = std::move(binding);
    }
}