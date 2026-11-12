#include "lemmings/actor_gadget_contact.hpp"
#include <cassert>
#include <cmath>
#include <limits>
using namespace lemmings;
int main() {
    GadgetBoundsHead receiver{};
    receiver.minimum[0] = 2; receiver.maximum[0] = 6;
    std::array<std::array<double,3>,6> points{};
    points[4][1] = 6; points[5][1] = 2;
    GadgetSurfaceView surface{&receiver, points};
    assert(gadget_surface_y(surface, 2) == 2 && gadget_surface_y(surface, 6) == 6);
    GadgetBoundsHead contact{};
    contact.minimum[0] = 0; contact.maximum[0] = 8;
    contact.minimum[1] = 3; contact.maximum[1] = 3.5;
    contact.minimum[2] = 0; contact.maximum[2] = 1;
    ActorGeometryState actor{};
    actor.position = {3,2.5,0.5}; actor.pair_360 = {0.5,0.5};
    CollisionGridHead grid{}; grid.height = 8;
    assert(adjust_actor_gadget_contact(surface,contact,actor,grid)); // second point only
    assert(actor.position[1] == 3); // slope 3.5, sample offset .5
    actor.position[0] = 9;
    const auto before = actor.position;
    assert(!adjust_actor_gadget_contact(surface,contact,actor,grid));
    assert(actor.position == before);
    receiver.maximum[0] = receiver.minimum[0];
    assert(std::isnan(gadget_surface_y(surface, 2)));
    assert(std::isinf(gadget_surface_y(surface, 3)));
}
