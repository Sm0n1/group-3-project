#include "vfx.hpp"
#include <SDL3/SDL.h>

namespace clayborne {
	void update_effects(entt::registry& registry) {
		auto view{ registry.view<struct vfx>() };

		for (auto entity : view) {
			auto &vfx{ view.get<struct vfx>(entity) };

			if (vfx.age >= vfx.lifespan) {
				registry.destroy(entity);
			}

			vfx.age += 1;
		}
	}
}