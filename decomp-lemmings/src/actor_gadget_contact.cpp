#include "lemmings/actor_gadget_contact.hpp"
#include "lemmings/actor_motion.hpp"

namespace lemmings {
long double gadget_surface_y(const GadgetSurfaceView& gadget, double x) {
    const auto& first = gadget.model_points[4];
    const auto& second = gadget.model_points[5];
    const double rise = static_cast<double>(static_cast<long double>(first[1]) - second[1]);
    const auto& origin = gadget.model_points[5];
    return ((static_cast<long double>(x) - gadget.bounds->minimum[0]) /
            (static_cast<long double>(gadget.bounds->maximum[0]) - gadget.bounds->minimum[0])) *
           rise + origin[1];
}
bool adjust_actor_gadget_contact(const GadgetSurfaceView& receiver,
    const GadgetBoundsHead& contact_bounds, ActorGeometryState& actor,
    const CollisionGridHead& grid) {
    const auto sample = actor_sample_position_360(actor);
    double point[3]{sample[0], static_cast<double>(static_cast<long double>(sample[1]) - actor.direction_44), sample[2]};
    if (!gadget_contains_point(contact_bounds, point)) {
        point[1] = static_cast<double>(static_cast<long double>(actor.direction_44) + point[1]);
        if (!gadget_contains_point(contact_bounds, point)) return false;
    }
    move_actor_sample_y(actor, static_cast<double>(gadget_surface_y(receiver, point[0])), grid);
    return true;
}
} // namespace lemmings
