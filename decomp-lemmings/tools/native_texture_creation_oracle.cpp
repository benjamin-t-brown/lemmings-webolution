#include "../tests/texture_creation_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>

namespace {
using namespace texture_creation_test;
Fixture* active;
Fixture* fatal_expected;
bool resource_allocation;
const char* __cdecl substring(const char* a, const char* b) { return std::strstr(a,b); }
const char* __cdecl last_character(const char* a, int b) { return std::strrchr(a,b); }
struct Sync {
    Sync() { std::memcpy(active->scratch.data(), reinterpret_cast<void*>(0x58d5b8), active->scratch.size()); }
    ~Sync() {
        std::memcpy(reinterpret_cast<void*>(0x58d5b8), active->scratch.data(), active->scratch.size());
        *reinterpret_cast<std::uint8_t*>(0x5cd5f2) = active->quality;
    }
};
void* __cdecl allocate(std::uint32_t size) {
    Sync sync;
    if (resource_allocation) { resource_allocation = false; return active->allocate_resource(size); }
    return active->allocate_alpha(size);
}
void* __attribute__((thiscall)) construct_file(TextureCreationFile* f, const char* p) {
    Sync sync; active->construct_file(*f,p); return f;
}
std::uint32_t __attribute__((thiscall)) open_file(TextureCreationFile* f, std::uint32_t a, std::uint32_t b) {
    Sync sync; return active->open(*f,a,b & 255u);
}
void __attribute__((thiscall)) read_file(TextureCreationFile* f, void* p, std::uint32_t n) {
    Sync sync; active->read(*f,p,n);
}
void __attribute__((thiscall)) close_file(TextureCreationFile* f) { Sync sync; active->close(*f); }
void __attribute__((thiscall)) destroy_file(TextureCreationFile* f) { Sync sync; active->destroy(*f); }
void __attribute__((thiscall)) load(TextureBitmapBuffer* b, const char* p) { Sync sync; active->load(*b,p); }
void __cdecl free_data(void* p) { Sync sync; active->free_data(p); }
void* WINAPI create_dc(void* p) { Sync sync; return active->create_dc(p); }
void* WINAPI create_dib(void* d, const void* i, std::uint32_t u, void** p, void* s, std::uint32_t o) {
    Sync sync; return active->create_dib(d,i,u,p,s,o);
}
int WINAPI delete_dc(void* d) { Sync sync; active->delete_dc(d); return 1; }
void __attribute__((thiscall)) graphics(TextureCreationResource* a, TextureCreationResource* b) {
    Sync sync; active->initialize(*a,*b);
}
void __cdecl report(const char* s, std::uint32_t n, const char* p) { Sync sync; active->report(s,n,p); }
[[noreturn]] void __cdecl terminate(int status) {
    { Sync sync; active->terminate(status); }
    const bool okay = fatal_expected && active->equivalent(*fatal_expected);
    std::cout << (okay ? "fatal alpha-open trace matches " : "fatal alpha-open mismatch ")
              << active->trial << '\n' << std::flush;
    std::_Exit(okay ? 0 : 1);
}
void jump(std::uintptr_t a, void* p) {
    auto* b = reinterpret_cast<std::uint8_t*>(a); b[0] = 0xe9;
    const auto delta = std::uint32_t(reinterpret_cast<std::uintptr_t>(p) - a - 5);
    std::memcpy(b + 1, &delta, 4);
}
struct Manager {
    void** vtable;
    std::array<std::byte, 0x38> unknown;
    ResourceCacheEntry* data_3c;
};
unsigned run(Fixture& f) {
    active = &f; resource_allocation = f.operation == 0 || f.operation == 4;
    std::memcpy(reinterpret_cast<void*>(0x58d5b8), f.scratch.data(), f.scratch.size());
    *reinterpret_cast<std::uint8_t*>(0x5cd5f2) = f.quality;
    using Factory = TextureCreationResource*(__attribute__((thiscall))*)(void*, const char*);
    using Constructor = TextureCreationResource*(__attribute__((thiscall))*)(TextureCreationResource*, const char*, std::uint32_t);
    using Bitmap = void(__attribute__((thiscall))*)(TextureCreationResource*, const char*, std::int32_t*, std::int32_t*);
    using Quality = std::uint8_t(__cdecl*)(const char*);
    using Index = TextureCreationResource*(__attribute__((thiscall))*)(Manager*, std::int32_t);
    unsigned result = 0;
    switch (f.operation) {
    case 0: result = reinterpret_cast<Factory>(0x4743dd)(nullptr,f.name()) != nullptr; break;
    case 1: result = reinterpret_cast<Constructor>(0x47218a)(&f.resource,f.name(),f.flags) == &f.resource; break;
    case 2: reinterpret_cast<Bitmap>(0x472490)(&f.resource,f.name(),&f.resource.width_14,
        f.trial % 2 ? &f.resource.width_14 : &f.resource.height_18); break;
    case 3: result = reinterpret_cast<Quality>(0x47244b)(f.name()); break;
    default: {
        void* vtable[6]{}; vtable[5] = reinterpret_cast<void*>(0x4743dd);
        Manager manager{vtable, {}, f.entries.data()};
        result = reinterpret_cast<Index>(0x471720)(&manager,1) != nullptr; break;
    }
    }
    std::memcpy(f.scratch.data(), reinterpret_cast<void*>(0x58d5b8), f.scratch.size());
    return result;
}
}
int main(int argc, char** argv) {
    static_assert(sizeof(void*) == 4 && sizeof(TextureCreationResource) == 40);
    static_assert(offsetof(TextureCreationResource, flags_24) == 0x24);
    static_assert(offsetof(Manager, data_3c) == 0x3c);
    _setmode(_fileno(stdin), _O_BINARY);
    auto* image = VirtualAlloc(reinterpret_cast<void*>(0x400000), 0x200000,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) { std::cerr << "mapping failed " << GetLastError() << '\n'; return 3; }
    std::uint32_t count; std::cin.read(reinterpret_cast<char*>(&count),4);
    while (count--) {
        std::uint32_t address, size;
        std::cin.read(reinterpret_cast<char*>(&address),4); std::cin.read(reinterpret_cast<char*>(&size),4);
        if (address < 0x400000 || size > 0x200000 || address > 0x600000 - size) return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
    }
    if (!std::cin) return 2;
    jump(0x49c1a8,reinterpret_cast<void*>(&allocate));
    jump(0x49c750,reinterpret_cast<void*>(&std::strcpy));
    jump(0x49c760,reinterpret_cast<void*>(&std::strcat));
    jump(0x49ca6a,reinterpret_cast<void*>(&std::sprintf));
    jump(0x49ce20,reinterpret_cast<void*>(&substring));
    jump(0x49d4c0,reinterpret_cast<void*>(&last_character));
    jump(0x49d010,reinterpret_cast<void*>(&std::memcpy));
    jump(0x47db73,reinterpret_cast<void*>(&construct_file));
    jump(0x47e40d,reinterpret_cast<void*>(&open_file));
    jump(0x43b5b0,reinterpret_cast<void*>(&read_file));
    jump(0x47e844,reinterpret_cast<void*>(&close_file));
    jump(0x47dc28,reinterpret_cast<void*>(&destroy_file));
    jump(0x45015a,reinterpret_cast<void*>(&load));
    jump(0x49c13d,reinterpret_cast<void*>(&free_data));
    jump(0x472f99,reinterpret_cast<void*>(&graphics));
    jump(0x482a02,reinterpret_cast<void*>(&report));
    jump(0x49d383,reinterpret_cast<void*>(&terminate));
    *reinterpret_cast<void**>(0x4b004c) = reinterpret_cast<void*>(&create_dc);
    *reinterpret_cast<void**>(0x4b0054) = reinterpret_cast<void*>(&create_dib);
    *reinterpret_cast<void**>(0x4b003c) = reinterpret_cast<void*>(&delete_dc);
    DWORD previous;
    if (!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&previous)) return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    if (argc == 3 && std::string(argv[1]) == "--fatal") {
        const auto trial = unsigned(std::strtoul(argv[2],nullptr,10));
        Fixture port(trial,true), original(trial,true);
        try { port.run(); return 1; } catch (const Terminated&) {}
        fatal_expected = &port; run(original); return 1;
    }
    for (unsigned trial = 0; trial < 29160; ++trial) {
        Fixture port(trial), original(trial);
        const auto expected = port.run(), actual = run(original);
        if (actual != expected || !original.equivalent(port)) {
            std::cerr << "texture creation mismatch " << trial << " op " << port.operation << '\n';
            for (unsigned i=0;i<std::min(port.trace.size(),original.trace.size());++i)
                if(port.trace[i]!=original.trace[i]) {std::cerr << "trace "<<i<<" port "<<port.trace[i]<<" original "<<original.trace[i]<<'\n';break;}
            return 1;
        }
    }
    std::cout << "29160 factory/constructor/BMP/quality/indexed traces match unchanged original routines\n";
}
