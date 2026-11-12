#include "lemmings/level_stream.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>

namespace {
void check(bool value) {
    if (!value) throw std::runtime_error("level string read mismatch");
}
}

int main() {
    constexpr std::array<std::uint8_t, 12> bytes =
        {8, 0, 0, 0, 'L', 'e', 'v', 'e', 'l', ' ', '2', '3'};
    lemmings::LevelByteStream stream{bytes};
    std::array<char, 12> output{};
    lemmings::read_level_string(stream, output.data(), 9);
    check(stream.cursor == bytes.size());
    check(output[0] == 'L' && output[7] == '3' && output[8] == 0);

    stream.cursor = 0;
    output.fill('x');
    lemmings::read_level_string(stream, output.data(), 4);
    check(stream.cursor == 4 && output[0] == 0 && output[1] == 'x');

    stream.cursor = 0;
    output.fill('x');
    lemmings::read_level_string(stream, output.data(), 8);
    check(stream.cursor == bytes.size() && output[6] == '2' && output[7] == 0);

    stream.cursor = bytes.size() - 3;
    bool rejected = false;
    try { lemmings::read_level_string(stream, output.data(), 9); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected);
}
