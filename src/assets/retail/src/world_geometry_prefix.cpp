#include "lemmings/world_geometry_prefix.hpp"

#include <stdexcept>

namespace lemmings {
namespace {
std::uint32_t read_u32(LevelByteStream& stream) {
    if (stream.cursor > stream.bytes.size() ||
        stream.bytes.size() - stream.cursor < 4) {
        throw std::invalid_argument("truncated world geometry field");
    }
    std::uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        value |= static_cast<std::uint32_t>(stream.bytes[stream.cursor++]) << (8 * i);
    }
    return value;
}

std::uint64_t read_u64(LevelByteStream& stream) {
    if (stream.cursor > stream.bytes.size() ||
        stream.bytes.size() - stream.cursor < 8) {
        throw std::invalid_argument("truncated world geometry double");
    }
    std::uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value |= static_cast<std::uint64_t>(stream.bytes[stream.cursor++]) << (8 * i);
    }
    return value;
}

std::uint16_t read_u16(LevelByteStream& stream) {
    if (stream.cursor > stream.bytes.size() ||
        stream.bytes.size() - stream.cursor < 2) {
        throw std::invalid_argument("truncated world short");
    }
    const auto value = static_cast<std::uint16_t>(stream.bytes[stream.cursor]) |
        (static_cast<std::uint16_t>(stream.bytes[stream.cursor + 1]) << 8);
    stream.cursor += 2;
    return value;
}

std::vector<std::uint32_t> read_u32_array(LevelByteStream& stream) {
    const auto count = read_u32(stream);
    if (count > (stream.bytes.size() - stream.cursor) / 4) {
        throw std::invalid_argument("invalid world integer count");
    }
    std::vector<std::uint32_t> values(count);
    for (auto& value : values) value = read_u32(stream);
    return values;
}

std::string read_name(LevelByteStream& stream) {
    char text[255]{};
    read_level_string(stream, text, 255);
    return text;
}
}

WorldGeometryPrefix read_world_geometry_prefix(LevelByteStream& stream) {
    WorldGeometryPrefix result{};
    result.leading_value = read_u32(stream);
    result.following_double_bits = read_u64(stream);
    const auto first_count = read_u32(stream);
    if (first_count > (stream.bytes.size() - stream.cursor) / 1536) {
        throw std::invalid_argument("invalid world triple block count");
    }
    result.first_blocks.resize(first_count);
    for (auto& block : result.first_blocks) {
        for (auto& triple : block) {
            for (auto& bits : triple) bits = read_u64(stream);
        }
    }
    const auto second_count = read_u32(stream);
    if (second_count > (stream.bytes.size() - stream.cursor) / 256) {
        throw std::invalid_argument("invalid world double block count");
    }
    result.second_blocks.resize(second_count);
    for (auto& block : result.second_blocks) {
        for (auto& bits : block) bits = read_u64(stream);
    }
    result.following_class_leading_value = read_u32(stream);
    result.following_class_one = read_class_one_record(stream);
    result.following_value = read_u32(stream);
    result.following_double_bits_2 = read_u64(stream);
    result.first_integer_array = read_u32_array(stream);
    const auto triple_count = read_u32(stream);
    if (triple_count > (stream.bytes.size() - stream.cursor) / 12) {
        throw std::invalid_argument("invalid world integer triple count");
    }
    result.integer_triples.resize(triple_count);
    for (auto& triple : result.integer_triples) {
        for (auto& value : triple) value = read_u32(stream);
    }
    result.second_integer_array = read_u32_array(stream);
    const auto byte_count = read_u32(stream);
    if (byte_count > stream.bytes.size() - stream.cursor) {
        throw std::invalid_argument("invalid world byte count");
    }
    result.byte_array.assign(stream.bytes.begin() + stream.cursor,
                             stream.bytes.begin() + stream.cursor + byte_count);
    stream.cursor += byte_count;
    if (stream.bytes.size() - stream.cursor < result.fixed_bytes.size()) {
        throw std::invalid_argument("truncated world fixed bytes");
    }
    for (auto& value : result.fixed_bytes) value = stream.bytes[stream.cursor++];
    result.first_name = read_name(stream);
    result.second_name = read_name(stream);
    for (auto& list : result.tail_lists) {
        const auto count = read_u32(stream);
        if (count > (stream.bytes.size() - stream.cursor) / 48) {
            throw std::invalid_argument("invalid world tail count");
        }
        list.resize(count);
        for (auto& record : list) {
            record.value = read_u32(stream);
            record.first_short = read_u16(stream);
            record.second_short = read_u16(stream);
            for (auto& bits : record.double_bits) bits = read_u64(stream);
        }
    }
    return result;
}

} // namespace lemmings
