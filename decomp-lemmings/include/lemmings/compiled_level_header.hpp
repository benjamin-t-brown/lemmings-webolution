#pragma once

#include "lemmings/level_stream.hpp"

#include <array>
#include <cstdint>

namespace lemmings {

// First 40 bytes of the installed Level 23 records, as read by 0x004432C1.
// The six field meanings are not established. The loader continues even if
// the marker-comparison helper reports a mismatch.
struct CompiledLevelHeader {
    bool marker_matches = false;
    std::array<std::uint32_t, 6> fields{};
    std::array<bool, 4> flags{};
    std::uint32_t first_field_times_58 = 0;
};

CompiledLevelHeader read_compiled_level_header(LevelByteStream& stream);

} // namespace lemmings
