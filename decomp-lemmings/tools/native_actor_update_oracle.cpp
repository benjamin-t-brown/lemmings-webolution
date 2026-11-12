// Native-byte comparison of the 0x42E2BF behavior scheduler, with an explicit
// adapter at its 0x42E58C terminal animation/terrain boundary.
#include "lemmings/actor_update.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

namespace {
using namespace lemmings;
struct Event {
    unsigned kind, state;
    std::uint64_t value;
    bool operator==(const Event&) const = default;
};
struct Scenario {
    int mode;
    std::uint8_t force;
    bool null_primary;
    bool mutate_entry;
    unsigned extras;
    std::array<std::uint8_t, 8> phases;
    std::array<std::uint32_t, 8> results;
    std::array<std::int32_t, 8> types;
    double elapsed;
};
struct Runtime {
    Scenario scenario;
    std::array<unsigned, 8> calls{};
    std::vector<Event> trace;
    bool valid = true;
    std::uint32_t update(unsigned id, double& time) {
        trace.push_back({1, id, std::bit_cast<std::uint64_t>(time)});
        time -= 0.25;
        return calls[id]++ == 0 ? scenario.results[id] : 3;
    }
};
Runtime* active;
struct SourceStateData { unsigned id; };
void source_destroy(ActorBehavior& state, std::uint32_t flags) {
    active->trace.push_back({2, static_cast<SourceStateData*>(state.context)->id, flags});
}
std::uint32_t source_update(ActorBehavior& state, ActorBehaviorUpdate& actor, double& time) {
    const auto result = active->update(static_cast<SourceStateData*>(state.context)->id, time);
    if (result == 1 && actor.primary == &state) {
        actor.primary = static_cast<ActorBehavior*>(actor.context) + 1;
    }
    return result;
}
void source_leave(ActorBehavior& state, ActorBehaviorUpdate&) {
    active->trace.push_back({3, static_cast<SourceStateData*>(state.context)->id, 0});
}
void source_enter(ActorBehavior& state, ActorBehaviorUpdate&) {
    active->trace.push_back({9, static_cast<SourceStateData*>(state.context)->id, 0});
    if (active->scenario.mutate_entry) state.extra_11 ^= 1;
}
const ActorBehaviorOperations source_operations{source_destroy, source_update, source_enter, source_leave};

struct Result {
    bool returned;
    std::uint8_t force;
    unsigned primary;
    std::vector<unsigned> extras;
    std::vector<Event> trace;
    bool valid;
    bool operator==(const Result&) const = default;
};
Result source_run(const Scenario& scenario) {
    Runtime runtime{scenario, {}, {}, true};
    active = &runtime;
    ActorBehaviorUpdate actor;
    std::array<ActorBehavior, 8> states{};
    std::array<SourceStateData, 8> data{};
    std::array<ActorBehaviorNode, 5> nodes{};
    for (unsigned i = 0; i < 8; ++i) {
        data[i].id = i;
        states[i] = {&source_operations, &data[i], scenario.phases[i]};
    }
    actor.context = states.data();
    actor.mode_50 = scenario.mode;
    actor.forced_transition_5d = scenario.force;
    actor.identifier_3a = -123;
    actor.primary = scenario.null_primary ? nullptr : &states[0];
    for (unsigned i = 0; i < scenario.extras; ++i) {
        nodes[i] = {&states[i + 2], &actor.extra.end, actor.extra.end.previous};
        actor.extra.end.previous->next = &nodes[i];
        actor.extra.end.previous = &nodes[i];
    }
    ActorBehaviorHooks hooks{};
    hooks.forced_event_value = 0x12345678;
    hooks.forced_event = [](void*, std::int16_t id, std::int32_t value, std::uint8_t flag) {
        active->valid &= id == -123 && flag == 0;
        active->trace.push_back({5, 0, static_cast<std::uint32_t>(value)});
    };
    hooks.create_forced = [](void*, ActorBehaviorUpdate& actor) {
        active->trace.push_back({6, 7, 0});
        return static_cast<ActorBehavior*>(actor.context) + 7;
    };
    hooks.attach = [](void*, ActorBehaviorUpdate& actor, ActorBehavior* state) {
        attach_actor_behavior(actor, *state, {});
    };
    hooks.release_node = [](void*, ActorBehaviorNode& node) {
        active->valid &= node.previous->next == node.next && node.next->previous == node.previous;
        active->trace.push_back({4, static_cast<SourceStateData*>(node.behavior->context)->id, 1});
    };
    hooks.finalize = [](void*, ActorBehaviorUpdate&) { active->trace.push_back({8, 0, 0}); };
    Result result{};
    result.returned = update_actor_behavior_schedule(actor, scenario.elapsed, hooks);
    result.force = actor.forced_transition_5d;
    result.primary = static_cast<SourceStateData*>(actor.primary->context)->id;
    for (auto* node = actor.extra.begin.next; node != &actor.extra.end; node = node->next) {
        result.extras.push_back(static_cast<SourceStateData*>(node->behavior->context)->id);
    }
    result.trace = runtime.trace;
    result.valid = runtime.valid;
    return result;
}

struct NativeState {
    void** table;
    unsigned id;
    std::uint64_t unknown;
    std::uint8_t phase, extra;
    std::uint16_t padding;
    std::int32_t type;
};
static_assert(offsetof(NativeState, phase) == 0x10);
static_assert(offsetof(NativeState, extra) == 0x11);
static_assert(offsetof(NativeState, type) == 0x14);
struct NativeNode { NativeState* state; NativeNode* next; NativeNode* previous; };
std::array<NativeState, 8> native_states;
std::array<NativeNode, 8> allocated_nodes;
unsigned allocated_node_count = 0;
alignas(8) std::array<std::uint8_t, 0x390> native_actor;
template<class T> T& field(std::size_t offset) {
    return *reinterpret_cast<T*>(native_actor.data() + offset);
}
void* __attribute__((thiscall)) native_destroy(NativeState* state, unsigned flags) {
    active->trace.push_back({2, state->id, flags});
    return state;
}
std::uint32_t __attribute__((thiscall)) native_update(NativeState* state, void* actor, double* time) {
    active->valid &= actor == native_actor.data();
    const auto result = active->update(state->id, *time);
    if (result == 1 && field<NativeState*>(0x340) == state) {
        field<NativeState*>(0x340) = &native_states[1];
    }
    return result;
}
void __attribute__((thiscall)) native_leave(NativeState* state, void* actor) {
    active->valid &= actor == native_actor.data();
    active->trace.push_back({3, state->id, 0});
}
void __attribute__((thiscall)) native_enter(NativeState* state, void* actor) {
    active->valid &= actor == native_actor.data();
    active->trace.push_back({9, state->id, 0});
    if (active->scenario.mutate_entry) state->extra ^= 1;
}
void* __attribute__((thiscall)) native_release(NativeNode* node, unsigned flags) {
    active->valid &= node->previous->next == node->next && node->next->previous == node->previous;
    active->trace.push_back({4, node->state->id, flags});
    return node;
}
void __attribute__((thiscall)) native_event(void* world, int id, int value, unsigned flag) {
    active->valid &= world == reinterpret_cast<void*>(0x4C6A08) && id == -123 && flag == 0;
    active->trace.push_back({5, 0, static_cast<std::uint32_t>(value)});
}
void* __cdecl native_allocate(unsigned bytes) {
    if (bytes == 12) {
        active->trace.push_back({11, 0, 12});
        return &allocated_nodes.at(allocated_node_count++);
    }
    active->valid &= bytes == 0x18;
    return &native_states[7];
}
void* __attribute__((thiscall)) native_construct(NativeState* state) {
    active->trace.push_back({6, state->id, 0});
    return state;
}
void __attribute__((thiscall)) native_finalize(void* actor) {
    active->valid &= actor == native_actor.data();
    active->trace.push_back({8, 0, 0});
}
Result native_run(const Scenario& scenario) {
    Runtime runtime{scenario, {}, {}, true};
    active = &runtime;
    void* table[] = {reinterpret_cast<void*>(&native_destroy), reinterpret_cast<void*>(&native_update),
                     reinterpret_cast<void*>(&native_enter), reinterpret_cast<void*>(&native_leave)};
    for (unsigned i = 0; i < 8; ++i) native_states[i] = {table, i, 0, scenario.phases[i], 0, 0, scenario.types[i]};
    native_actor.fill(0);
    field<std::int32_t>(0x50) = scenario.mode;
    field<std::uint8_t>(0x5D) = scenario.force;
    field<std::int16_t>(0x3A) = -123;
    field<NativeState*>(0x340) = scenario.null_primary ? nullptr : &native_states[0];
    auto* begin = reinterpret_cast<NativeNode*>(native_actor.data() + 0x374);
    auto* end = reinterpret_cast<NativeNode*>(native_actor.data() + 0x380);
    begin->next = end;
    end->previous = begin;
    std::array<NativeNode, 5> nodes{};
    for (unsigned i = 0; i < scenario.extras; ++i) {
        nodes[i] = {&native_states[i + 2], end, end->previous};
        end->previous->next = &nodes[i];
        end->previous = &nodes[i];
    }
    using NativeUpdate = unsigned char (__attribute__((thiscall)) *)(void*, double);
    Result result{};
    result.returned = reinterpret_cast<NativeUpdate>(0x42E2BF)(native_actor.data(), scenario.elapsed) != 0;
    result.force = field<std::uint8_t>(0x5D);
    result.primary = field<NativeState*>(0x340)->id;
    for (auto* node = begin->next; node != end; node = node->next) result.extras.push_back(node->state->id);
    result.trace = runtime.trace;
    result.valid = runtime.valid;
    return result;
}

Result source_attachment_run(const Scenario& scenario) {
    Runtime runtime{scenario, {}, {}, true};
    active = &runtime;
    ActorBehaviorUpdate actor;
    std::array<ActorBehavior, 8> states{};
    std::array<SourceStateData, 8> data{};
    std::array<ActorBehaviorNode, 8> nodes{};
    for (unsigned i = 0; i < 8; ++i) {
        data[i].id = i;
        states[i] = {&source_operations, &data[i], 0, scenario.phases[i], scenario.types[i]};
    }
    actor.primary = scenario.null_primary ? nullptr : &states[0];
    struct Allocation { ActorBehaviorNode* nodes; unsigned count = 0; } allocation{nodes.data()};
    ActorBehaviorAttachmentHooks hooks{&allocation, [](void* context) {
        auto& allocation = *static_cast<Allocation*>(context);
        active->trace.push_back({11, 0, 12});
        return &allocation.nodes[allocation.count++];
    }};
    for (unsigned i = 1; i < 8; ++i) {
        attach_actor_behavior(actor, states[i], hooks);
        for (auto type : scenario.types) runtime.trace.push_back(
            {10, has_actor_behavior(actor, type) ? 1U : 0U, static_cast<std::uint32_t>(type)});
        runtime.trace.push_back({10, has_actor_behavior(actor, 0x12345678) ? 1U : 0U, 0x12345678});
    }
    Result result{};
    result.returned = true;
    result.primary = actor.primary == nullptr ? 99 : static_cast<SourceStateData*>(actor.primary->context)->id;
    for (auto* node = actor.extra.begin.next; node != &actor.extra.end; node = node->next) {
        runtime.valid &= node->next->previous == node && node->previous->next == node;
        result.extras.push_back(static_cast<SourceStateData*>(node->behavior->context)->id);
    }
    result.trace = runtime.trace;
    result.valid = runtime.valid;
    return result;
}
Result native_attachment_run(const Scenario& scenario) {
    Runtime runtime{scenario, {}, {}, true};
    active = &runtime;
    void* table[] = {reinterpret_cast<void*>(&native_destroy), reinterpret_cast<void*>(&native_update),
                     reinterpret_cast<void*>(&native_enter), reinterpret_cast<void*>(&native_leave)};
    for (unsigned i = 0; i < 8; ++i) native_states[i] = {table, i, 0, 0, scenario.phases[i], 0, scenario.types[i]};
    native_actor.fill(0);
    allocated_nodes.fill({});
    allocated_node_count = 0;
    field<NativeState*>(0x340) = scenario.null_primary ? nullptr : &native_states[0];
    auto* begin = reinterpret_cast<NativeNode*>(native_actor.data() + 0x374);
    auto* end = reinterpret_cast<NativeNode*>(native_actor.data() + 0x380);
    begin->next = end;
    end->previous = begin;
    using Attach = void (__attribute__((thiscall)) *)(void*, NativeState*);
    using Query = unsigned char (__attribute__((thiscall)) *)(void*, std::int32_t);
    const auto attach = reinterpret_cast<Attach>(0x42E696);
    const auto query = reinterpret_cast<Query>(0x42ECC9);
    for (unsigned i = 1; i < 8; ++i) {
        attach(native_actor.data(), &native_states[i]);
        for (auto type : scenario.types) runtime.trace.push_back(
            {10, query(native_actor.data(), type) != 0 ? 1U : 0U, static_cast<std::uint32_t>(type)});
        runtime.trace.push_back({10, query(native_actor.data(), 0x12345678) != 0 ? 1U : 0U, 0x12345678});
    }
    Result result{};
    result.returned = true;
    result.primary = field<NativeState*>(0x340) == nullptr ? 99 : field<NativeState*>(0x340)->id;
    for (auto* node = begin->next; node != end; node = node->next) {
        runtime.valid &= node->next->previous == node && node->previous->next == node;
        result.extras.push_back(node->state->id);
    }
    result.trace = runtime.trace;
    result.valid = runtime.valid;
    return result;
}
void jump(std::uintptr_t address, std::uintptr_t target, std::uint8_t opcode = 0xE9) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(address);
    bytes[0] = opcode;
    const auto displacement = static_cast<std::uint32_t>(target - address - 5);
    std::memcpy(bytes + 1, &displacement, 4);
}
template<std::size_t Size> bool read_at(std::uintptr_t address) {
    std::array<char, Size> bytes{};
    std::cin.read(bytes.data(), bytes.size());
    if (std::cin.gcount() != static_cast<std::streamsize>(bytes.size())) return false;
    std::memcpy(reinterpret_cast<void*>(address), bytes.data(), bytes.size());
    return true;
}
}

