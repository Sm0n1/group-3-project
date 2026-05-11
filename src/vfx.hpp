#include <entt/entt.hpp>

namespace clayborne {
	struct vfx {
		int age; // Number of frames it has existed
		int lifespan; // Number of frames it shoudl exist
	};

	void update_effects(entt::registry& registry);

}