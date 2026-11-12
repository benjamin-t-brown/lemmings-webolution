#include "../tests/sprite_resource_destruction_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

namespace {
using namespace sprite_destruction_test;
Fixture* active;
void __cdecl free_memory(void* p) { active->free_memory(p); }
void jump(std::uintptr_t address, void* target) {
    auto* bytes = reinterpret_cast<unsigned char*>(address);
    bytes[0] = 0xe9;
    const auto offset = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target) - address - 5);
    std::memcpy(bytes + 1, &offset, 4);
}
unsigned run(Fixture& fixture) {
    active = &fixture;
    using Clear = void(__attribute__((thiscall))*)(void*);
    using Delete = void*(__attribute__((thiscall))*)(void*, std::uint32_t);
    switch (fixture.operation) {
    case 0: reinterpret_cast<Clear>(0x484580)(&fixture.resource.clips_00); break;
    case 1: reinterpret_cast<Clear>(0x4847a0)(&fixture.resource.triple_arrays_10); break;
    case 2: reinterpret_cast<Clear>(0x484520)(&fixture.clips[0].frames_00); break;
    case 3: reinterpret_cast<Clear>(0x484930)(&fixture.frames[0][0].pieces_00); break;
    case 4: return fixture.pointer(reinterpret_cast<Delete>(0x484990)(fixture.clips, fixture.flags));
    case 5: return fixture.pointer(reinterpret_cast<Delete>(0x484a50)(fixture.triples, fixture.flags));
    case 6: return fixture.pointer(reinterpret_cast<Delete>(0x484880)(fixture.frames[0], fixture.flags));
    case 7: return fixture.pointer(reinterpret_cast<Delete>(0x484800)(fixture.pieces[0][0], fixture.flags));
    case 8: reinterpret_cast<Clear>(0x484470)(fixture.clips); break;
    case 9: reinterpret_cast<Clear>(0x47c5d3)(fixture.triples); break;
    case 10: reinterpret_cast<Clear>(0x4848f0)(fixture.frames[0]); break;
    case 11: reinterpret_cast<Clear>(0x484870)(fixture.pieces[0][0]); break;
    case 12: reinterpret_cast<Clear>(0x4850d0)(&fixture.resource); break;
    }
    return 0;
}
} // namespace

int main() {
    static_assert(sizeof(void*) == 4);
    static_assert(sizeof(SpriteDestructionClip) == 152);
    static_assert(sizeof(SpriteDestructionFrame) == 16);
    static_assert(sizeof(SpriteDestructionPiece) == 72);
    static_assert(sizeof(SpriteDestructionTripleArray) == 8);
    static_assert(offsetof(SpriteDestructionClip,name_18) == 0x18);
    static_assert(offsetof(SpriteDestructionTripleArray,data_04) == 4);
    _setmode(_fileno(stdin),_O_BINARY);
    void* image = VirtualAlloc(reinterpret_cast<void*>(0x400000),0x100000,
                               MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    std::uint32_t count;
    std::cin.read(reinterpret_cast<char*>(&count),4);
    while (count--) {
        std::uint32_t address,size;
        std::cin.read(reinterpret_cast<char*>(&address),4);
        std::cin.read(reinterpret_cast<char*>(&size),4);
        if (address < 0x400000 || size > 0x100000 || address > 0x500000-size) return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
    }
    if (!std::cin) return 2;
    jump(0x49c13d,reinterpret_cast<void*>(&free_memory));
    DWORD previous;
    if (!VirtualProtect(image,0x100000,PAGE_EXECUTE_READ,&previous)) return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0x100000);
    for (unsigned trial=0; trial<18720; ++trial) {
        Fixture original(trial),port(trial);
        const auto actual=run(original);
        const auto expected=port.run();
        if (actual!=expected || original.trace!=port.trace || original.snapshot()!=port.snapshot()) {
            std::cerr << "nested sprite cleanup mismatch " << trial << '\n';
            return 1;
        }
    }
    std::cout << "18720 complete nested sprite cleanup traces match original; only free adapted\n";
}
