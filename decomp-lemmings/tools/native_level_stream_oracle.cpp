// Compare the original level-string reader with an in-memory read adapter.
#include "lemmings/level_stream.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

namespace {
constexpr std::uintptr_t image_base = 0x00400000;
constexpr std::size_t image_size = 0x90000;
using NativeRead = void (__attribute__((thiscall)) *)(void*, char*, int);

struct MemoryStream {
    const std::uint8_t* data;
    std::size_t size;
    std::size_t cursor;
};

std::uint32_t __attribute__((thiscall)) read_adapter(
    MemoryStream* stream, std::uint32_t count, char* destination) {
    const auto available = stream->size - stream->cursor;
    const auto copied = std::min<std::size_t>(count, available);
    std::memcpy(destination, stream->data + stream->cursor, copied);
    stream->cursor += copied;
    return static_cast<std::uint32_t>(copied);
}

template<std::size_t Size>
bool read_at(std::uintptr_t address) {
    std::array<char, Size> bytes{};
    std::cin.read(bytes.data(), bytes.size());
    if (std::cin.gcount() != static_cast<std::streamsize>(bytes.size())) return false;
    std::memcpy(reinterpret_cast<void*>(address), bytes.data(), bytes.size());
    return true;
}
} // namespace

int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    void* image = VirtualAlloc(reinterpret_cast<void*>(image_base), image_size,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(image_base)) return 3;
    if (!read_at<30>(0x00402540) || !read_at<30>(0x00402560) ||
        !read_at<135>(0x0047D940) || !read_at<29>(0x0047DAC8)) {
        std::cerr << "expected verified reader bytes on stdin\n";
        return 2;
    }
    auto* call = reinterpret_cast<std::uint8_t*>(0x0047EEF7);
    call[0] = 0xE9;
    const auto target = reinterpret_cast<std::uintptr_t>(&read_adapter);
    const std::int32_t displacement = static_cast<std::int32_t>(
        target - (0x0047EEF7 + 5));
    std::memcpy(call + 1, &displacement, 4);
    DWORD old_protection = 0;
    if (!VirtualProtect(image, image_size, PAGE_EXECUTE_READ,
                        &old_protection)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, image_size);
    auto native = reinterpret_cast<NativeRead>(0x0047D940);

    std::mt19937 random(0x47D940);
    for (int trial = 0; trial != 5000; ++trial) {
        const std::int32_t length = static_cast<std::int32_t>(random() % 290);
        const int capacity = 1 + static_cast<int>(random() % 270);
        std::vector<std::uint8_t> bytes(4 + length);
        std::memcpy(bytes.data(), &length, 4);
        for (int i = 0; i != length; ++i) bytes[4 + i] =
            static_cast<std::uint8_t>(random());
        MemoryStream stream{bytes.data(), bytes.size(), 0};
        lemmings::LevelByteStream recovered_stream{bytes};
        std::array<char, 300> original{};
        std::array<char, 300> recovered{};
        original.fill(static_cast<char>(0xA5));
        recovered.fill(static_cast<char>(0xA5));
        native(&stream, original.data(), capacity);
        lemmings::read_level_string(recovered_stream, recovered.data(), capacity);
        if (original != recovered || stream.cursor != recovered_stream.cursor) {
            std::cerr << "level string mismatch at trial " << trial
                      << " length=" << length << " capacity=" << capacity
                      << " native cursor=" << stream.cursor
                      << " recovered cursor=" << recovered_stream.cursor << '\n';
            return 1;
        }
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native level string passed 5000 comparisons\n";
}
