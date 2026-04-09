
#include <SDL3/SDL.h>
#include <span>
#include <optional>
#include "manager.hpp"
#include "action.hpp"

namespace clayborne::input {
    void manager::set_active_map(map::id map) {
        if (map.value >= maps.size()) {
            active_map->value = map::id::invalid;
        }

        active_map = map;
    }

    [[nodiscard]] map::id manager::add_map() {
        map::id map{ .value = maps.size() };
        maps.push_back({});
        return map;
    }

    [[nodiscard]] map::map *manager::get_map(map::id map) {
        if (map.value >= maps.size()) {
            return nullptr;
        }

        return &maps[static_cast<std::size_t>(map.value)];
    }
    
    [[nodiscard]] const map::map *manager::get_map(map::id map) const {
        if (map.value >= maps.size()) {
            return nullptr;
        }

        return &maps[static_cast<std::size_t>(map.value)];
    }

    void manager::clear_events() {
        events.clear();
    }
    
    [[nodiscard]] std::span<const action::event> manager::get_events() const {
        return events;
    }

    inline bool manager::process_key(map::map &map, const SDL_Event& event) {
        auto &key{ map.keyboard[static_cast<std::size_t>(event.key.scancode)] };
        if (!key) {
            return false;
        }

        auto action{ key->apply(event.key) };
        if (!action) {
            return false;
        }

        events.push_back(*action);
        return true;
    }

    inline bool manager::process_mouse_button(map::map &map, const SDL_Event& event) {
        auto &mouse_button{ map.mouse_buttons[static_cast<std::size_t>(event.button.button)] };
        if (!mouse_button) {
            return false;
        }

        auto action{ mouse_button->apply(event.button) };
        if (!action) {
            return false;
        }

        events.push_back(*action);
        return true;
    }

    inline bool manager::process_mouse_motion(map::map &map, const SDL_Event& event) {
        auto &mouse_motion{ map.mouse_motion };
        if (!mouse_motion) {
            return false;
        }

        auto action{ mouse_motion->apply(event.motion) };
        if (!action) {
            return false;
        }

        events.push_back(*action);
        return true;
    }

    inline bool manager::process_mouse_wheel(map::map &map, const SDL_Event& event) {
        auto &mouse_wheel{ map.mouse_wheel };
        if (!mouse_wheel) {
            return false;
        }

        auto action{ mouse_wheel->apply(event.wheel) };
        if (!action) {
            return false;
        }

        events.push_back(*action);
        return true;
    }

    inline bool manager::process_gamepad_button(map::map &map, const SDL_Event& event) {
        auto &gamepad_button{ map.gamepad_buttons[static_cast<std::size_t>(event.gbutton.button)] };
        if (!gamepad_button) {
            return false;
        }

        auto action{ gamepad_button->apply(event.gbutton) };
        if (!action) {
            return false;
        }

        events.push_back(*action);
        return true;
    }

    inline bool manager::process_gamepad_axis(map::map &map, const SDL_Event& event) {
        auto &gamepad_axis{ map.gamepad_axes[static_cast<std::size_t>(event.gaxis.axis)] };
        if (!gamepad_axis) {
            return false;
        }

        auto action{ gamepad_axis->apply(event.gaxis) };
        if (!action) {
            return false;
        }

        events.push_back(*action);
        return true;
    }

    bool manager::process_event(const SDL_Event& event) {
        if (!active_map) {
            return false;
        }

        if (active_map->value >= maps.size()) {
            return false;
        }

        auto &map{ maps[static_cast<std::size_t>(active_map->value)] };

        switch (event.type) {
        case SDL_EVENT_KEY_UP:
            [[fallthrough]];
        case SDL_EVENT_KEY_DOWN:
            return process_key(map, event);
        case SDL_EVENT_MOUSE_BUTTON_UP:
            [[fallthrough]];
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            return process_mouse_button(map, event);
        case SDL_EVENT_MOUSE_MOTION:
            return process_mouse_motion(map, event);
        case SDL_EVENT_MOUSE_WHEEL: 
            return process_mouse_wheel(map, event);
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            [[fallthrough]];
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            return process_gamepad_button(map, event);
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            return process_gamepad_axis(map, event);
        default:
            return false;
        }
    }
}