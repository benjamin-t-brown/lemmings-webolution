#include "lemmings/sprite_resource.hpp"

#include <bit>
#include <limits>
#include <stdexcept>

namespace lemmings {
namespace {
std::uint64_t integer(LevelByteStream& stream, unsigned bytes) {
    if (stream.cursor > stream.bytes.size() || bytes > stream.bytes.size() - stream.cursor)
        throw std::invalid_argument("truncated sprite field");
    std::uint64_t result = 0;
    for (unsigned i = 0; i < bytes; ++i)
        result |= std::uint64_t(stream.bytes[stream.cursor++]) << (8 * i);
    return result;
}
std::uint32_t count(LevelByteStream& stream, unsigned minimum_bytes) {
    const auto result = static_cast<std::uint32_t>(integer(stream, 4));
    if (result > std::uint32_t(std::numeric_limits<std::int32_t>::max()) ||
        result > (stream.bytes.size() - stream.cursor) / minimum_bytes)
        throw std::invalid_argument("invalid sprite array count");
    return result;
}
std::string name(LevelByteStream& stream, int capacity) {
    char buffer[260]{};
    read_level_string(stream, buffer, capacity);
    return buffer;
}
SpritePiece piece(LevelByteStream& stream) { // 0x485CE0
    SpritePiece result;
    for (auto& value : result.values_00) value = static_cast<std::uint32_t>(integer(stream, 4));
    for (auto& value : result.double_bits_10) value = integer(stream, 8);
    for (auto& value : result.values_30) value = static_cast<std::uint32_t>(integer(stream, 4));
    result.texture_name = name(stream, 260);
    return result;
}
}

SpriteResource read_sprite_resource(LevelByteStream& stream) {
    SpriteResource result;
    result.clips.resize(count(stream, 16)); // 0x484160 -> 0x485800
    for (auto& clip : result.clips) {
        clip.frames.resize(count(stream, 4)); // 0x485840 -> 0x485A80
        for (auto& frame : clip.frames) {
            const auto pieces = count(stream, 68); // 0x485AA0
            frame.pieces.reserve(pieces);
            for (std::uint32_t i = 0; i < pieces; ++i) frame.pieces.push_back(piece(stream));
        }
        clip.name = name(stream, 128); // 0x48252D
        clip.speed_bits_10 = integer(stream, 8);
    }
    result.triple_arrays.resize(count(stream, 4)); // 0x4841E0 -> 0x47C643
    for (auto& array : result.triple_arrays) {
        array.type = static_cast<std::uint32_t>(integer(stream, 4));
        const unsigned triples = array.type == 0 ? 16 : array.type == 1 ? 256 : 0;
        if (triples > (stream.bytes.size() - stream.cursor) / 24)
            throw std::invalid_argument("truncated sprite triple array");
        array.triples.resize(triples);
        for (auto& triple : array.triples)
            for (auto& value : triple) value = integer(stream, 8); // 0x480BB0
    }
    return result;
}

SpriteResource parse_sprite_resource(std::span<const std::uint8_t> bytes) {
    LevelByteStream stream{bytes};
    auto result = read_sprite_resource(stream);
    if (stream.cursor != bytes.size()) throw std::invalid_argument("trailing sprite bytes");
    return result;
}

std::vector<AnimationClip> sprite_animation_clips(const SpriteResource& sprite) {
    std::vector<AnimationClip> result;
    result.reserve(sprite.clips.size());
    for (const auto& clip : sprite.clips) {
        if (clip.frames.size() > std::size_t(std::numeric_limits<std::int32_t>::max()))
            throw std::invalid_argument("sprite frame count exceeds original integer range");
        result.push_back({static_cast<std::int32_t>(clip.frames.size()),
                          std::bit_cast<double>(clip.speed_bits_10), clip.name});
    }
    return result;
}
} // namespace lemmings
