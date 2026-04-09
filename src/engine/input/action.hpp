#ifndef CLAYBORNE_INPUT_ACTION_HPP
#define CLAYBORNE_INPUT_ACTION_HPP

#include <SDL3/SDL.h>
#include <cstdint>
#include <limits>
#include <string>

namespace clayborne::input::action {
    struct id {
        static constexpr std::uint64_t invalid{ std::numeric_limits<std::uint64_t>::max() };

        std::uint64_t value = invalid;

        [[nodiscard]] constexpr bool valid() const noexcept {
            return value != invalid;
        }

        constexpr std::strong_ordering operator<=>(const id&) const noexcept = default;
    };

    struct event {
        id action;
        Uint64 timestamp_ns;
        std::uint64_t flags;
        float axis;
    };

    enum class tag : std::uint8_t {
        gameplay_jump,
        gameplay_use_head,
        gameplay_move,
        menu_confirm,
        menu_cancel,
        menu_move,
        pause,
    };

    struct directions {
        bool up;
        bool down;
        bool left;
        bool right;
    };

    union data {
        bool gameplay_jump;
        bool gameplay_use_head;
        directions gameplay_move;
        directions menu_move;
    };

    struct descriptor {
        tag t;
        std::string name;
    };
}

#endif // CLAYBORNE_INPUT_ACTION_HPP