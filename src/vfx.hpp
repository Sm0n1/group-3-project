#include <entt/entt.hpp>

namespace clayborne {
	struct vfx {
		int age;      // Number of frames it has existed
		int lifespan; // Number of frames it should exist
	};

	void update_effects(
		entt::registry &registry
	);
}