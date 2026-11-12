#pragma once

#include "lemmings/animation_playback.hpp"
#include "lemmings/level_stream.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace lemmings {

// Serialized portion of the 0x48-byte piece read by 0x485CE0. The original
// resolves texture_name into the runtime object at +0x40; that lookup is omitted.
struct SpritePiece {
    std::array<std::uint32_t, 4> values_00{};
    std::array<std::uint64_t, 4> double_bits_10{};
    std::array<std::uint32_t, 4> values_30{};
    std::string texture_name;
};
struct SpriteFrame {
    std::vector<SpritePiece> pieces;
};
struct SpriteClip {
    std::vector<SpriteFrame> frames; // original array at +0, frame stride 0x10
    std::uint64_t speed_bits_10 = 0;
    std::string name;               // original inline string at +0x18
};
struct SpriteTripleArray {
    std::uint32_t type = 2;
    // 0x47C810: type 0 has 16 triples, type 1 has 256, all others have none.
    std::vector<std::array<std::uint64_t, 3>> triples;
};
struct SpriteResource {
    std::vector<SpriteClip> clips; // original clip stride 0x98
    // Original second array at +0x10; each element owns a sequence of triples.
    // Their runtime meaning is not yet established.
    std::vector<SpriteTripleArray> triple_arrays;
};

// 0x483EF0 and its nested readers. Numeric fields retain the serialized bits;
// names follow 0x47D940's bounded C-string behavior. Rejects truncated arrays
// and unsupported allocation counts instead of reproducing memory corruption.
SpriteResource read_sprite_resource(LevelByteStream& stream);
// Standalone .SPR entry reader; requires consumption through exact EOF.
SpriteResource parse_sprite_resource(std::span<const std::uint8_t> bytes);
// Metadata consumed by the recovered animation playback functions. The caller
// owns this vector and must keep it alive when using AnimationResource::clips.
std::vector<AnimationClip> sprite_animation_clips(const SpriteResource& sprite);

} // namespace lemmings
