#include "animation.hpp"
#include "camera.hpp"

namespace clayborne {
    void animate(entt::registry &registry) {
        auto view{ registry.view<renderer, animator>() };
        for (auto [e, r, a] : view.each()) {
            if(!a.resource) {
                continue;
            }

            const auto &animation{ a.resource->frames };
            const auto frame{ animation[a.current_frame] };

            r.srcrect.x = frame.x;
            r.srcrect.y = frame.y;
            r.srcrect.w = frame.w;
            r.srcrect.h = frame.h;
            
            if (a.current_frame++ >= animation.size()) {
                a.current_frame = !a.is_looping * animation.size();
            }
        }
    }
}