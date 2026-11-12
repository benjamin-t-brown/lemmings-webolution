#include "lemmings/actor_shadow_construction.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cstring>
#include <iostream>
#include <random>

namespace {
using namespace lemmings;
template<class T> void read_field(const unsigned char* raw, unsigned offset, T& value) {
    std::memcpy(&value, raw + offset, sizeof(value));
}
template<class T> void write_field(unsigned char* raw, unsigned offset, const T& value) {
    std::memcpy(raw + offset, &value, sizeof(value));
}
void decode(const unsigned char* raw, ActorShadow& shadow) {
    read_field(raw, 0, shadow.vertices_000);
    read_field(raw, 0x120, shadow.mode_120);
    read_field(raw, 0x3a8, shadow.actor_3a8);
    read_field(raw, 0x3ac, shadow.flag_3ac);
    read_field(raw, 0x3ad, shadow.visible_3ad);
    read_field(raw, 0x3b0, shadow.position_3b0);
    read_field(raw, 0x3c8, shadow.intensity_3c8);
    for (unsigned group = 0; group < 5; ++group) {
        const unsigned offset = 0x128 + group * 0x80;
        auto& metadata = shadow.quad_metadata_128[group];
        read_field(raw, offset, metadata.color_00);
        read_field(raw, offset + 0x18, metadata.alpha_18);
        for (unsigned corner = 0; corner < 4; ++corner) {
            read_field(raw, offset + 0x20 + corner * 0x18, metadata.vertex_indices_20[corner]);
            read_field(raw, offset + 0x28 + corner * 0x18, shadow.uv_128[group][corner]);
        }
    }
}
void encode(unsigned char* raw, const ActorShadow& shadow) {
    write_field(raw, 0, shadow.vertices_000);
    write_field(raw, 0x120, shadow.mode_120);
    write_field(raw, 0x3a8, shadow.actor_3a8);
    write_field(raw, 0x3ac, shadow.flag_3ac);
    write_field(raw, 0x3ad, shadow.visible_3ad);
    write_field(raw, 0x3b0, shadow.position_3b0);
    write_field(raw, 0x3c8, shadow.intensity_3c8);
    for (unsigned group = 0; group < 5; ++group) {
        const unsigned offset = 0x128 + group * 0x80;
        const auto& metadata = shadow.quad_metadata_128[group];
        write_field(raw, offset, metadata.color_00);
        write_field(raw, offset + 0x18, metadata.alpha_18);
        for (unsigned corner = 0; corner < 4; ++corner) {
            write_field(raw, offset + 0x20 + corner * 0x18, metadata.vertex_indices_20[corner]);
            write_field(raw, offset + 0x28 + corner * 0x18, shadow.uv_128[group][corner]);
        }
    }
}
}
int main() {
    static_assert(sizeof(void*) == 4);
    _setmode(_fileno(stdin), _O_BINARY);
    void* image = VirtualAlloc(reinterpret_cast<void*>(0x400000), 0xd0000,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    unsigned blocks = 0;
    std::cin.read(reinterpret_cast<char*>(&blocks), 4);
    if (!std::cin || blocks > 64) return 2;
    for (unsigned i = 0; i < blocks; ++i) {
        unsigned address = 0, size = 0;
        std::cin.read(reinterpret_cast<char*>(&address), 4);
        std::cin.read(reinterpret_cast<char*>(&size), 4);
        if (address < 0x400000 || size > 0xd0000 || address - 0x400000 > 0xd0000 - size) return 2;
        std::cin.read(reinterpret_cast<char*>(address), size);
        if (!std::cin) return 2;
    }
    DWORD old = 0;
    if (!VirtualProtect(image, 0xc0000, PAGE_EXECUTE_READ, &old)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xc0000);
    using Construct = void*(__attribute__((thiscall))*)(void*, ShadowActor*);
    const auto construct = reinterpret_cast<Construct>(0x457f40);
    std::mt19937 random(0x457f40);
    constexpr unsigned cases = 10000;
    for (unsigned test = 0; test < cases; ++test) {
        // Eight guard bytes on each side. Uniform fills exercise every possible
        // byte value; random fills also include arbitrary floating bit patterns.
        std::array<unsigned char, 0x3dc> native;
        for (auto& byte : native) byte = static_cast<unsigned char>(test < 256 ? test : random());
        auto expected = native;
        auto* actor = reinterpret_cast<ShadowActor*>(test % 4 == 0 ? 0 : random());
        ActorShadow shadow;
        decode(expected.data() + 8, shadow);
        if (&initialize_actor_shadow(shadow, actor) != &shadow) return 1;
        encode(expected.data() + 8, shadow);
        void* result = construct(native.data() + 8, actor);
        if (result != native.data() + 8 || native != expected) {
            std::cerr << "shadow constructor mismatch case=" << test << '\n';
            for (unsigned i = 0; i < native.size(); ++i) if (native[i] != expected[i]) {
                std::cerr << "object offset=" << std::hex << (i - 8) << '\n'; break;
            }
            return 1;
        }
    }
    std::cout << "shadow constructor: " << cases
        << " complete object/guard comparisons passed; unchanged native chain, no adapters\n";
}
