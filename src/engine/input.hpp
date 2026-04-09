#ifndef CLAYBORNE_INPUT_HPP
#define CLAYBORNE_INPUT_HPP

#include <SDL3/SDL.h>
#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace clayborne::input {
    template <class Enum>
    constexpr std::size_t to_index(Enum e) noexcept {
        return static_cast<std::size_t>(e);
    }

    template<typename Context>
    concept context = requires(Context &ctx, const SDL_Event &event) {
        { Context::handle_event(ctx, event) } -> std::same_as<bool>;
    };
    
    enum class state_tag {
        up,
        down,
    };

    enum class event_tag : std::uint8_t {
        pressed,
        released,
    };

    struct menu_context {
        enum class command_tag : std::uint8_t {
            confirm,
            cancel,
            move_up, 
            move_down,
            move_left,
            move_right,
            //
            count,
        };

        struct event {
            event_tag event;
            command_tag command;
        };

        std::array<state_tag, to_index(command_tag::count)> states;
        std::vector<event> events;
        std::array<std::optional<command_tag>, SDL_SCANCODE_COUNT> keybinds;

        static menu_context init();
        static bool handle_event(menu_context &ctx, const SDL_Event &event);
    };

    struct gameplay_context {
        enum class command_tag : std::uint8_t {
            jump,
            use_head,
            move_up,
            move_down,
            move_left,
            move_right,
            //
            count,
        };

        struct event {
            event_tag event;
            command_tag command;
        };

        std::array<state_tag, to_index(command_tag::count)> states;
        std::vector<event> events;
        std::array<std::optional<command_tag>, SDL_SCANCODE_COUNT> keybinds;

        static gameplay_context init();
        static bool handle_event(gameplay_context &ctx, const SDL_Event &event);
        static bool is_move_up(const gameplay_context &ctx);
        static bool is_move_down(const gameplay_context &ctx);
        static bool is_move_left(const gameplay_context &ctx);
        static bool is_move_right(const gameplay_context &ctx);
    };

    struct manager {
        enum class context_tag {
            menu,
            gameplay,
        };

        menu_context menu{ menu_context::init() };
        gameplay_context gameplay{ gameplay_context::init() };
        context_tag active_context{ context_tag::gameplay };

        static manager init();

        // Returns true in case event was handled, otherwise false.
        static bool handle_event(manager &manager, const SDL_Event &event);
        static void clear_events(manager &manager);
    };
}

// There are three different possible states the game can be in:
// - There is only the menu being visualized and interacted with
// - There is only the gameplay being visualized and interacted with
// - Both the gameplay and menu state is stored, but the game is paused and the menu is interacted with
// 
// In terms of input, either the menu or gameplay is active.
// In terms of the simulation, every gameplay related system should be paused when the menu is open.
// Using a shared registry, this would require a menu or gameplay component and for every system to only
// simuluate the relevant entities.
// Using two registries would allow 

#endif // CLAYBORNE_INPUT_HPP