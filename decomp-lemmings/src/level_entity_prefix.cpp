#include "lemmings/level_entity_prefix.hpp"

#include <bit>
#include <stdexcept>
#include <utility>

namespace lemmings {
namespace {
std::uint64_t read_integer(LevelByteStream& stream, std::size_t width) {
    if (stream.cursor > stream.bytes.size() ||
        width > stream.bytes.size() - stream.cursor) {
        throw std::invalid_argument("truncated entity field");
    }
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < width; ++i) {
        value |= static_cast<std::uint64_t>(stream.bytes[stream.cursor++]) << (8 * i);
    }
    return value;
}

std::uint32_t read_u32(LevelByteStream& stream) {
    return static_cast<std::uint32_t>(read_integer(stream, 4));
}

bool read_flag(LevelByteStream& stream) {
    return read_integer(stream, 1) != 0;
}

std::uint32_t read_count(LevelByteStream& stream, std::size_t minimum_bytes) {
    const auto count = read_u32(stream);
    if (count > (stream.bytes.size() - stream.cursor) / minimum_bytes) {
        throw std::invalid_argument("invalid entity record count");
    }
    return count;
}

std::string read_name(LevelByteStream& stream, int capacity) {
    char bytes[128]{};
    read_level_string(stream, bytes, capacity);
    return bytes;
}

LevelEntityPrefix::ActorRecord read_actor(LevelByteStream& stream, bool subclass) {
    LevelEntityPrefix::ActorRecord actor{};
    for (auto& bits : actor.position_bits) bits = read_integer(stream, 8);
    actor.field_20_bits = read_integer(stream, 8);
    actor.field_30_bits = read_integer(stream, 8);
    actor.flag_38 = read_flag(stream);
    actor.flag_28 = read_flag(stream);
    actor.id = std::bit_cast<std::int16_t>(
        static_cast<std::uint16_t>(read_integer(stream, 2)));
    for (auto& value : actor.values_40_44) value = read_u32(stream);
    for (auto& flag : actor.flags_370_48_49_4a) flag = read_flag(stream);
    actor.value_4c = read_u32(stream);
    actor.flag_5c = read_flag(stream);
    actor.flag_5f = read_flag(stream);
    for (auto& bits : actor.pairs_68_350_360) bits = read_integer(stream, 8);
    if (subclass) actor.subclass_flag_3a4 = read_flag(stream);
    return actor;
}
}

LevelEntityPrefix read_level_entity_prefix(LevelByteStream& stream) {
    LevelEntityPrefix result{};
    result.registry_size = read_u32(stream);
    const auto groups = read_count(stream, 13);
    result.groups.reserve(groups);
    for (std::uint32_t i = 0; i < groups; ++i) {
        LevelEntityPrefix::Group group{};
        group.flag = read_flag(stream);
        const auto records = read_count(stream, 44);
        group.records.resize(records);
        for (auto& record : group.records) {
            record.leading_double_bits = read_integer(stream, 8);
            record.value = read_u32(stream);
            for (auto& bits : record.double_bits) bits = read_integer(stream, 8);
        }
        const auto integers = read_count(stream, 4);
        group.integer_values.resize(integers);
        for (auto& value : group.integer_values) value = read_u32(stream);
        group.name = read_name(stream, 128);
        result.groups.push_back(std::move(group));
    }
    const auto pairs = read_count(stream, 9);
    result.name_pairs.reserve(pairs);
    for (std::uint32_t i = 0; i < pairs; ++i) {
        LevelEntityPrefix::NamePair pair{};
        pair.flag = read_flag(stream);
        pair.first = read_name(stream, 64);
        pair.second = read_name(stream, 64);
        result.name_pairs.push_back(std::move(pair));
    }
    const auto actors = read_count(stream, 110);
    result.actors.reserve(actors);
    for (std::uint32_t i = 0; i < actors; ++i) {
        result.actors.push_back(read_actor(stream, false));
    }
    const auto subclass_actors = read_count(stream, 111);
    result.subclass_actors.reserve(subclass_actors);
    for (std::uint32_t i = 0; i < subclass_actors; ++i) {
        result.subclass_actors.push_back(read_actor(stream, true));
    }
    return result;
}

} // namespace lemmings
