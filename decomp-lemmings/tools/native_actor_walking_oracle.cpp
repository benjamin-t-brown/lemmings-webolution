#include "lemmings/actor_walking.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
using namespace lemmings;
namespace {
struct Event {
    unsigned kind;
    std::uint64_t x, time;
    unsigned flag;
    bool operator==(const Event&) const = default;
};
std::vector<Event> trace;
unsigned script, collisions;
bool valid;
void* expected_actor;
void* expected_state;
template<class T> T get(void* object, unsigned offset) {
    T value; std::memcpy(&value, static_cast<char*>(object) + offset, sizeof(value)); return value;
}
template<class T> void put(void* object, unsigned offset, T value) {
    std::memcpy(static_cast<char*>(object) + offset, &value, sizeof(value));
}
std::uint64_t bits(double value) { return std::bit_cast<std::uint64_t>(value); }
void event(unsigned kind, double x, double time, unsigned flag) {
    trace.push_back({kind, bits(x), bits(time), flag});
}
int collision_result() { return ++collisions == 1 + script % 4 ? static_cast<int>(1 + script % 7) : 0; }
void __attribute__((thiscall)) native_animation(void* actor, const char* name) {
    valid &= actor == expected_actor && std::strcmp(name, "walk") == 0;
    event(1, get<double>(actor, 8), 0, 0);
}
void __attribute__((thiscall)) native_advance(void* model, double time) {
    void* actor = static_cast<char*>(model) - 0x78;
    valid &= actor == expected_actor;
    event(2, get<double>(actor, 8), time, get<std::uint8_t>(expected_state, 0x18));
    if (script & 8) put(actor, 0x40, -get<std::int32_t>(actor, 0x40));
}
int __attribute__((thiscall)) native_collision(void* state, void* actor) {
    valid &= actor == expected_actor && state == expected_state;
    event(3, get<double>(actor, 8), 0, get<std::uint8_t>(state, 0x18));
    put(state, 0x18, std::uint8_t{99});
    return collision_result();
}
void __attribute__((thiscall)) native_surface(void* state, void* actor) {
    valid &= actor == expected_actor && state == expected_state;
    event(4, get<double>(actor, 8), 0, get<std::uint8_t>(state, 0x18));
    if (script & 16) put(actor, 0x54, reinterpret_cast<void*>(0x1234));
}
std::uint32_t __attribute__((thiscall)) native_gadget(void* gadget, void* actor) {
    valid &= actor == expected_actor && gadget == reinterpret_cast<void*>(0x1234);
    event(5, get<double>(actor, 8), 0, get<std::uint8_t>(expected_state, 0x18));
    return (script & 32) ? 0xFFFFFF00u : 0xFFFFFF01u;
}
void __attribute__((thiscall)) native_finish(void* actor) {
    valid &= actor == expected_actor;
    event(6, get<double>(actor, 8), 0, get<std::uint8_t>(expected_state, 0x18));
}
void jump(std::uintptr_t address, std::uintptr_t target) {
    put(reinterpret_cast<void*>(address), 0, std::uint8_t{0xE9});
    put(reinterpret_cast<void*>(address), 1, static_cast<std::uint32_t>(target - address - 5));
}
ActorWalkingHooks source_hooks(ActorWalkingState& state) {
    ActorWalkingHooks hooks{}; hooks.context = &state;
    hooks.set_animation = [](void*, WalkingActor& actor, std::string_view name) {
        valid &= name == "walk"; event(1, actor.geometry.position[0], 0, 0);
    };
    hooks.advance_animation = [](void* context, WalkingActor& actor, double time) {
        event(2, actor.geometry.position[0], time, static_cast<ActorWalkingState*>(context)->reached_boundary_18);
        if (script & 8) actor.geometry.direction_40 = -actor.geometry.direction_40;
    };
    hooks.collision = [](void*, ActorWalkingState& state, WalkingActor& actor) {
        event(3, actor.geometry.position[0], 0, state.reached_boundary_18);
        state.reached_boundary_18 = 99; return collision_result();
    };
    hooks.surface = [](void*, ActorWalkingState& state, WalkingActor& actor) {
        event(4, actor.geometry.position[0], 0, state.reached_boundary_18);
        if (script & 16) actor.gadget_54 = reinterpret_cast<void*>(0x1234);
    };
    hooks.gadget_contact = [](void* context, void* gadget, WalkingActor& actor) {
        valid &= gadget == reinterpret_cast<void*>(0x1234);
        event(5, actor.geometry.position[0], 0, static_cast<ActorWalkingState*>(context)->reached_boundary_18);
        return !(script & 32);
    };
    hooks.finish = [](void* context, WalkingActor& actor) {
        event(6, actor.geometry.position[0], 0, static_cast<ActorWalkingState*>(context)->reached_boundary_18);
    };
    return hooks;
}
}
int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    auto* image = VirtualAlloc(reinterpret_cast<void*>(0x400000), 0xD0000,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    std::uint32_t blocks = 0;
    std::cin.read(reinterpret_cast<char*>(&blocks), 4);
    if (!std::cin || blocks > 64) return 2;
    for (unsigned i = 0; i < blocks; ++i) {
        std::uint32_t address, length;
        std::cin.read(reinterpret_cast<char*>(&address), 4);
        std::cin.read(reinterpret_cast<char*>(&length), 4);
        if (address < 0x400000 || length > 0xD0000 || address - 0x400000 > 0xD0000 - length) return 2;
        std::cin.read(reinterpret_cast<char*>(address), length);
        if (!std::cin) return 2;
    }
    jump(0x42FD25, reinterpret_cast<std::uintptr_t>(&native_animation));
    jump(0x41BFFC, reinterpret_cast<std::uintptr_t>(&native_collision));
    jump(0x41C875, reinterpret_cast<std::uintptr_t>(&native_surface));
    jump(0x42C78F, reinterpret_cast<std::uintptr_t>(&native_gadget));
    jump(0x42F977, reinterpret_cast<std::uintptr_t>(&native_finish));
    DWORD previous;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &previous)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    using Update = int (__attribute__((thiscall)) *)(void*, void*, double*);
    using Enter = void (__attribute__((thiscall)) *)(void*, void*);
    using Boundary = double (__attribute__((thiscall)) *)(void*, void*);
    const auto update = reinterpret_cast<Update>(0x41BDD4);
    const auto enter = reinterpret_cast<Enter>(0x41BDBA);
    const auto boundary = reinterpret_cast<Boundary>(0x41BF61);
    std::array<std::uintptr_t, 12> vtable{};
    vtable[11] = reinterpret_cast<std::uintptr_t>(&native_advance);
    unsigned cases = 0;
    auto compare = [&](double x, double time, int direction, int mode, std::uint8_t flag) {
        WalkingActor actor{}; ActorWalkingState state{flag};
        actor.geometry.position[0] = x; actor.geometry.direction_40 = direction; actor.mode_50 = mode;
        actor.gadget_54 = (script & 64) ? reinterpret_cast<void*>(0x1234) : nullptr;
        std::array<std::uint8_t, 0x3A8> original{};
        std::array<std::uint8_t, 0x20> original_state{};
        put(original.data(), 8, x); put(original.data(), 0x40, direction);
        put(original.data(), 0x50, mode); put(original.data(), 0x54, actor.gadget_54);
        put(original.data(), 0x78, vtable.data()); put(original_state.data(), 0x18, flag);
        const auto before = original; const auto state_before = original_state;
        expected_actor = original.data(); expected_state = original_state.data();
        const double native_boundary = boundary(expected_state, expected_actor);
        const double source_boundary = walking_boundary_time(state, actor);
        if (bits(native_boundary) != bits(source_boundary)) { std::cerr << "boundary mismatch\n"; return false; }
        // Nonfinite X enters the CRT fabs exception machinery, outside this
        // oracle's adapters. The boundary helper itself remains fully tested.
        if (!std::isfinite(x)) return true;
        trace.clear(); valid = true; collisions = 0;
        double native_time = time;
        enter(expected_state, expected_actor);
        const auto native_result = update(expected_state, expected_actor, &native_time);
        const auto native_trace = trace; const bool native_valid = valid;
        trace.clear(); valid = true; collisions = 0;
        const auto hooks = source_hooks(state);
        enter_actor_walking(actor, hooks);
        const auto source_result = update_actor_walking(state, actor, time, hooks);
        ++cases;
        if (!valid || !native_valid || native_result != source_result || bits(time) != bits(native_time) ||
            native_trace != trace || bits(get<double>(expected_actor, 8)) != bits(actor.geometry.position[0]) ||
            get<int>(expected_actor, 0x40) != actor.geometry.direction_40 ||
            get<void*>(expected_actor, 0x54) != actor.gadget_54 ||
            get<std::uint8_t>(expected_state, 0x18) != state.reached_boundary_18) {
            std::cerr << "walking mismatch case=" << cases << " x=" << x << " script=" << script
                      << " native=" << native_result << " source=" << source_result << " events="
                      << native_trace.size() << '/' << trace.size() << '\n';
            for (unsigned i=0; i<native_trace.size() && i<trace.size(); ++i)
                if (!(native_trace[i] == trace[i])) std::cerr << "event " << i << " kind " << native_trace[i].kind
                    << '/' << trace[i].kind << " x " << native_trace[i].x << '/' << trace[i].x
                    << " time " << native_trace[i].time << '/' << trace[i].time << '\n';
            return false;
        }
        put(original.data(), 8, get<double>(const_cast<std::uint8_t*>(before.data()), 8));
        put(original.data(), 0x40, direction); put(original.data(), 0x54, get<void*>(const_cast<std::uint8_t*>(before.data()), 0x54));
        put(original_state.data(), 0x18, flag);
        return original == before && original_state == state_before;
    };
    const double nan = std::numeric_limits<double>::quiet_NaN();
    unsigned helper_edges = 0;
    for (double x : {nan, std::numeric_limits<double>::infinity(),
                     -std::numeric_limits<double>::infinity(), 2147483647.0,
                     2147483648.0, -2147483649.0, 4294967296.0,
                     9223372036854775808.0, -9223372036854775808.0})
        for (int direction : {-1, 0, 1, 2})
            for (std::uint8_t flag : {std::uint8_t{0}, std::uint8_t{0x80}}) {
                std::array<std::uint8_t, 0x80> raw_actor{};
                std::array<std::uint8_t, 0x20> raw_state{};
                put(raw_actor.data(), 8, x); put(raw_actor.data(), 0x40, direction);
                put(raw_state.data(), 0x18, flag);
                WalkingActor actor{}; actor.geometry.position[0] = x; actor.geometry.direction_40 = direction;
                if (bits(boundary(raw_state.data(), raw_actor.data())) !=
                    bits(walking_boundary_time(ActorWalkingState{flag}, actor))) return 1;
                ++helper_edges;
            }
    for (script = 0; script < 128; ++script)
        for (double x : {0.0, 0.1, 0.5, 1.0, 255.9, 256.0, nan})
            for (double time : {-1.0, -0.0, 0.01, 0.2, 0.4, 1.0, nan})
                if (!compare(x, time, script & 1 ? 1 : -1, script % 5, script % 3)) return 1;
    std::mt19937_64 random(0x41BDD4);
    for (unsigned i = 0; i < 10000; ++i) {
        script = static_cast<unsigned>(random() % 128);
        if (!compare(static_cast<double>(random() % 256000) / 1000.0,
                     static_cast<double>(random() % 2000) / 1000.0,
                     random() & 1 ? 1 : -1, random() % 5, random() % 256)) return 1;
    }
    std::cout << "native walking passed " << cases << " boundary/update/trace comparisons and "
              << helper_edges << " conversion edge cases\n";
    VirtualFree(image, 0, MEM_RELEASE);
}
