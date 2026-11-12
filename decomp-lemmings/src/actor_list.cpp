#include "lemmings/actor_list.hpp"

namespace lemmings {

ActorHead* first_actor_inside(const GadgetBoundsHead& gadget,
                              const ActorListHead& actors) {
    auto* node = actors.first;
    while (node != &actors.end) {
        auto* next = node->next;
        auto* actor = node->actor;
        if (gadget_contains_point(gadget, actor->position)) {
            return actor;
        }
        node = next;
    }
    return nullptr;
}

} // namespace lemmings
