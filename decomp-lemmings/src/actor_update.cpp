#include "lemmings/actor_update.hpp"

namespace lemmings {

ActorBehaviorList::ActorBehaviorList() {
    begin.next = &end;
    end.previous = &begin;
}

void attach_actor_behavior(ActorBehaviorUpdate& actor, ActorBehavior& state,
                           const ActorBehaviorAttachmentHooks& hooks) {
    state.operations->enter(state, actor);
    if (state.extra_11 != 0) {
        auto* node = hooks.allocate_node(hooks.context);
        node->behavior = &state;
        node->next = actor.extra.begin.next;
        node->previous = &actor.extra.begin;
        node->next->previous = node;
        actor.extra.begin.next = node;
    } else {
        if (actor.primary != nullptr) actor.primary->operations->leave(*actor.primary, actor);
        actor.primary = &state;
    }
}

bool has_actor_behavior(const ActorBehaviorUpdate& actor, std::int32_t type) {
    if (actor.primary != nullptr && actor.primary->type_14 == type) return true;
    for (auto* node = actor.extra.begin.next; node != &actor.extra.end; node = node->next) {
        if (node->behavior->type_14 == type) return true;
    }
    return false;
}

ActorBehavior* find_actor_behavior(const ActorBehaviorUpdate& actor, std::int32_t type) {
    if (actor.primary->type_14 == type) return actor.primary;
    for (auto* node = actor.extra.begin.next; node != &actor.extra.end; node = node->next)
        if (node->behavior->type_14 == type) return node->behavior;
    return nullptr;
}

void clear_actor_primary_behavior(ActorBehaviorUpdate& actor) {
    actor.primary->operations->leave(*actor.primary, actor);
    auto* const current = actor.primary;
    if (current != nullptr) current->operations->destroy(*current, 1);
    actor.primary = nullptr;
}

bool remove_actor_behavior(ActorBehaviorUpdate& actor, std::int32_t type,
                           const ActorBehaviorClearHooks& hooks) {
    auto* const state = find_actor_behavior(actor, type);
    if (state == nullptr) return false;
    state->operations->leave(*state, actor);
    if (state == actor.primary) {
        auto* const primary = actor.primary;
        if (primary != nullptr) primary->operations->destroy(*primary, 1);
        actor.primary = nullptr;
    } else {
        // Original 0x4301B0 searches by identity again after leave, then the
        // caller dereferences its result without an absent-node check.
        auto* node = actor.extra.begin.next;
        while (node != &actor.extra.end && node->behavior != state) node = node->next;
        if (node == &actor.extra.end) node = nullptr;
        if (node->behavior != nullptr) node->behavior->operations->destroy(*node->behavior, 1);
        node->previous->next = node->next;
        node->next->previous = node->previous;
        hooks.release_node(hooks.context, *node);
    }
    return true;
}

void clear_actor_extra_behaviors(ActorBehaviorUpdate& actor,
                                 const ActorBehaviorClearHooks& hooks) {
    for (auto* node = actor.extra.begin.next; node != &actor.extra.end; node = node->next) {
        auto* state = node->behavior;
        state->operations->leave(*state, actor);
    }
    auto* node = actor.extra.begin.next;
    while (node != &actor.extra.end) {
        auto* state = node->behavior;
        if (state != nullptr) state->operations->destroy(*state, 1);
        auto* next = node->next;
        hooks.release_node(hooks.context, *node);
        node = next;
    }
    actor.extra.begin.next = &actor.extra.end;
    actor.extra.begin.previous = nullptr;
    actor.extra.end.next = nullptr;
    actor.extra.end.previous = &actor.extra.begin;
}

bool update_actor_behavior_schedule(ActorBehaviorUpdate& actor, double elapsed,
                                    const ActorBehaviorHooks& hooks) {
    int passes = actor.mode_50 == 2 ? 2 : 1;
    while (passes-- > 0) {
        double remaining = elapsed;
        while (remaining > 0.0) {
            auto* const previous = actor.primary;
            std::uint32_t result = 3;
            if (actor.forced_transition_5d != 0) {
                hooks.forced_event(hooks.context, actor.identifier_3a,
                                   hooks.forced_event_value, 0);
                auto* replacement = hooks.create_forced(hooks.context, actor);
                hooks.attach(hooks.context, actor, replacement);
                result = 1;
                actor.forced_transition_5d = 0;
            } else if (previous->phase_10 == 1 ||
                       (previous->phase_10 == 0 && passes == 0)) {
                result = previous->operations->update(*previous, actor, remaining);
            }
            if (result == 2) return false;
            if (result == 1) {
                if (previous != nullptr) previous->operations->destroy(*previous, 1);
            } else if (result == 3) {
                remaining = 0.0;
            }
        }

        auto* node = actor.extra.begin.next;
        while (node != &actor.extra.end) {
            double remaining = elapsed;
            auto* const state = node->behavior;
            std::uint32_t result = 3;
            if (state->phase_10 == 1 || (state->phase_10 == 0 && passes == 0)) {
                result = state->operations->update(*state, actor, remaining);
            }
            switch (result) {
            case 0:
            case 3:
                node = node->next;
                break;
            case 1: {
                state->operations->leave(*state, actor);
                if (state != nullptr) state->operations->destroy(*state, 1);
                auto* const next = node->next;
                node->previous->next = node->next;
                node->next->previous = node->previous;
                hooks.release_node(hooks.context, *node);
                node = next;
                break;
            }
            case 2:
                return false;
            }
        }
    }
    hooks.finalize(hooks.context, actor);
    return true;
}

} // namespace lemmings
