#pragma once
#include "lemmings/animation_playback.hpp"
#include <array>

namespace lemmings {
// Offsets within a 0x68-byte record. Empty subconstructors preserve all fields;
// 0x440A10 writes only triple00[2], triple38, and field50. Bit fields keep
// unknown meaning and signaling NaNs explicit without floating point loads.
struct ModelCornerConstructionState {
    std::array<std::uint64_t,3> triple_00{};
    std::array<std::uint64_t,2> pair_18{}, pair_28{};
    std::array<std::uint64_t,3> triple_38{};
    std::uint64_t field_50 = 0;
    std::array<std::uint64_t,2> unknown_58{};
};
struct ModelQuadConstructionState {
    std::uint32_t field_00 = 0, unknown_04 = 0;
    std::array<ModelCornerConstructionState,4> corners_08{};
};
struct AnimationConstructionState {
    ModelQuadConstructionState quad;
    std::int32_t field_1a8 = 0;
    std::uint32_t unknown_1ac = 0, field_1b0 = 0, unknown_1b4 = 0;
    std::uint32_t field_1b8 = 0, unknown_1bc = 0;
    AnimationPlayback playback;
    std::uint32_t unknown_1d4 = 0;
    double field_1d8 = 0;
    std::array<double,3> triple_1e0{};
};
struct ModelConstructionState {
    std::uint32_t original_vtable = 0, unknown_04 = 0;
    std::array<std::uint8_t,128> buffer_08{}; // 0x41A490 clears only first byte.
    std::int32_t mode_88 = 0;
    std::uint32_t unknown_8c = 0;
    std::array<std::uint64_t,3> position_90{}; // Empty 0x401B80 constructor.
    std::int32_t field_a8 = 0;
    std::uint32_t unknown_ac = 0;
    AnimationConstructionState animation_b0;
    std::array<std::uint8_t,6> unknown_2b2{};
    std::uint8_t flag_2b8 = 0;
    std::array<std::uint8_t,7> unknown_2b9{};
    std::uint32_t field_2c0 = 0, field_2c4 = 0;
};

void initialize_model_quad(ModelQuadConstructionState&);       // 0x440A10
void initialize_animation_base(AnimationConstructionState&);   // 0x4409E0
void initialize_animation_model(AnimationConstructionState&);  // 0x4680F0
void initialize_model_base(ModelConstructionState&);           // 0x465840
void initialize_actor_model(ModelConstructionState&);          // 0x43CE00
// Normal construction is completely concrete; exceptional CRT/SEH unwind is
// not modeled. These field projections do not promise original ABI layout.
} // namespace lemmings
