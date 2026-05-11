#include "vfx.hpp"
#include <SDL3/SDL.h>

namespace clayborne {

	void update_effects(entt::registry& registry) {
		auto view{ registry.view<struct vfx>() };
		for (auto [e, v] : view.each()) {
			v.age++;
			if (v.age > v.lifespan) {
				registry.destroy(e);
			}
		}
	}
}