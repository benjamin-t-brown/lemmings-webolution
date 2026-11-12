#include "lemmings/gadget_subboxes.hpp"

namespace lemmings {

int containing_subbox_index(const GadgetSubboxHead& gadget,
                            const ActorHead& actor) {
    for (int box = 0; box < gadget.local_lower.count; ++box) {
        // Both branches in the original have the same six comparisons.
        if (actor.facing != -1 && actor.facing != 1) {
            return -1;
        }

        for (int axis = 0; axis != 3; ++axis) {
            const auto low = static_cast<long double>(gadget.local_lower.data[box].value[axis])
                           + static_cast<long double>(gadget.bounds.minimum[axis]);
            // TEST AH,0x41 accepts x87 less-than, equal, and unordered.
            if (low > static_cast<long double>(actor.position[axis])) {
                goto next_box;
            }
        }
        for (int axis = 0; axis != 3; ++axis) {
            const auto high = static_cast<long double>(gadget.local_upper.data[box].value[axis])
                            + static_cast<long double>(gadget.bounds.minimum[axis]);
            // TEST AH,1 rejects x87 less-than and unordered.
            if (!(high >= static_cast<long double>(actor.position[axis]))) {
                goto next_box;
            }
        }
        return box;

    next_box:
        continue;
    }
    return -1;
}

ActorHead* first_actor_in_subbox(const GadgetSubboxHead& gadget,
                                 const ActorListHead& actors) {
    auto* node = actors.first;
    while (node != &actors.end) {
        auto* next = node->next;
        auto* actor = node->actor;
        if (containing_subbox_index(gadget, *actor) != -1) {
            return actor;
        }
        node = next;
    }
    return nullptr;
}

void collect_subbox_actor_ids(const GadgetSubboxHead& gadget,
                              const ActorListHead& actors,
                              bool all_matches,
                              NativeIntArray& output) {
    auto* node = actors.first;
    clear_native_int_array(output);
    while (node != &actors.end) {
        auto* next = node->next;
        auto* actor = node->actor;
        if (containing_subbox_index(gadget, *actor) != -1) {
            append_native_int(output, actor->identifier);
            if (!all_matches) {
                return;
            }
        }
        node = next;
    }
}

} // namespace lemmings
