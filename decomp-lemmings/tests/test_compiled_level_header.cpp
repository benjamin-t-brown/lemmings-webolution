#include "lemmings/compiled_level_header.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>

namespace {
void check(bool value) {
    if (!value) throw std::runtime_error("compiled level header mismatch");
}
}

int main() {
    // Header of installed COMPILED\MM_CLIMB_002.LVL, pinned in the
    // archive evidence note. No complete asset is included in the test.
    std::array<std::uint8_t, 40> bytes = {
        0x08, 0x00, 0x00, 0x00, 'L', 'e', 'v', 'e', 'l', ' ', '2', '3',
        0x63, 0x00, 0x00, 0x00, 0x92, 0xc6, 0x12, 0x00,
        0x92, 0xc6, 0x2d, 0x00, 0x00, 0xb0, 0x01, 0x00,
        0x00, 0xb0, 0x01, 0x00, 0x4a, 0x28, 0x07, 0x00,
        0x01, 0x00, 0x00, 0x00
    };
    lemmings::LevelByteStream stream{bytes};
    const auto parsed = lemmings::read_compiled_level_header(stream);
    check(parsed.marker_matches && stream.cursor == 40);
    check(parsed.fields == std::array<std::uint32_t, 6>{
        99, 1230482, 2999954, 110592, 110592, 469066});
    check(parsed.flags == std::array<bool, 4>{true, false, false, false});
    check(parsed.first_field_times_58 == 5742);

    bytes[4] = 'X';
    bytes[37] = 0xFF;
    stream.cursor = 0;
    const auto altered = lemmings::read_compiled_level_header(stream);
    check(!altered.marker_matches && altered.flags[1] && stream.cursor == 40);

    lemmings::LevelByteStream short_stream{
        std::span<const std::uint8_t>(bytes.data(), 39)};
    bool rejected = false;
    try { lemmings::read_compiled_level_header(short_stream); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected);
}
