// Emit the recovered archive index in an unambiguous binary test format.
#include "lemmings/box_archive.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace {
void write_u32(std::uint32_t value) {
    const char bytes[4] = {
        static_cast<char>(value), static_cast<char>(value >> 8),
        static_cast<char>(value >> 16), static_cast<char>(value >> 24)
    };
    std::cout.write(bytes, 4);
}
}

int main(int argc, char** argv) {
    if (argc != 2 && argc != 4) return 2;
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    try {
        const auto index = lemmings::read_box_archive_index(argv[1]);
        if (argc == 4) {
            if (std::string(argv[2]) != "--entry") return 2;
            const auto ordinal = static_cast<std::size_t>(std::stoul(argv[3]));
            const auto bytes = index.read_entry(ordinal);
            std::cout.write(reinterpret_cast<const char*>(bytes.data()),
                            static_cast<std::streamsize>(bytes.size()));
            return std::cout ? 0 : 2;
        }
        write_u32(static_cast<std::uint32_t>(index.entries.size()));
        for (const auto& entry : index.entries) {
            write_u32(static_cast<std::uint32_t>(entry.name.size()));
            std::cout.write(entry.name.data(), entry.name.size());
            write_u32(entry.offset);
            write_u32(entry.size);
        }
        return std::cout ? 0 : 2;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 2;
    }
}
