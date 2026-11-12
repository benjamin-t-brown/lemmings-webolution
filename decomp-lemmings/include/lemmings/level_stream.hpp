#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace lemmings {

struct LevelByteStream {
    std::span<const std::uint8_t> bytes;
    std::size_t cursor = 0;
};

// Source form of 0x0047D940 on a complete in-memory input. `capacity` is
// the original integer buffer limit, not a count inferred from `destination`.
// A declared length larger than capacity writes only an empty string and
// leaves the payload unread, matching the original helper.
void read_level_string(LevelByteStream& stream, char* destination,
                       std::int32_t capacity);

} // namespace lemmings
