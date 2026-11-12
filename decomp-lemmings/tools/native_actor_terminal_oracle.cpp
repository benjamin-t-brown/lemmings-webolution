#include "lemmings/actor_terminal.hpp"

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

namespace {
using Triple = std::array<double, 3>;
using Model = std::array<std::uint8_t, 0x160>;
struct Trace : lemmings::ActorTerminalEntryCallbacks, lemmings::ActorTerminalUpdateCallbacks {
    std::array<Model, 2> models{};
    std::array<std::array<std::uint8_t, 0x48>, 2> entities{};
    std::array<std::array<std::uint8_t, 0x300>, 6> children{};
    std::vector<std::uint64_t> events;
    unsigned model_count = 0, entity_count = 0;
    int child_total = 0;
    bool mutate_category = false, mutate_update = false, null_entities = false;
    std::uint8_t finished = 0;
    lemmings::ActorTerminalState* state = nullptr;
    double* remaining = nullptr;
    void reset() { events.clear(); model_count = entity_count = 0; }
    unsigned model_index(void* model) {
        return model == models[0].data() ? 0 : model == models[1].data() ? 1 : 99;
    }
    void number(std::uint64_t value) { events.push_back(value); }
    void scalar(double value) { number(std::isnan(value) ? 0x7ff8000000000000ULL : std::bit_cast<std::uint64_t>(value)); }
    void triple(const Triple& value) { for (double component : value) scalar(component); }
    void event_44535b(lemmings::TerminalActor& actor, std::int16_t id, std::int32_t value, std::uint8_t flag) override {
        number(1); number(static_cast<std::uint32_t>(id)); number(static_cast<std::uint32_t>(value)); number(flag);
        number(actor.flag_28);
        if (mutate_category) actor.category_4c = actor.category_4c == 3 ? 0 : 3;
    }
    void* create_effect(lemmings::TerminalEffectKind kind) override {
        number(2); number(static_cast<unsigned>(kind));
        return models.at(model_count++).data();
    }
    void set_count(void* effect, std::int32_t count) override { number(3); number(model_index(effect)); number(count); }
    void set_position_08(void* effect, const Triple& value) override { number(4); number(model_index(effect)); triple(value); }
    void set_pair(void* effect, double first, double second) override { number(5); number(model_index(effect)); scalar(first); scalar(second); }
    void set_mode(void* effect, std::int32_t mode) override { number(6); number(model_index(effect)); number(mode); }
    Triple world_position_43d56c(const Triple& value) override {
        number(7); triple(value);
        return {value[0] * 2 + 1, value[1] * 3 - 2, value[2] - 7};
    }
    void set_direction_54(void* effect, const Triple& value) override { number(8); number(model_index(effect)); triple(value); }
    Triple sample_triple_403f40(std::uint32_t x, std::int32_t y) override {
        number(9); number(x); number(static_cast<std::uint32_t>(y)); return {17, 29, 43};
    }
    std::int32_t child_count_4018d0(void* effect) override { number(10); number(model_index(effect)); return child_total; }
    void* child_4018b0(void* effect, std::int32_t index) override {
        number(11); number(model_index(effect)); number(index); return children.at(index).data();
    }
    void set_child_triple_403820(void* child, const Triple& value) override {
        number(12); unsigned index = 99;
        for (unsigned i = 0; i < children.size(); ++i) if (child == children[i].data()) index = i;
        number(index); triple(value);
    }
    void set_triple_4017d0(void* effect, const Triple& value) override { number(13); number(model_index(effect)); triple(value); }
    void* create_entity_432080(const Triple& value, void* effect, std::uint8_t flag) override {
        if (null_entities) return nullptr;
        number(14); triple(value); number(model_index(effect)); number(flag);
        return entities.at(entity_count++).data();
    }
    void insert_entity_401890(void*& entity) override {
        number(15); number(entity == nullptr ? 99 : entity == entities[0].data() ? 0 : 1);
    }
    void advance_model_2c(lemmings::TerminalActor&, double elapsed) override {
        number(16); scalar(elapsed);
        if (mutate_update) { *remaining = 0.75; state->elapsed_08 = 1.25; }
    }
    std::uint8_t finished_model_44(lemmings::TerminalActor&) override {
        number(17); scalar(*remaining); scalar(state->elapsed_08); return finished;
    }
} trace;
std::array<void*, 22> effect_vtable{};
std::array<void*, 18> actor_vtable{};
std::array<std::uint8_t, 0x340>* raw_actor = nullptr;
std::array<std::uint8_t, 0x18>* raw_state = nullptr;
unsigned model_allocations = 0, entity_allocations = 0;
bool pointers_ok = true;
template<class T, class B> void put(B& buffer, unsigned offset, T value) { std::memcpy(buffer.data() + offset, &value, sizeof(value)); }
template<class T, class B> T get(const B& buffer, unsigned offset) { T value; std::memcpy(&value, buffer.data() + offset, sizeof(value)); return value; }
void* __cdecl allocation(std::uint32_t size) {
    if (size == 0x128 || size == 0x158) return trace.models.at(model_allocations++).data();
    if (size == 0x48) {
        auto* result = trace.entities.at(entity_allocations++).data();
        return trace.null_entities ? nullptr : result;
    }
    pointers_ok = false; return nullptr;
}
void* construct(void* self, lemmings::TerminalEffectKind kind) {
    auto* expected = trace.create_effect(kind); pointers_ok &= expected == self;
    auto* table = effect_vtable.data(); std::memcpy(self, &table, 4); return self;
}
void* __attribute__((thiscall)) first_constructor(void* self) { return construct(self, lemmings::TerminalEffectKind::effect_4018e0); }
void* __attribute__((thiscall)) alternate_constructor(void* self) { return construct(self, lemmings::TerminalEffectKind::effect_4072a0); }
void* __attribute__((thiscall)) second_constructor(void* self) { return construct(self, lemmings::TerminalEffectKind::effect_402870); }
void __attribute__((thiscall)) native_event(void*, std::int32_t id, std::int32_t value, std::uint32_t flag) {
    lemmings::TerminalActor actor;
    actor.category_4c = get<std::int32_t>(*raw_actor, 0x4c); actor.flag_28 = (*raw_actor)[0x28];
    trace.event_44535b(actor, static_cast<std::int16_t>(id), value, static_cast<std::uint8_t>(flag));
    put(*raw_actor, 0x4c, actor.category_4c);
}
void __attribute__((thiscall)) count_adapter(void* self, std::int32_t count) { trace.set_count(self, count); }
void __attribute__((thiscall)) position_adapter(void* self, const Triple* value) { trace.set_position_08(self, *value); }
void __attribute__((thiscall)) pair_adapter(void* self, double first, double second) { trace.set_pair(self, first, second); }
void __attribute__((thiscall)) mode_adapter(void* self, std::int32_t mode) { trace.set_mode(self, mode); }
Triple* __attribute__((thiscall)) world_adapter(void*, Triple* output, const Triple* value) { *output = trace.world_position_43d56c(*value); return output; }
void __attribute__((thiscall)) direction_adapter(void* self, const Triple* value) { trace.set_direction_54(self, *value); }
Triple* __attribute__((thiscall)) sample_adapter(void*, std::uint32_t x, std::int32_t y) { static Triple value; value = trace.sample_triple_403f40(x, y); return &value; }
std::int32_t __attribute__((thiscall)) child_count_adapter(void* self) { return trace.child_count_4018d0(static_cast<std::uint8_t*>(self) - 0xb0); }
void* __attribute__((thiscall)) child_adapter(void* self, std::int32_t index) { return trace.child_4018b0(static_cast<std::uint8_t*>(self) - 0xb0, index); }
void __attribute__((thiscall)) child_triple_adapter(void* self, const Triple* value) { trace.set_child_triple_403820(static_cast<std::uint8_t*>(self) - 0xb0, *value); }
void __attribute__((thiscall)) triple_adapter(void* self, const Triple* value) { trace.set_triple_4017d0(static_cast<std::uint8_t*>(self) - 0xb0, *value); }
void* __attribute__((thiscall)) entity_constructor(void* self, const Triple* value, void* effect, std::uint32_t flag) {
    pointers_ok &= self == trace.create_entity_432080(*value, effect, static_cast<std::uint8_t>(flag)); return self;
}
void __attribute__((thiscall)) insert_adapter(void*, void** entity) { trace.insert_entity_401890(*entity); }
void __attribute__((thiscall)) advance_adapter(void* self, double elapsed) {
    pointers_ok &= self == raw_actor->data() + 0x78;
    lemmings::TerminalActor actor;
    lemmings::ActorTerminalState state{get<double>(*raw_state, 8)};
    trace.state = &state; trace.advance_model_2c(actor, elapsed); put(*raw_state, 8, state.elapsed_08);
}
std::uint8_t __attribute__((thiscall)) finished_adapter(void* self) {
    pointers_ok &= self == raw_actor->data() + 0x78;
    lemmings::TerminalActor actor;
    lemmings::ActorTerminalState state{get<double>(*raw_state, 8)};
    trace.state = &state; return trace.finished_model_44(actor);
}
void jump(std::uintptr_t address, void* target) {
    auto* code = reinterpret_cast<std::uint8_t*>(address); code[0] = 0xe9;
    const auto delta = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target) - address - 5);
    std::memcpy(code + 1, &delta, 4);
}
bool load() {
    std::uint32_t count = 0; std::cin.read(reinterpret_cast<char*>(&count), 4);
    for (unsigned i = 0; i < count; ++i) {
        std::uint32_t address = 0, size = 0;
        std::cin.read(reinterpret_cast<char*>(&address), 4); std::cin.read(reinterpret_cast<char*>(&size), 4);
        if (address < 0x400000 || address > 0x4f0000 || size > 0x4f0000 - address) return false;
        std::cin.read(reinterpret_cast<char*>(address), size);
    }
    return static_cast<bool>(std::cin);
}
}
int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    auto* image = VirtualAlloc(reinterpret_cast<void*>(0x400000), 0xf0000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    if (!load()) return 2;
    jump(0x49c1a8, reinterpret_cast<void*>(&allocation));
    jump(0x4018e0, reinterpret_cast<void*>(&first_constructor));
    jump(0x4072a0, reinterpret_cast<void*>(&alternate_constructor));
    jump(0x402870, reinterpret_cast<void*>(&second_constructor));
    jump(0x44535b, reinterpret_cast<void*>(&native_event));
    for (auto address : {0x401bb0, 0x407560, 0x402ce0}) jump(address, reinterpret_cast<void*>(&count_adapter));
    for (auto address : {0x401ef0, 0x4076d0, 0x402d80}) jump(address, reinterpret_cast<void*>(&pair_adapter));
    for (auto address : {0x401f50, 0x407730, 0x402dc0}) jump(address, reinterpret_cast<void*>(&mode_adapter));
    jump(0x43d56c, reinterpret_cast<void*>(&world_adapter));
    jump(0x403f40, reinterpret_cast<void*>(&sample_adapter));
    jump(0x4018d0, reinterpret_cast<void*>(&child_count_adapter));
    jump(0x4018b0, reinterpret_cast<void*>(&child_adapter));
    jump(0x403820, reinterpret_cast<void*>(&child_triple_adapter));
    jump(0x4017d0, reinterpret_cast<void*>(&triple_adapter));
    jump(0x432080, reinterpret_cast<void*>(&entity_constructor));
    jump(0x401890, reinterpret_cast<void*>(&insert_adapter));
    effect_vtable[2] = reinterpret_cast<void*>(&position_adapter);
    effect_vtable[21] = reinterpret_cast<void*>(&direction_adapter);
    actor_vtable[11] = reinterpret_cast<void*>(&advance_adapter);
    actor_vtable[17] = reinterpret_cast<void*>(&finished_adapter);
    DWORD old = 0; if (!VirtualProtect(image, 0xc0000, PAGE_EXECUTE_READ, &old)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xc0000);
    std::array<std::uint8_t, 0x340> actor{};
    std::array<std::uint8_t, 0x18> state{};
    raw_actor = &actor; raw_state = &state;
    using Enter = void (__attribute__((thiscall)) *)(void*, void*);
    using Update = std::uint32_t (__attribute__((thiscall)) *)(void*, void*, double*);
    using Constructor = void* (__attribute__((thiscall)) *)(void*);
    std::mt19937_64 random(0x40b3ec);
    const double special[] = {-0.0, 0.0, -1.0, 1.0, 127.0, 128.0, 10000.0,
        std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity()};
    for (unsigned trial = 0; trial < 10000; ++trial) {
        lemmings::TerminalActor source;
        for (unsigned i = 0; i < 3; ++i) source.position_08[i] = trial < 10 ? special[(trial + i) % 10] : double(static_cast<int>(random() % 8000) - 4000) / 8;
        source.flag_28 = static_cast<std::uint8_t>(random());
        source.identifier_3a = static_cast<std::int16_t>(random());
        source.direction_44 = trial < 4 ? std::array<int, 4>{1, -1, INT32_MAX, INT32_MIN}[trial] : static_cast<std::int32_t>(random());
        source.category_4c = random() % 5;
        const auto height = static_cast<std::int32_t>(random() % 1024);
        const auto event = static_cast<std::int32_t>(random());
        actor.fill(0xa5); put(actor, 8, source.position_08); put(actor, 0x28, source.flag_28);
        put(actor, 0x3a, source.identifier_3a); put(actor, 0x44, source.direction_44); put(actor, 0x4c, source.category_4c);
        put(actor, 0x78, actor_vtable.data());
        trace.child_total = random() % 7; trace.mutate_category = trial % 3 == 0;
        trace.null_entities = trial % 5 == 0;
        trace.reset();
        lemmings::enter_actor_terminal(source, height, event, trace);
        const auto expected = trace.events;
        std::memcpy(reinterpret_cast<void*>(0x4c6e28), &height, 4);
        std::memcpy(reinterpret_cast<void*>(0x4e32e0), &event, 4);
        trace.reset(); model_allocations = entity_allocations = 0;
        reinterpret_cast<Enter>(0x40b3ec)(state.data(), actor.data());
        if (!pointers_ok || trace.events != expected || model_allocations != 2 || entity_allocations != 2 ||
            actor[0x28] != source.flag_28 || get<std::int32_t>(actor, 0x4c) != source.category_4c) {
            std::cerr << "terminal entry mismatch " << trial << " events " << trace.events.size() << '/' << expected.size() << '\n';
            for (unsigned i = 0; i < std::min(trace.events.size(), expected.size()); ++i)
                if (trace.events[i] != expected[i]) { std::cerr << "first " << i << ' ' << trace.events[i] << ' ' << expected[i] << '\n'; break; }
            return 1;
        }
        state.fill(0xa5); reinterpret_cast<Constructor>(0x40b3a0)(state.data());
        lemmings::ActorTerminalState source_state{123.0}; lemmings::ActorBehavior behavior;
        lemmings::initialize_actor_terminal(source_state, behavior);
        std::array<std::uint8_t, 0x18> expected_state;
        expected_state.fill(0xa5); put(expected_state, 0, std::uint32_t{0x4b0668});
        put(expected_state, 8, source_state.elapsed_08); put(expected_state, 0x10, behavior.phase_10);
        put(expected_state, 0x11, behavior.extra_11); put(expected_state, 0x14, behavior.type_14);
        if (state != expected_state) return 1;
        reinterpret_cast<Enter>(0x40ba60)(state.data(), actor.data());
        if (state != expected_state) return 1;
        double time = trial < 10 ? special[trial] : double(static_cast<int>(random() % 800) - 400) / 16;
        source_state.elapsed_08 = trial < 10 ? special[(trial + 2) % 10] : double(static_cast<int>(random() % 800) - 400) / 8;
        put(state, 8, source_state.elapsed_08);
        double source_time = time;
        trace.mutate_update = trial % 4 == 0; trace.finished = static_cast<std::uint8_t>(random());
        trace.state = &source_state; trace.remaining = &source_time; trace.reset();
        const auto source_result = lemmings::update_actor_terminal(source_state, source, source_time, trace);
        const auto update_events = trace.events;
        trace.remaining = &time; trace.reset();
        const auto native_result = reinterpret_cast<Update>(0x40b9f2)(state.data(), actor.data(), &time);
        const auto elapsed = get<double>(state, 8);
        if (!pointers_ok || native_result != source_result || time != source_time ||
            (!(std::isnan(elapsed) && std::isnan(source_state.elapsed_08)) && elapsed != source_state.elapsed_08) ||
            trace.events != update_events) { std::cerr << "terminal update mismatch " << trial << '\n'; return 1; }
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native terminal behavior passed 10000 entries, constructors and updates\n";
}
