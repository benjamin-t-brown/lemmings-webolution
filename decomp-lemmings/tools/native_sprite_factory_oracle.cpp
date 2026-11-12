#include "../tests/sprite_factory_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

namespace {
using namespace sprite_factory_test;
Fixture* active = nullptr;
Fixture* fatal_expected = nullptr;

void* __cdecl allocate(std::uint32_t size) { return active->allocate(size); }
int __cdecl format(char* output, const char* pattern, const char* name) {
    active->format(output, pattern, name);
    return static_cast<int>(std::strlen(output));
}
void* __attribute__((thiscall)) construct_file(SpriteFactoryFile* file, const char* path) {
    active->construct_file(*file, path);
    return file;
}
std::uint32_t __attribute__((thiscall)) open_file(SpriteFactoryFile* file,
                                                std::uint32_t a, std::uint32_t b) {
    return active->open_file(*file, a, b);
}
void __attribute__((thiscall)) read_resource(SpriteFactoryResource* resource,
                                            SpriteFactoryFile* file) {
    active->read_resource(resource, *file);
}
void __attribute__((thiscall)) close_file(SpriteFactoryFile* file) { active->close_file(*file); }
void __attribute__((thiscall)) destroy_file(SpriteFactoryFile* file) { active->destroy_file(*file); }
void __cdecl report(const char* source, std::uint32_t line, const char* path) {
    active->report(source, line, path);
}
[[noreturn]] void __cdecl terminate_process(int status) {
    active->terminating(status);
    const bool match = fatal_expected != nullptr && active->equivalent(*fatal_expected);
    if (match) std::cout << "fatal-open trace matches original: " << active->trial << '\n' << std::flush;
    else std::cerr << "fatal-open trace mismatch\n" << std::flush;
    // Do not unwind the original MSVC SEH chain or return from its exit call.
    std::_Exit(match ? 0 : 1);
}
void __attribute__((thiscall)) clear_clips(SpriteRuntimeArray* array) { active->clear_clips(*array); }
void __attribute__((thiscall)) clear_triples(SpriteRuntimeArray* array) { active->clear_triples(*array); }
void __cdecl free_resource(SpriteFactoryResource* resource) { active->free_resource(resource); }

void jump(std::uintptr_t address, void* target) {
    auto* bytes = reinterpret_cast<unsigned char*>(address);
    bytes[0] = 0xe9;
    const auto displacement = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target) - address - 5);
    std::memcpy(bytes + 1, &displacement, 4);
}
unsigned run(Fixture& fixture) {
    active = &fixture;
    using Create = SpriteFactoryResource*(__attribute__((thiscall))*)(void*, const char*);
    using Construct = SpriteFactoryResource*(__attribute__((thiscall))*)(SpriteFactoryResource*);
    using Array = SpriteRuntimeArray*(__attribute__((thiscall))*)(SpriteRuntimeArray*);
    using Destruct = void(__attribute__((thiscall))*)(SpriteFactoryResource*);
    using Delete = SpriteFactoryResource*(__attribute__((thiscall))*)(SpriteFactoryResource*, std::uint32_t);
    using Destroy = void(__attribute__((thiscall))*)(void*, SpriteFactoryResource*);
    switch (fixture.operation) {
    case 0: {
        auto* result = reinterpret_cast<Create>(0x484eb0)(nullptr, fixture.input_name());
        require(result == (fixture.allocation_null ? nullptr : &fixture.resource));
        return result != nullptr;
    }
    case 1: return reinterpret_cast<Construct>(0x485050)(&fixture.resource) == &fixture.resource;
    case 2: return reinterpret_cast<Array>(0x485210)(&fixture.resource.clips_00) == &fixture.resource.clips_00;
    case 3: return reinterpret_cast<Array>(0x485270)(&fixture.resource.triple_arrays_10) == &fixture.resource.triple_arrays_10;
    case 4: reinterpret_cast<Destruct>(0x4850d0)(&fixture.resource); return 0;
    case 5: return reinterpret_cast<Delete>(0x4850a0)(&fixture.resource, fixture.flags) == &fixture.resource;
    default: reinterpret_cast<Destroy>(0x484fe7)(nullptr, fixture.allocation_null ? nullptr : &fixture.resource); return 0;
    }
}
} // namespace

int main(int argc, char** argv) {
    static_assert(sizeof(void*) == 4);
    static_assert(sizeof(SpriteRuntimeArray) == 16);
    static_assert(sizeof(SpriteFactoryResource) == 32);
    static_assert(sizeof(SpriteFactoryFile) == 308);
    static_assert(offsetof(SpriteFactoryResource, triple_arrays_10) == 0x10);
    _setmode(_fileno(stdin), _O_BINARY);
    void* image = VirtualAlloc(reinterpret_cast<void*>(0x400000), 0x100000,
                              MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    std::uint32_t count;
    std::cin.read(reinterpret_cast<char*>(&count), 4);
    while (count--) {
        std::uint32_t address, size;
        std::cin.read(reinterpret_cast<char*>(&address), 4);
        std::cin.read(reinterpret_cast<char*>(&size), 4);
        if (address < 0x400000 || size > 0x100000 || address > 0x500000 - size) return 2;
        std::cin.read(reinterpret_cast<char*>(address), size);
    }
    if (!std::cin) return 2;
    jump(0x49c1a8, reinterpret_cast<void*>(&allocate));
    jump(0x49ca6a, reinterpret_cast<void*>(&format));
    jump(0x47db73, reinterpret_cast<void*>(&construct_file));
    jump(0x47e40d, reinterpret_cast<void*>(&open_file));
    jump(0x483ef0, reinterpret_cast<void*>(&read_resource));
    jump(0x47e844, reinterpret_cast<void*>(&close_file));
    jump(0x47dc28, reinterpret_cast<void*>(&destroy_file));
    jump(0x482a02, reinterpret_cast<void*>(&report));
    jump(0x49d383, reinterpret_cast<void*>(&terminate_process));
    jump(0x484580, reinterpret_cast<void*>(&clear_clips));
    jump(0x4847a0, reinterpret_cast<void*>(&clear_triples));
    jump(0x49c13d, reinterpret_cast<void*>(&free_resource));
    DWORD previous;
    if (!VirtualProtect(image, 0x100000, PAGE_EXECUTE_READ, &previous)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0x100000);
    if (argc == 3 && std::string(argv[1]) == "--fatal") {
        const auto trial = static_cast<unsigned>(std::strtoul(argv[2], nullptr, 10));
        Fixture port(trial, true), original(trial, true);
        try { port.run(); return 1; } catch (const Terminated&) {}
        fatal_expected = &port;
        run(original);
        return 1;
    }
    for (unsigned trial = 0; trial < 25200; ++trial) {
        Fixture port(trial), original(trial);
        const auto expected = port.run();
        const auto actual = run(original);
        if (expected != actual || !original.equivalent(port)) {
            std::cerr << "sprite factory mismatch " << trial << '\n';
            return 1;
        }
    }
    std::cout << "25200 sprite factory/constructor/destructor traces match original\n";
}
