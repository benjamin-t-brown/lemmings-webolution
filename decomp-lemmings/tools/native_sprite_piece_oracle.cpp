// Execute the unchanged SPR piece reader with stream/texture lookup adapters.
#include "lemmings/sprite_resource.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

namespace {
struct Stream {const std::uint8_t* data; std::size_t size, cursor;};
std::string texture;
void* destination = nullptr;
unsigned lookups = 0;
std::uint32_t __attribute__((thiscall)) stream_read(Stream* stream, std::uint32_t count, void* target) {
    const auto copied = std::min<std::size_t>(count, stream->size - stream->cursor);
    std::memcpy(target, stream->data + stream->cursor, copied);
    stream->cursor += copied;
    return copied;
}
void __cdecl lookup(const char* name, void* output) {
    texture = name; destination = output; ++lookups;
}
void jump(std::uintptr_t address, void* target) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(address);
    bytes[0] = 0xe9;
    const auto delta = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target) - address - 5);
    std::memcpy(bytes + 1, &delta, 4);
}
template<class T> void append(std::vector<std::uint8_t>& bytes, T value) {
    const auto offset = bytes.size(); bytes.resize(offset + sizeof(value));
    std::memcpy(bytes.data() + offset, &value, sizeof(value));
}
std::uint64_t loaded(std::uint64_t value) {
    if ((value & 0x7ff0000000000000ULL) == 0x7ff0000000000000ULL &&
        (value & 0xfffffffffffffULL) != 0) value |= 0x8000000000000ULL;
    return value;
}
template<class T> void put(std::array<std::uint8_t, 0x48>& bytes, unsigned offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(value));
}
}
int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    auto* image = VirtualAlloc(reinterpret_cast<void*>(0x400000), 0xa0000,
                              MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    std::uint32_t count = 0; std::cin.read(reinterpret_cast<char*>(&count), 4);
    for (unsigned i = 0; i < count; ++i) {
        std::uint32_t address = 0, size = 0;
        std::cin.read(reinterpret_cast<char*>(&address), 4);
        std::cin.read(reinterpret_cast<char*>(&size), 4);
        if (address < 0x400000 || address > 0x4a0000 || size > 0x4a0000 - address) return 2;
        std::cin.read(reinterpret_cast<char*>(address), size);
    }
    if (!std::cin) return 2;
    jump(0x47eef7, reinterpret_cast<void*>(&stream_read));
    jump(0x474519, reinterpret_cast<void*>(&lookup));
    DWORD old = 0;
    if (!VirtualProtect(image, 0xa0000, PAGE_EXECUTE_READ, &old)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xa0000);
    using Native = void (__attribute__((thiscall)) *)(void*, void*);
    std::mt19937_64 random(0x485ce0);
    const std::uint64_t special[] = {0, 0x8000000000000000ULL, 1, 0x8000000000000001ULL,
        0x7fefffffffffffffULL, 0xffefffffffffffffULL, 0x7ff0000000000000ULL,
        0xfff0000000000000ULL, 0x7ff0000000000001ULL, 0xfff8000000001234ULL};
    for (unsigned trial = 0; trial < 20000; ++trial) {
        std::vector<std::uint8_t> bytes;
        for (unsigned i = 0; i < 4; ++i) append(bytes, static_cast<std::uint32_t>(random()));
        for (unsigned i = 0; i < 4; ++i) append(bytes, trial < 10 ? special[(trial + i) % 10] : random());
        for (unsigned i = 0; i < 4; ++i) append(bytes, static_cast<std::uint32_t>(random()));
        const auto length = static_cast<std::uint32_t>(trial < 5 ?
            std::array<unsigned, 5>{0, 1, 259, 260, 261}[trial] : random() % 271);
        append(bytes, length);
        if (length <= 260)
            for (unsigned i = 0; i < length; ++i) bytes.push_back(static_cast<std::uint8_t>(random()));
        std::vector<std::uint8_t> wrapped;
        append(wrapped, std::uint32_t{1}); // one clip
        append(wrapped, std::uint32_t{1}); // one frame
        append(wrapped, std::uint32_t{1}); // one piece
        wrapped.insert(wrapped.end(), bytes.begin(), bytes.end());
        append(wrapped, std::uint32_t{0}); // clip name
        append(wrapped, std::uint64_t{0}); // speed
        append(wrapped, std::uint32_t{0}); // triple arrays
        const auto source = lemmings::parse_sprite_resource(wrapped);
        const auto& piece = source.clips[0].frames[0].pieces[0];
        std::array<std::uint8_t, 0x48> original, expected;
        original.fill(0xa5); expected.fill(0xa5);
        for (unsigned i = 0; i < 4; ++i) {
            put(expected, i * 4, piece.values_00[i]);
            put(expected, 0x10 + i * 8, loaded(piece.double_bits_10[i]));
            put(expected, 0x30 + i * 4, piece.values_30[i]);
        }
        Stream stream{bytes.data(), bytes.size(), 0};
        texture.clear(); destination = nullptr; lookups = 0;
        reinterpret_cast<Native>(0x485ce0)(original.data(), &stream);
        if (original != expected || texture != piece.texture_name || stream.cursor != bytes.size() ||
            destination != original.data() + 0x40 || lookups != 1) {
            std::cerr << "sprite piece mismatch trial " << trial << " cursor " << stream.cursor << '\n';
            return 1;
        }
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native sprite piece reader passed 20000 field/name/cursor comparisons\n";
}
