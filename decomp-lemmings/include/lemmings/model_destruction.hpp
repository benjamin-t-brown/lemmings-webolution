#pragma once
#include "lemmings/model_construction.hpp"

namespace lemmings {
// 0x468255: ID==-1 preserves even a nonnull resource pointer. Other IDs call
// release only if the pointer is nonnull, then reset ID/pointer after callback.
// release must be present when called; callback mutation remains visible in
// every other field. The hook represents manager 0x58D23C / method 0x430050.
void release_animation_model_resource(AnimationPlayback&, const AnimationResourceHooks&);

// 0x46820D: release, then 0x440BF0/0x440C10 destroy four empty corner records.
void destroy_animation_model(AnimationConstructionState&, const AnimationResourceHooks&);
// 0x4658B7: writes base vtable; component 0x41A4B0 destructor is empty.
void destroy_model_base(ModelConstructionState&);
// 0x43CE88: actor-model vtable, animation destruction, then model base teardown.
// Normal-return destructor body only; resource management is the sole effect
// boundary. No allocation release or exceptional CRT/SEH unwind is claimed.
void destroy_actor_model(ModelConstructionState&, const AnimationResourceHooks&);
} // namespace lemmings
