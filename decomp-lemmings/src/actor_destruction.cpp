#include "lemmings/actor_destruction.hpp"

namespace lemmings {
void clear_actor_behaviors(ActorBehaviorUpdate& actor, const ActorBehaviorClearHooks& hooks) {
    clear_actor_extra_behaviors(actor, hooks);
    clear_actor_primary_behavior(actor);
}
void destroy_actor_base(ActorDestructionState& actor, ActorIdRegistry registry,
    ActorDestructionCallbacks& callbacks) {
    actor.original_vtable = 0x4b0e90;
    const auto index = static_cast<std::int64_t>(actor.behaviors.identifier_3a) - registry.first_id;
    registry.slots[static_cast<std::size_t>(index)] = nullptr;
    callbacks.unregister_world_43b536(actor);
}
void destroy_actor(ActorDestructionState& actor, std::span<std::uint16_t> counts,
    ActorIdRegistry registry, const ActorBehaviorClearHooks& hooks,
    ActorDestructionCallbacks& callbacks) {
    actor.original_vtable = 0x4b10e8;
    if (actor.shadow_344 != nullptr) callbacks.destroy_shadow_42fec0(actor, actor.shadow_344, 1);
    clear_actor_behaviors(actor.behaviors, hooks);
    const auto decremented = static_cast<std::uint16_t>(counts[actor.category_4c] - 1u);
    counts[actor.category_4c] = decremented;
    callbacks.refresh_444abc(actor);
    callbacks.destroy_extra_list_430110(actor);
    callbacks.destroy_model_43ce88(actor);
    destroy_actor_base(actor, registry, callbacks);
}
} // namespace lemmings
