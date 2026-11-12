#pragma once

#include "lemmings/level_stream.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace lemmings {

// Serialized fields following 0x00437D48 in the Level 23 loader.
struct LevelEntityPrefix {
    std::uint32_t registry_size = 0;
    struct GroupRecord {
        std::uint64_t leading_double_bits = 0;
        std::uint32_t value = 0;
        std::array<std::uint64_t, 4> double_bits{};
    };
    struct Group {
        bool flag = false;
        std::vector<GroupRecord> records;
        std::vector<std::uint32_t> integer_values;
        std::string name;
    };
    std::vector<Group> groups;
    struct NamePair {
        bool flag = false;
        std::string first;
        std::string second;
    };
    std::vector<NamePair> name_pairs;
    struct ActorRecord {
        std::array<std::uint64_t, 3> position_bits{};
        std::uint64_t field_20_bits = 0;
        std::uint64_t field_30_bits = 0;
        bool flag_38 = false;
        bool flag_28 = false;
        std::int16_t id = 0;
        std::array<std::uint32_t, 2> values_40_44{};
        std::array<bool, 4> flags_370_48_49_4a{};
        std::uint32_t value_4c = 0;
        bool flag_5c = false;
        bool flag_5f = false;
        std::array<std::uint64_t, 6> pairs_68_350_360{};
        bool subclass_flag_3a4 = false;
    };
    std::vector<ActorRecord> actors;
    std::vector<ActorRecord> subclass_actors;
};

LevelEntityPrefix read_level_entity_prefix(LevelByteStream& stream);

} // namespace lemmings
