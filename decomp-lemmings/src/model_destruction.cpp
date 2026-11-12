#include "lemmings/model_destruction.hpp"

namespace lemmings {
void release_animation_model_resource(AnimationPlayback& playback, const AnimationResourceHooks& hooks) {
    if (playback.resource_id_1fc == -1) return;
    if (playback.resource_1f8 != nullptr) hooks.release(hooks.context, playback.resource_id_1fc);
    playback.resource_id_1fc = -1;
    playback.resource_1f8 = nullptr;
}
void destroy_animation_model(AnimationConstructionState& state, const AnimationResourceHooks& hooks) {
    release_animation_model_resource(state.playback, hooks);
    // 0x440BF0 -> 0x440C10 -> reverse four 0x420DA0 destructors, all empty.
}
void destroy_model_base(ModelConstructionState& model) {
    model.original_vtable = 0x4b1858;
    // Component +8's 0x41A4B0 destructor performs no writes.
}
void destroy_actor_model(ModelConstructionState& model, const AnimationResourceHooks& hooks) {
    model.original_vtable = 0x4b14a8;
    destroy_animation_model(model.animation_b0, hooks);
    destroy_model_base(model);
}
} // namespace lemmings
