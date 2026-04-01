#ifndef CLAYBORNE_PHYSICS_HPP
#define CLAYBORNE_PHYSICS_HPP

namespace clayborne {
    struct position {
        float x;
        float y;
    };

    struct velocity {
        float x;
        float y;
    };

    struct collider {
        float w;
        float h;
    };

    void update_physics(entt::registry &registry);
}

#endif // CLAYBORNE_PHYSICS_HPP