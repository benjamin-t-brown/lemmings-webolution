// Batch probe for checking the recovered header reader against installed LVLs.
#include "lemmings/compiled_level_header.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <span>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

int main() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
#endif
    std::array<std::uint8_t, 40> bytes{};
    while (std::cin.read(reinterpret_cast<char*>(bytes.data()), bytes.size())) {
        lemmings::LevelByteStream stream{std::span<const std::uint8_t>(bytes)};
        const auto header = lemmings::read_compiled_level_header(stream);
        std::cout << header.marker_matches;
        for (const auto field : header.fields) std::cout << ' ' << field;
        for (const auto flag : header.flags) std::cout << ' ' << flag;
        std::cout << ' ' << header.first_field_times_58
                  << ' ' << stream.cursor << '\n';
    }
    if (!std::cin.eof() || std::cin.gcount() != 0) {
        std::cerr << "input must contain complete 40-byte headers\n";
        return 2;
    }
}
