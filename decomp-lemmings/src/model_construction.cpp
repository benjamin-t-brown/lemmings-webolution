#include "lemmings/model_construction.hpp"

namespace lemmings {
void initialize_model_quad(ModelQuadConstructionState& quad) {
    // 0x420D60 constructs four empty triple/pair members per corner.
    quad.field_00 = 0;
    for(auto& corner:quad.corners_08) corner.triple_00[2] = 0x3f6d7dbf487fcb92ULL;
    for(auto& corner:quad.corners_08) corner.triple_38.fill(0x406fe00000000000ULL); // 255.0
    for(auto& corner:quad.corners_08) corner.field_50 = 0x406fe00000000000ULL;
}
void initialize_animation_base(AnimationConstructionState& state) {
    initialize_model_quad(state.quad);
    state.field_1a8 = -1;
}
void initialize_animation_model(AnimationConstructionState& state) {
    initialize_animation_base(state);
    auto& playback=state.playback;
    playback.direction_1d0 = 1;
    playback.finished_200 = 0;
    state.field_1d8 = 0.0;
    playback.resource_1f8 = nullptr;
    playback.clip_1c8 = 0;
    playback.frame_1cc = 0;
    playback.fraction_1c0 = 0.0;
    state.field_1b0 = 0;
    playback.looping_201 = 1;
    playback.resource_id_1fc = -1;
    state.field_1b8 = 0;
    state.triple_1e0 = {0.0, 0.0, -1.0};
}
void initialize_model_base(ModelConstructionState& state) {
    state.buffer_08[0] = 0; // 0x41A490.
    // Position +90's empty constructor leaves all three doubles untouched.
    state.original_vtable = 0x4b1858;
    state.field_a8 = 1;
    state.mode_88 = 3;
}
void initialize_actor_model(ModelConstructionState& state) {
    initialize_model_base(state);
    initialize_animation_model(state.animation_b0);
    state.original_vtable = 0x4b14a8;
    state.flag_2b8 = 1;
    state.field_2c0 = 0;
    state.field_2c4 = 0;
    state.mode_88 = 1;
}
} // namespace lemmings
