#pragma once
#include "lemmings/actor_update.hpp"
#include <span>

namespace lemmings {
struct ActorDestructionState {
    ActorBehaviorUpdate& behaviors; // ID +3A, primary +340 and extra list +374.
    std::uint32_t original_vtable = 0;
    std::int32_t category_4c = 0;
    void* shadow_344 = nullptr;
};
struct ActorIdRegistry {
    std::span<void*> slots;
    std::int32_t first_id = 0; // slots[0] corresponds to this signed ID.
};
struct ActorDestructionCallbacks {
    virtual ~ActorDestructionCallbacks() = default;
    virtual void destroy_shadow_42fec0(ActorDestructionState&, void* shadow, std::uint32_t flags) = 0;
    virtual void refresh_444abc(ActorDestructionState&) = 0;
    virtual void destroy_extra_list_430110(ActorDestructionState&) = 0;
    virtual void destroy_model_43ce88(ActorDestructionState&) = 0;
    virtual void unregister_world_43b536(ActorDestructionState&) = 0;
};

// 0x42E70A: extra-state leave/destroy/release first, then primary leave/destroy.
// Uses the existing mutation-sensitive helpers without adding null guards.
void clear_actor_behaviors(ActorBehaviorUpdate&, const ActorBehaviorClearHooks&);

// 0x42316C plus 0x423380: write base vtable, null the signed-ID registry slot,
// then unregister from the world. Empty position destructor performs no writes.
void destroy_actor_base(ActorDestructionState&, ActorIdRegistry, ActorDestructionCallbacks&);

// 0x42E17C normal-return orchestration, including concrete 0x42E70A cleanup
// and base destruction. Shadow remains stored after its deleting callback.
// Category must index counts when cleanup returns; current signed ID must be
// in registry after model teardown. Primary must be valid when its cleanup
// begins; callbacks must preserve helper traversal/lifetime requirements and
// return normally. Original SEH exceptional unwind is outside this projection.
void destroy_actor(ActorDestructionState&, std::span<std::uint16_t> counts,
    ActorIdRegistry, const ActorBehaviorClearHooks&, ActorDestructionCallbacks&);
} // namespace lemmings
