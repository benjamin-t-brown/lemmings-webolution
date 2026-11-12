// Execute the original landing decision and coordinate/grid helpers unchanged.
#include "lemmings/actor_landing.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

namespace {
using namespace lemmings;
struct Event {
    unsigned kind;
    std::int32_t first, second;
    bool operator==(const Event&) const = default;
};
std::vector<Event> trace;
bool valid = true;
std::array<std::uint8_t, 0x30> allocated{};
std::uint32_t allocated_size = 0;
void* expected_actor;
std::int32_t allocated_type() {
    return allocated_size == 0x18 ? 0x1A : allocated_size == 0x30 ? 0x21 : 0x12;
}
void* __cdecl allocate_state(std::uint32_t size) {
    valid &= size == 0x18 || size == 0x30 || size == 0x20;
    allocated_size = size;
    trace.push_back({2, allocated_type(), 0});
    return allocated.data();
}
void* __attribute__((thiscall)) construct_terminal(void* state) {
    valid &= state == allocated.data() && allocated_size == 0x18;
    return state;
}
void* __attribute__((thiscall)) construct_bounce(void* state) {
    valid &= state == allocated.data() && allocated_size == 0x30;
    return state;
}
void* __attribute__((thiscall)) construct_landed(void* state) {
    valid &= state == allocated.data() && allocated_size == 0x20;
    return state;
}
void __attribute__((thiscall)) attach(void* actor, void* state) {
    valid &= actor == expected_actor && state == allocated.data();
    trace.push_back({3, allocated_type(), 0});
}
void __cdecl bounce_log(const char* format, std::int32_t x, std::int32_t y) {
    valid &= format == reinterpret_cast<const char*>(0x4BF38C);
    trace.push_back({1, x, y});
}
void jump(std::uintptr_t address, std::uintptr_t target) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(address);
    bytes[0] = 0xE9;
    const auto delta = static_cast<std::uint32_t>(target - address - 5);
    std::memcpy(bytes + 1, &delta, 4);
}
template<class T, std::size_t Size>
void put(std::array<std::uint8_t, Size>& bytes, std::size_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(value));
}
ActorBehavior source_created;
ActorLandingHooks source_hooks() {
    ActorLandingHooks hooks{};
    hooks.bounce_notice = [](void*, std::int32_t x, std::int32_t y) {
        trace.push_back({1, x, y});
    };
    hooks.create_state = [](void*, ActorLandingTransition type) {
        source_created.type_14 = type == ActorLandingTransition::state_40b3a0 ? 0x1A :
            type == ActorLandingTransition::bounce_40ec7e ? 0x21 : 0x12;
        trace.push_back({2, source_created.type_14, 0});
        return &source_created;
    };
    hooks.attach_state = [](void*, ActorBehavior* state) {
        valid &= state == &source_created;
        trace.push_back({3, state->type_14, 0});
    };
    hooks.select_landed_state = [](void*) { select_actor_landed_state(source_hooks()); };
    return hooks;
}
}

