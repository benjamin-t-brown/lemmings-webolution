#include "lemmings/actor_motion.hpp"
#include "lemmings/cylinder_coordinate.hpp"

namespace lemmings {

void move_actor_sample_x(ActorGeometryState& actor, double sample_x) {
    const double x = static_cast<double>(
        static_cast<long double>(sample_x) - actor.pair_360[0]);
    actor.position[0] = static_cast<double>(wrap_cylinder_x(x));
}

void move_actor_sample_y(ActorGeometryState& actor, double sample_y,
                         const CollisionGridHead& grid) {
    const double y = static_cast<double>(
        static_cast<long double>(sample_y) - actor.pair_360[1]);
    actor.position[1] = static_cast<double>(clamp_cylinder_y(grid, y));
}

} // namespace lemmings
