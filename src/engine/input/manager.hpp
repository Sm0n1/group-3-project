#ifndef CLAYBORNE_INPUT_MANAGER_HPP
#define CLAYBORNE_INPUT_MANAGER_HPP

#include <SDL3/SDL.h>
#include <span>
#include <vector>
#include "action.hpp"
#include "map.hpp"

namespace clayborne::input {
    struct manager {
        std::optional<map::id> active_map;
        std::vector<map::map> maps;

        std::vector<trigger::descriptor> triggers;
        std::vector<action::descriptor> actions;

        std::vector<action::event> events;

        [[nodiscard]] trigger::id add_trigger();
        [[nodiscard]] trigger::descriptor *get_trigger(trigger::id map);
        [[nodiscard]] const trigger::descriptor *get_trigger(trigger::id map) const;

        [[nodiscard]] action::id add_action();
        [[nodiscard]] action::descriptor *get_action(action::id map);
        [[nodiscard]] const action::descriptor *get_action(action::id map) const;

        [[nodiscard]] map::id add_map();
        [[nodiscard]] map::map *get_map(map::id map);
        [[nodiscard]] const map::map *get_map(map::id map) const;

        void set_active_map(map::id map);

        void clear_events();
        
        [[nodiscard]] std::span<const action::event> get_events() const;

        inline bool process_key(map::map &map, const SDL_Event& event);
        inline bool process_mouse_button(map::map &map, const SDL_Event& event);
        inline bool process_mouse_motion(map::map &map, const SDL_Event& event);
        inline bool process_mouse_wheel(map::map &map, const SDL_Event& event);
        inline bool process_gamepad_button(map::map &map, const SDL_Event& event);
        inline bool process_gamepad_axis(map::map &map, const SDL_Event& event);

        bool process_event(const SDL_Event& event);
    };
}

#endif // CLAYBORNE_INPUT_MANAGER_HPP