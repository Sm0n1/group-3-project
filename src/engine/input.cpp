#include <SDL3/SDL.h>
#include "input.hpp"

namespace clayborne::input {
    menu_context menu_context::init() {
        menu_context menu_context;

        menu_context.states.fill(state_tag::up);
        menu_context.keybinds[SDL_SCANCODE_W] = menu_context::command_tag::move_up;
        menu_context.keybinds[SDL_SCANCODE_A] = menu_context::command_tag::move_left;
        menu_context.keybinds[SDL_SCANCODE_S] = menu_context::command_tag::move_down;
        menu_context.keybinds[SDL_SCANCODE_D] = menu_context::command_tag::move_right;
        menu_context.keybinds[SDL_SCANCODE_RETURN] = menu_context::command_tag::confirm;
        menu_context.keybinds[SDL_SCANCODE_ESCAPE] = menu_context::command_tag::cancel;

        return menu_context;
    }

    bool menu_context::handle_event(menu_context &ctx, const SDL_Event &event) {
        (void)ctx;
        (void)event;
        return false;
    }

    gameplay_context gameplay_context::init() {
        gameplay_context gameplay_context;

        gameplay_context.states.fill(state_tag::up);
        gameplay_context.keybinds[SDL_SCANCODE_W] = gameplay_context::command_tag::move_up;
        gameplay_context.keybinds[SDL_SCANCODE_A] = gameplay_context::command_tag::move_left;
        gameplay_context.keybinds[SDL_SCANCODE_S] = gameplay_context::command_tag::move_down;
        gameplay_context.keybinds[SDL_SCANCODE_D] = gameplay_context::command_tag::move_right;
        gameplay_context.keybinds[SDL_SCANCODE_J] = gameplay_context::command_tag::jump;
        gameplay_context.keybinds[SDL_SCANCODE_K] = gameplay_context::command_tag::use_head;

        return gameplay_context;
    }

    manager manager::init() {
        return manager{};
    }

    bool gameplay_context::handle_event(gameplay_context &ctx, const SDL_Event &event) {
        switch (event.type) {
        case SDL_EVENT_KEY_UP: {
            if (event.key.repeat) {
                return false;
            }

            auto keybind{ ctx.keybinds[event.key.scancode] };
            if (!keybind) {
                return false;
            }

            auto command{ *keybind }; // deref for no safety check
            ctx.states[to_index(command)] = state_tag::up;
            ctx.events.push_back({ event_tag::released, command });

            return true;
        }
        case SDL_EVENT_KEY_DOWN: {
            if (event.key.repeat) {
                return false;
            }
            
            auto keybind{ ctx.keybinds[event.key.scancode] };
            if (!keybind) {
                return false;
            }

            auto command{ *keybind }; // deref for no safety check
            ctx.states[to_index(command)] = state_tag::down;
            ctx.events.push_back({ event_tag::pressed, command });

            return true;
        }
        default:
            return false;
        }
    }

    bool gameplay_context::is_move_up(const gameplay_context &ctx) {
        return ctx.states[to_index(gameplay_context::command_tag::move_up)] == state_tag::down;
    }

    bool gameplay_context::is_move_down(const gameplay_context &ctx) {
        return ctx.states[to_index(gameplay_context::command_tag::move_down)] == state_tag::down;
    }

    bool gameplay_context::is_move_left(const gameplay_context &ctx) {
        return ctx.states[to_index(gameplay_context::command_tag::move_left)] == state_tag::down;
    }

    bool gameplay_context::is_move_right(const gameplay_context &ctx) {
        return ctx.states[to_index(gameplay_context::command_tag::move_right)] == state_tag::down;
    }

    bool manager::handle_event(manager &manager, const SDL_Event &event) {
        switch (manager.active_context) {
        case context_tag::menu:
            return menu_context::handle_event(manager.menu, event);
        case context_tag::gameplay:
            return gameplay_context::handle_event(manager.gameplay, event);
        }

        return false;
    }

    void manager::clear_events(manager &manager) {
        manager.menu.events.clear();
        manager.gameplay.events.clear();
    }
}