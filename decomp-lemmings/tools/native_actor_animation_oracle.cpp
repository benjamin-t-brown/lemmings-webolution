// Original selection and name lookup bodies; resource bind and model select
// are explicit callback boundaries, not implementations of resource loading.
#include "lemmings/actor_animation.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <array>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

namespace {
std::vector<std::int32_t> events;
std::string selected;
std::uint8_t* actor_pointer;
bool offsets_ok = true;
void bind(void*, std::int32_t id) { events.push_back(1); events.push_back(id); }
void select(void*, const char* name) { events.push_back(2); selected = name; }
void __attribute__((thiscall)) native_bind(void* self, std::int32_t id) {
    offsets_ok &= self == actor_pointer + 0x128;
    bind(nullptr, id);
}
void __attribute__((thiscall)) native_select(void* self, const char* name) {
    offsets_ok &= self == actor_pointer + 0x78;
    select(nullptr, name);
}
template<class T, class B> void put(B& buffer, unsigned offset, T value) {
    std::memcpy(buffer.data() + offset, &value, sizeof(value));
}
void jump(std::uintptr_t address, void* target) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(address);
    bytes[0] = 0xe9;
    const auto delta = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target) - address - 5);
    std::memcpy(bytes + 1, &delta, 4);
}
bool load_blocks() {
    std::uint32_t count = 0;
    std::cin.read(reinterpret_cast<char*>(&count), 4);
    for (unsigned i = 0; i < count; ++i) {
        std::uint32_t address = 0, size = 0;
        std::cin.read(reinterpret_cast<char*>(&address), 4);
        std::cin.read(reinterpret_cast<char*>(&size), 4);
        if (address < 0x400000 || address > 0x4f0000 || size > 0x4f0000 - address) return false;
        std::cin.read(reinterpret_cast<char*>(address), size);
    }
    return static_cast<bool>(std::cin);
}
}

int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    auto* image = VirtualAlloc(reinterpret_cast<void*>(0x400000), 0xf0000,
                              MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    if (!load_blocks()) return 2;
    jump(0x46836d, reinterpret_cast<void*>(&native_bind));
    DWORD old = 0;
    if (!VirtualProtect(image, 0xc0000, PAGE_EXECUTE_READ, &old)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xc0000);

    constexpr unsigned resources_count = 8, clips_count = 6;
    std::array<std::array<lemmings::AnimationClip, clips_count>, resources_count> clips;
    std::array<lemmings::AnimationResource, resources_count> resources;
    std::array<std::array<std::uint8_t, 0x10>, resources_count> raw_resources{};
    std::array<std::array<std::uint8_t, clips_count * 0x98>, resources_count> raw_clips{};
    struct RawOverride {void* resource; std::int32_t id;};
    std::array<RawOverride, resources_count> raw_overrides{};
    std::array<lemmings::ActorAnimationOverride, resources_count> overrides{};
    std::array<std::int32_t, 4> defaults{};
    std::array<std::uint8_t, 0x340> actor{};
    std::array<void*, 13> vtable{};
    vtable[12] = reinterpret_cast<void*>(&native_select);
    put(actor, 0x78, vtable.data());
    actor_pointer = actor.data();
    const char* names[] = {"walk", "fall", "float", "", "Walk", "never", "umbopen"};
    std::mt19937 random(0x42fd25);
    using Native = void (__attribute__((thiscall)) *)(void*, const char*);
    using Contains = std::uint8_t (__attribute__((thiscall)) *)(void*, const char*);
    unsigned lookups = 0;
    for (unsigned trial = 0; trial < 20000; ++trial) {
        for (auto& value : defaults) value = static_cast<std::int32_t>(random());
        std::memcpy(reinterpret_cast<void*>(0x4e335c), defaults.data(), sizeof(defaults));
        for (unsigned r = 0; r < resources_count; ++r) {
            const auto count = random() % (clips_count + 1);
            resources[r].clips = std::span(clips[r]).first(count);
            raw_clips[r].fill(0);
            put(raw_resources[r], 0, count);
            put(raw_resources[r], 12, raw_clips[r].data());
            for (unsigned c = 0; c < count; ++c) {
                clips[r][c].name = names[random() % std::size(names)];
                std::strcpy(reinterpret_cast<char*>(raw_clips[r].data() + c * 0x98 + 0x18), clips[r][c].name.c_str());
            }
            const auto id = static_cast<std::int32_t>(random());
            overrides[r] = {&resources[r], id};
            raw_overrides[r] = {raw_resources[r].data(), id};
        }
        const auto count = random() % (resources_count + 1);
        std::memcpy(reinterpret_cast<void*>(0x4c5b10), &count, 4);
        auto* records = raw_overrides.data();
        std::memcpy(reinterpret_cast<void*>(0x4c5b1c), &records, 4);
        const auto category = static_cast<std::int32_t>(random() % defaults.size());
        put(actor, 0x4c, category);
        const auto* name = names[random() % std::size(names)];
        for (unsigned r = 0; r < resources_count; ++r) {
            if (reinterpret_cast<Contains>(0x48408c)(raw_resources[r].data(), name) !=
                lemmings::contains_animation_clip(resources[r], name)) return 1;
            ++lookups;
        }
        events.clear(); selected.clear();
        lemmings::select_actor_animation(category, defaults, std::span(overrides).first(count),
                                         name, {nullptr, bind, select});
        const auto expected_events = events;
        const auto expected_name = selected;
        events.clear(); selected.clear();
        reinterpret_cast<Native>(0x42fd25)(actor.data(), name);
        if (!offsets_ok || events != expected_events || selected != expected_name) {
            std::cerr << "actor animation selection mismatch at trial " << trial << '\n'; return 1;
        }
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native actor animation passed 20000 selection traces and " << lookups << " contains queries\n";
}
