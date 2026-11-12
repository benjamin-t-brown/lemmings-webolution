#include "lemmings/gadget_subboxes.hpp"

#include <limits>
#include <stdexcept>

namespace {
void check(bool condition) {
    if (!condition) {
        throw std::runtime_error("gadget subbox mismatch");
    }
}
}

int main() {
    lemmings::GadgetSubboxHead gadget{};
    gadget.bounds.minimum[0] = 10.0;
    gadget.bounds.minimum[1] = 20.0;
    gadget.bounds.minimum[2] = 30.0;
    const lemmings::Vec3d lower[2] = {{{0.0, 0.0, 0.0}}, {{5.0, 5.0, 5.0}}};
    const lemmings::Vec3d upper[2] = {{{1.0, 1.0, 1.0}}, {{6.0, 6.0, 6.0}}};
    gadget.local_lower.count = 2;
    gadget.local_lower.data = lower;
    gadget.local_upper.count = 2;
    gadget.local_upper.data = upper;

    lemmings::ActorHead actor{};
    actor.facing = -1;
    actor.position[0] = 15.0;
    actor.position[1] = 25.0;
    actor.position[2] = 35.0;
    check(lemmings::containing_subbox_index(gadget, actor) == 1);

    actor.position[0] = 11.0;
    actor.position[1] = 21.0;
    actor.position[2] = 31.0;
    check(lemmings::containing_subbox_index(gadget, actor) == 0);
    actor.facing = 1;
    check(lemmings::containing_subbox_index(gadget, actor) == 0);
    actor.facing = 0;
    check(lemmings::containing_subbox_index(gadget, actor) == -1);
    actor.facing = -1;
    actor.position[0] = 11.1;
    check(lemmings::containing_subbox_index(gadget, actor) == -1);

    // The x87 FADD intermediate keeps the +1 beyond double's 2^53 spacing.
    const lemmings::Vec3d large_lower[1] = {{{0x1p53, 0.0, 0.0}}};
    const lemmings::Vec3d large_upper[1] = {{{0x1p54, 1.0, 1.0}}};
    gadget.local_lower.data = large_lower;
    gadget.local_upper.data = large_upper;
    gadget.local_lower.count = 1;
    gadget.bounds.minimum[0] = 1.0;
    gadget.bounds.minimum[1] = 0.0;
    gadget.bounds.minimum[2] = 0.0;
    actor.position[0] = 0x1p53;
    actor.position[1] = 0.5;
    actor.position[2] = 0.5;
    check(lemmings::containing_subbox_index(gadget, actor) == -1);

    gadget.bounds.minimum[0] = 0.0;
    check(lemmings::containing_subbox_index(gadget, actor) == 0);

    lemmings::ActorListHead list{};
    list.first = &list.end;
    check(lemmings::first_actor_in_subbox(gadget, list) == nullptr);

    lemmings::ActorHead miss{};
    miss.facing = 1;
    miss.position[0] = 1.0;
    lemmings::ActorListNode second{&actor, &list.end};
    lemmings::ActorListNode first{&miss, &second};
    list.first = &first;
    check(lemmings::first_actor_in_subbox(gadget, list) == &actor);
    second.actor = &miss;
    check(lemmings::first_actor_in_subbox(gadget, list) == nullptr);

    lemmings::Vec3d unordered_lower[1] = {{{
        std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0
    }}};
    lemmings::Vec3d unordered_upper[1] = {{{1.0, 1.0, 1.0}}};
    gadget.local_lower.data = unordered_lower;
    gadget.local_upper.data = unordered_upper;
    actor.position[0] = 0.5;
    check(lemmings::containing_subbox_index(gadget, actor) == 0);
    unordered_upper[0].value[0] = std::numeric_limits<double>::quiet_NaN();
    check(lemmings::containing_subbox_index(gadget, actor) == -1);
    unordered_upper[0].value[0] = 1.0;
    actor.position[0] = std::numeric_limits<double>::quiet_NaN();
    check(lemmings::containing_subbox_index(gadget, actor) == -1);

    actor.position[0] = 0.5;
    lemmings::ActorHead another = actor;
    actor.identifier = -123;
    another.identifier = 456;
    second.actor = &another;
    list.first = &second;
    lemmings::NativeIntArray ids{};
    ids.growth = 2;
    lemmings::collect_subbox_actor_ids(gadget, list, true, ids);
    check(ids.count == 1 && ids.data[0] == 456);

    first.actor = &actor;
    first.next = &second;
    list.first = &first;
    lemmings::collect_subbox_actor_ids(gadget, list, false, ids);
    check(ids.count == 1 && ids.data[0] == -123);
    lemmings::collect_subbox_actor_ids(gadget, list, true, ids);
    check(ids.count == 2 && ids.data[0] == -123 && ids.data[1] == 456);
    lemmings::clear_native_int_array(ids);
}
