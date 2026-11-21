#pragma once

#include "lemmings/object_base_prefix.hpp"

namespace lemmings {

// File-read variants selected by 0x00465924. These retain serialized values;
// resource resolution, animation setup and runtime allocation are separate.
struct LevelModel {
    std::uint32_t type = 0;
    // Type zero: 0x00450F57.
    std::string name;
    std::array<std::uint64_t, 3> double_bits{};
    std::optional<SerializedObject> object;
    bool has_final_name = false;
    std::string final_name;
    // Type one: 0x0043CEDF -> 0x00468BF2.
    std::array<std::uint32_t, 4> resource_values{};
    // Type two: 0x00466410.
    std::array<std::uint64_t, 7> type_two_double_bits{};
    std::uint32_t value_230 = 0;
    std::vector<std::array<std::uint64_t, 10>> records_1c8;
    std::uint32_t allocation_count = 0;
    std::uint32_t discarded_value = 0;
    std::array<std::string, 2> child_names;
    std::vector<LevelModel> children;
};

LevelModel read_level_model(LevelByteStream& stream);

} // namespace lemmings
