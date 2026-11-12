#include "lemmings/actor_list.hpp"

#include <stdexcept>

namespace {
void check(bool condition) {
    if (!condition) {
        throw std::runtime_error("actor list search mismatch");
    }
}
}

int main() {
    lemmings::GadgetBoundsHead gadget{};
    for (int axis = 0; axis != 3; ++axis) {
        gadget.minimum[axis] = 0.0;
        gadget.maximum[axis] = 1.0;
    }

    lemmings::ActorListHead list{};
    list.first = &list.end;
    check(lemmings::first_actor_inside(gadget, list) == nullptr);

    lemmings::ActorHead outside{};
    outside.position[0] = 2.0;
    lemmings::ActorHead first_inside{};
    first_inside.position[0] = 0.5;
    lemmings::ActorHead second_inside{};
    second_inside.position[0] = 1.0;

    lemmings::ActorListNode third{&second_inside, &list.end};
    lemmings::ActorListNode second{&first_inside, &third};
    lemmings::ActorListNode first{&outside, &second};
    list.first = &first;
    check(lemmings::first_actor_inside(gadget, list) == &first_inside);

    second.actor = &outside;
    check(lemmings::first_actor_inside(gadget, list) == &second_inside);

    third.actor = &outside;
    check(lemmings::first_actor_inside(gadget, list) == nullptr);
}