int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    auto* image = static_cast<std::uint8_t*>(VirtualAlloc(reinterpret_cast<void*>(0x400000),
        0xD0000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    std::uint32_t blocks = 0;
    std::cin.read(reinterpret_cast<char*>(&blocks), 4);
    if (!std::cin || blocks > 64) return 2;
    for (std::uint32_t i = 0; i < blocks; ++i) {
        std::uint32_t address = 0, size = 0;
        std::cin.read(reinterpret_cast<char*>(&address), 4);
        std::cin.read(reinterpret_cast<char*>(&size), 4);
        if (address < 0x400000 || size > 0xD0000 || address - 0x400000 > 0xD0000 - size) return 2;
        std::cin.read(reinterpret_cast<char*>(address), size);
        if (!std::cin) return 2;
    }
    jump(0x49C1A8, reinterpret_cast<std::uintptr_t>(&allocate_state));
    jump(0x40B3A0, reinterpret_cast<std::uintptr_t>(&construct_terminal));
    jump(0x40EC7E, reinterpret_cast<std::uintptr_t>(&construct_bounce));
    jump(0x41BD60, reinterpret_cast<std::uintptr_t>(&construct_landed));
    jump(0x42E696, reinterpret_cast<std::uintptr_t>(&attach));
    jump(0x40F860, reinterpret_cast<std::uintptr_t>(&bounce_log));
    DWORD old_protection = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &old_protection)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    using NativeLanding = std::uint32_t (__attribute__((thiscall)) *)(void*, void*);
    const auto native = reinterpret_cast<NativeLanding>(0x416433);
    std::array<std::int32_t, 8 * 256> cells{};
    CollisionGridHead grid{};
    grid.height = 8;
    grid.cells.data = cells.data();
    std::memcpy(reinterpret_cast<void*>(0x4C6A10), &grid, sizeof(grid));
    unsigned cases = 0;
    const auto compare = [&](const ActorGeometryState& actor, double distance, std::uint8_t umbrella) {
        std::array<std::uint8_t, 0x3A8> original{};
        for (int i = 0; i < 3; ++i) put(original, 8 + 8 * i, actor.position[i]);
        put(original, 0x44, actor.direction_44);
        put(original, 0x49, umbrella);
        put(original, 0x4C, actor.category_4c);
        for (int i = 0; i < 2; ++i) put(original, 0x360 + 8 * i, actor.pair_360[i]);
        const auto before = original;
        std::array<std::uint8_t, 0x20> falling{};
        put(falling, 0x18, distance);
        expected_actor = original.data();
        trace.clear();
        valid = true;
        const auto native_result = native(falling.data(), original.data());
        const auto native_trace = trace;
        const auto native_valid = valid;
        trace.clear();
        valid = true;
        const auto source_result = resolve_actor_landing(actor, distance, umbrella, grid, source_hooks());
        ++cases;
        if (native_result != static_cast<std::uint32_t>(source_result) || native_trace != trace ||
            !native_valid || !valid || original != before) {
            std::cerr << "landing mismatch case=" << cases << " x=" << actor.position[0]
                      << " y=" << actor.position[1] << " distance=" << distance
                      << " category=" << actor.category_4c << " native=" << native_result
                      << " source=" << source_result << '\n';
            return false;
        }
        return true;
    };
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const double distances[] = {0.0, 30.0, std::nextafter(30.0, inf), inf, nan};
    ActorGeometryState actor{};
    actor.position = {2.5, 3.5, 0};
    for (unsigned combination = 0; combination < 32; ++combination) {
        const std::uint32_t bits[] = {1, 0x80, 0x200, 0x1000, 0x4000};
        std::uint32_t mask = 0;
        for (unsigned i = 0; i < 5; ++i) if ((combination & (1U << i)) != 0) mask |= bits[i];
        cells.fill(static_cast<std::int32_t>(mask));
        for (int category = -1; category <= 3; ++category) {
            actor.category_4c = category;
            for (const auto distance : distances) {
                for (std::uint8_t umbrella : {std::uint8_t{0}, std::uint8_t{1}, std::uint8_t{0x80}}) {
                    if (!compare(actor, distance, umbrella)) return 1;
                }
            }
        }
    }
    for (const auto y : {-inf, -1.0, -0.0, 0.0, 0.1, 7.999, 8.0, inf, nan}) {
        actor.position[1] = y;
        for (std::uint32_t mask : {0U, 1U, 0x80U, 0x1000U, 0x1080U}) {
            cells.fill(static_cast<std::int32_t>(mask));
            if (!compare(actor, 31.0, 0)) return 1;
        }
    }
    std::mt19937_64 random(0x416433);
    const std::uint32_t masks[] = {0, 1, 0x80, 0x1000, 0x1080, 0x200, 0x201, 0x4000,
                                   0x4001, 0x4200, 0x2000, 0xFFFFFFFF};
    for (unsigned trial = 0; trial < 10000; ++trial) {
        for (auto& cell : cells) cell = static_cast<std::int32_t>(masks[random() % 12]);
        actor.position = {static_cast<double>(static_cast<int>(random() % 2048) - 1024) / 4.0,
                          static_cast<double>(static_cast<int>(random() % 48) - 8) / 4.0, nan};
        actor.pair_360 = {static_cast<double>(static_cast<int>(random() % 32) - 16) / 8.0,
                          static_cast<double>(static_cast<int>(random() % 32) - 16) / 8.0};
        actor.direction_44 = static_cast<std::int32_t>(random() % 5) - 2;
        actor.category_4c = static_cast<std::int32_t>(random() % 5) - 1;
        if (trial < 3) actor.position[0] = trial == 0 ? nan : trial == 1 ? inf : -inf;
        if (!compare(actor, distances[trial % 5], static_cast<std::uint8_t>(trial % 3 == 0 ? 0 : random()))) return 1;
    }
    std::cout << "native actor landing passed " << cases << " decision/trace comparisons\n";
    VirtualFree(image, 0, MEM_RELEASE);
}