int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    constexpr std::size_t image_size = 0xF0000;
    void* image = VirtualAlloc(reinterpret_cast<void*>(0x400000), image_size,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    if (!read_at<983>(0x42E2BF) || !read_at<18>(0x406A00) ||
        !read_at<83>(0x42FEF0) || !read_at<17>(0x430170) ||
        !read_at<29>(0x430190) || !read_at<8>(0x4B0338) ||
        !read_at<116>(0x42E696) || !read_at<132>(0x42ECC9) ||
        !read_at<17>(0x430A50) || !read_at<17>(0x430A70) ||
        !read_at<25>(0x430290) || !read_at<147>(0x430590) ||
        !read_at<44>(0x430630)) return 2;
    jump(0x44535B, reinterpret_cast<std::uintptr_t>(&native_event));
    jump(0x49C1A8, reinterpret_cast<std::uintptr_t>(&native_allocate));
    jump(0x416BF0, reinterpret_cast<std::uintptr_t>(&native_construct));
    jump(0x42FF50, reinterpret_cast<std::uintptr_t>(&native_release));
    // Terminal-only boundary replacement: mov ecx,[ebp-5c]; call finalize;
    // jump to original mov al,1 and original exception-frame epilogue.
    const std::uint8_t actor_load[] = {0x8B, 0x4D, 0xA4};
    std::memcpy(reinterpret_cast<void*>(0x42E58C), actor_load, sizeof(actor_load));
    jump(0x42E58F, reinterpret_cast<std::uintptr_t>(&native_finalize), 0xE8);
    jump(0x42E594, 0x42E674);
    *reinterpret_cast<std::int32_t*>(0x4E32CC) = 0x12345678;
    DWORD old_protection = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &old_protection)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    std::mt19937 random(0x42E2BF);
    const double elapsed[] = {1.0, 0.125, 2.5, 0.0, -1.0,
        std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity()};
    const std::uint8_t phases[] = {0, 1, 2, 0xFF};
    for (unsigned trial = 0; trial < 20000; ++trial) {
        Scenario scenario{};
        scenario.mode = trial % 3 == 0 ? 2 : trial % 3 == 1 ? 1 : -1;
        scenario.force = trial % 7 == 0 ? 0x80 : 0;
        scenario.extras = trial % 6;
        scenario.elapsed = elapsed[trial % 7];
        scenario.null_primary = scenario.force != 0 && scenario.elapsed > 0.0 && trial % 2 == 0;
        for (unsigned i = 0; i < 8; ++i) {
            scenario.phases[i] = phases[random() % 4];
            scenario.results[i] = random() % 5;
            scenario.types[i] = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(random()));
        }
        scenario.results[1] = 3; // Replacement is stable after the transition.
        scenario.results[7] = 3;
        const auto native = native_run(scenario);
        const auto source = source_run(scenario);
        if (native != source || !native.valid) {
            std::cerr << "actor scheduler mismatch trial=" << trial
                      << " native events=" << native.trace.size()
                      << " source events=" << source.trace.size() << '\n';
            return 1;
        }
    }
    for (unsigned trial = 0; trial < 10000; ++trial) {
        Scenario scenario{};
        scenario.null_primary = trial % 2 == 0;
        scenario.mutate_entry = trial % 3 == 0;
        for (unsigned i = 0; i < 8; ++i) {
            scenario.phases[i] = phases[random() % 4];
            scenario.types[i] = trial % 3 == 0 ? static_cast<int>(i % 3) - 1 :
                std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(random()));
        }
        const auto native = native_attachment_run(scenario);
        const auto source = source_attachment_run(scenario);
        if (native != source || !native.valid) {
            std::cerr << "actor attachment/query mismatch trial=" << trial << '\n';
            return 1;
        }
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native actor scheduler passed 20000 trace/state comparisons; "
                 "attachment/query passed 10000 scenarios (70000 attachments, 630000 queries)\n";
}
