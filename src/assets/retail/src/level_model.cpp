#include "lemmings/level_model.hpp"

#include <stdexcept>

namespace lemmings {
namespace {
std::uint64_t integer(LevelByteStream& stream, std::size_t bytes) {
    if (stream.cursor > stream.bytes.size() ||
        bytes > stream.bytes.size() - stream.cursor) {
        throw std::invalid_argument("truncated model field");
    }
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < bytes; ++i) {
        value |= static_cast<std::uint64_t>(stream.bytes[stream.cursor++]) << (i * 8);
    }
    return value;
}
std::uint32_t word(LevelByteStream& stream) {
    return static_cast<std::uint32_t>(integer(stream, 4));
}
std::string name(LevelByteStream& stream, int capacity) {
    char value[260]{};
    read_level_string(stream, value, capacity);
    return value;
}
LevelModel read_model(LevelByteStream& stream, unsigned depth) {
    if (depth > 256) throw std::invalid_argument("model nesting exceeds supported depth");
    LevelModel result{};
    result.type = word(stream);
    switch (result.type) {
    case 0:
        result.name = name(stream, 255);
        for (auto& bits : result.double_bits) bits = integer(stream, 8);
        result.object = read_serialized_object(stream);
        result.has_final_name = integer(stream, 1) != 0;
        if (result.has_final_name) result.final_name = name(stream, 128);
        break;
    case 1:
        for (auto& value : result.resource_values) value = word(stream);
        result.name = name(stream, 255);
        break;
    case 2: {
        for (auto& bits : result.type_two_double_bits) bits = integer(stream, 8);
        result.value_230 = word(stream);
        const auto records = word(stream);
        if (records > (stream.bytes.size() - stream.cursor) / 80) {
            throw std::invalid_argument("invalid model record count");
        }
        result.records_1c8.resize(records);
        for (auto& record : result.records_1c8) {
            for (auto& bits : record) bits = integer(stream, 8);
        }
        result.allocation_count = word(stream);
        result.discarded_value = word(stream);
        for (auto& value : result.child_names) value = name(stream, 128);
        const auto children = word(stream);
        if (children > (stream.bytes.size() - stream.cursor) / 4) {
            throw std::invalid_argument("invalid child model count");
        }
        result.children.reserve(children);
        for (std::uint32_t i = 0; i < children; ++i) {
            result.children.push_back(read_model(stream, depth + 1));
        }
        break;
    }
    default:
        throw std::invalid_argument("unrecovered model type");
    }
    return result;
}
}
LevelModel read_level_model(LevelByteStream& stream) {
    return read_model(stream, 0);
}
} // namespace lemmings
