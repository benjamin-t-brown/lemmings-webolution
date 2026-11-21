#pragma once

#include "lemmings/level_name_tables.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace lemmings {

// Serialized read path of 0x0043D0F0 and following world fields.
// leading_value is the coordinate mesh height (+0x28); first_blocks stores
// two radial layers of 32 triples per row. See world_coordinates.hpp.
// Other field meanings and the remaining allocation layout are unresolved.
struct WorldGeometryPrefix {
    std::uint32_t leading_value = 0;
    std::uint64_t following_double_bits = 0;
    using Triple = std::array<std::uint64_t, 3>;
    using TripleBlock = std::array<Triple, 64>;
    using DoubleBlock = std::array<std::uint64_t, 32>;
    std::vector<TripleBlock> first_blocks;
    std::vector<DoubleBlock> second_blocks;
    std::uint32_t following_class_leading_value = 0;
    ClassOneRecord following_class_one;
    std::uint32_t following_value = 0;
    std::uint64_t following_double_bits_2 = 0;
    std::vector<std::uint32_t> first_integer_array;
    std::vector<std::array<std::uint32_t, 3>> integer_triples;
    std::vector<std::uint32_t> second_integer_array;
    std::vector<std::uint8_t> byte_array;
    std::array<std::uint8_t, 6144> fixed_bytes{};
    std::string first_name;
    std::string second_name;
    struct TailRecord {
        std::uint32_t value = 0;
        std::uint16_t first_short = 0;
        std::uint16_t second_short = 0;
        std::array<std::uint64_t, 5> double_bits{};
    };
    std::array<std::vector<TailRecord>, 32> tail_lists;
};

WorldGeometryPrefix read_world_geometry_prefix(LevelByteStream& stream);

} // namespace lemmings
