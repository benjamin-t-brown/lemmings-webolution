#pragma once
#include "lemmings/actor_geometry.hpp"
#include "lemmings/animation_playback.hpp"
#include "lemmings/world_geometry_prefix.hpp"
#include "lemmings/world_coordinates.hpp"

namespace lemmings {
using SampleTriple = std::array<std::uint64_t, 3>;
struct WorldSampleTable {
    std::span<const std::uint8_t> indices_664;
    std::span<const std::uint8_t, 6144> triples_678;
};
// New view adapter over the serialized fields, with no asset copies.
WorldSampleTable world_sample_table(const WorldGeometryPrefix& world);
// 0x403F40 + 0x403FC0/0x403FA0. X wraps by masking, Y is not clamped.
// Throws for a sample outside the supplied index storage; original is unchecked.
SampleTriple world_sample_triple(const WorldSampleTable&, std::uint32_t x, std::int32_t y);

struct ActorFinalizationState {
    ActorGeometryState geometry;
    AnimationPlayback animation; // actor +0x128
    std::int32_t orientation_2d8 = 0;
    // Original playback offsets 0x40, 0xA8, 0x110, 0x178. Raw bits are copied.
    std::array<SampleTriple, 4> model_triples{};
    std::array<double, 3> model_position_108{};
    void* model_78 = nullptr;
    void* effect_344 = nullptr;
};
// Model virtual slot +8: 0x4658D6 maps actor coordinates, 0x465900 copies
// the mapped triple to inline model +0x90 (actor +0x108).
void position_actor_model(ActorFinalizationState&, const WorldCoordinateMesh&);
struct ActorFinalizationHooks {
    void* context = nullptr;
    std::uint8_t (*has_behavior)(void*, ActorFinalizationState&, std::int32_t) = nullptr;
    void (*model_position_08)(void*, ActorFinalizationState&) = nullptr;
    void (*effect_458186)(void*, ActorFinalizationState&) = nullptr;
};
// Terminal block 0x42E58C..0x42E673 in actor update 0x42E2BF.
// Model positioning and effect geometry remain explicit callback boundaries.
// Callbacks may mutate the view; subsequent reads preserve original call order.
// A valid current resource/clip and valid sample coordinates are required.
void finalize_actor_update(ActorFinalizationState&, const WorldSampleTable&,
                           const ActorFinalizationHooks&);
} // namespace lemmings
