#include "../tests/actor_walking_collision_fixture.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <cstring>
#include <iostream>
#include <random>

namespace {
using namespace walking_collision_test;
using Type = lemmings::WalkingCollisionTransition;
Fixture* active;
std::array<std::uint8_t, 0x3A8> native_actor{};
std::array<std::array<std::uint8_t, 0x130>, 3> native_gadgets{};
std::array<void*, 3> gadget_entries{};
std::array<std::array<std::uint8_t, 0x30>, 4> native_states{};
std::array<std::uintptr_t, 9> gadget_vtable{};
std::array<std::uintptr_t, 3> state_vtable{};
template<class T> void store(void* address, std::size_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(address) + offset, &value, sizeof(value));
}
template<class T> T load(const void* address, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(address) + offset, sizeof(value));
    return value;
}
std::size_t gadget_index(void* gadget) {
    for (std::size_t i = 0; i < native_gadgets.size(); ++i)
        if (gadget == native_gadgets[i].data()) return i;
    throw std::runtime_error("unexpected native gadget");
}
std::size_t state_index(void* state) {
    for (std::size_t i = 0; i < native_states.size(); ++i)
        if (state == native_states[i].data()) return i;
    throw std::runtime_error("unexpected native state");
}
void read_native() {
    auto& actor = active->actor;
    std::memcpy(actor.geometry.position.data(), native_actor.data() + 8, 24);
    actor.geometry.direction_40 = load<std::int32_t>(native_actor.data(), 0x40);
    actor.geometry.direction_44 = load<std::int32_t>(native_actor.data(), 0x44);
    std::memcpy(actor.geometry.pair_350.data(), native_actor.data() + 0x350, 16);
    std::memcpy(actor.geometry.pair_360.data(), native_actor.data() + 0x360, 16);
    actor.special_4a = native_actor[0x4A];
    const auto primary = load<void*>(native_actor.data(), 0x340);
    actor.primary_340 = primary ? &active->states[state_index(primary)] : nullptr;
    for (std::size_t i = 0; i < native_gadgets.size(); ++i) {
        active->gadgets[i].associated = load<void*>(native_gadgets[i].data(), 0xA4) == native_actor.data();
    }
}
std::uint8_t __attribute__((thiscall)) query_adapter(void*, std::int32_t id) {
    return active->has_state_42ecc9(active->actor, id);
}
std::int32_t __attribute__((thiscall)) count_adapter(void*) { return active->gadget_count_41cb60(); }
void** __attribute__((thiscall)) at_adapter(void*, std::int32_t index) {
    active->gadget_at_41cb40(index); return &gadget_entries[index];
}
std::uint8_t __attribute__((thiscall)) contains_adapter(void* gadget, const std::array<double, 3>* point) {
    return active->gadget_contains_430ba1(&active->gadgets[gadget_index(gadget)], *point);
}
std::uint8_t __attribute__((thiscall)) enabled_adapter(void* gadget) {
    return active->gadget_enabled_41cba0(&active->gadgets[gadget_index(gadget)]);
}
void __attribute__((thiscall)) activate_adapter(void* gadget) {
    active->activate_gadget_20(&active->gadgets[gadget_index(gadget)]);
}
const char* __attribute__((thiscall)) name_adapter(void* string) {
    const auto gadget = static_cast<std::uint8_t*>(string) - 0xA8;
    active->gadget_animation_406500(&active->gadgets[gadget_index(gadget)]);
    return "gadget-animation";
}
void __attribute__((thiscall)) animation_adapter(void*, const char* name) {
    active->select_animation_42fd25(active->actor, name);
}
void __attribute__((thiscall)) clear_adapter(void*) { active->clear_states_42e78a(active->actor); }
void* __cdecl allocate_adapter(std::uint32_t bytes) {
    active->record(allocation, bytes); return native_states[active->created].data();
}
void* constructed(void* state, Type type) {
    active->record(construct, static_cast<std::uint64_t>(type));
    active->types[active->created++] = type;
    store(state, 0, state_vtable.data());
    return state;
}
void* __attribute__((thiscall)) construct_gadget(void* state) { return constructed(state, Type::gadget_41ad90); }
void* __attribute__((thiscall)) construct_falling(void* state) { return constructed(state, Type::falling_415fa0); }
void* __attribute__((thiscall)) construct_other(void* state) { return constructed(state, Type::gadget_4041fd); }
void* __attribute__((thiscall)) construct_climbing(void* state) { return constructed(state, Type::climbing_413750); }
void __attribute__((thiscall)) attach_adapter(void*, void* state) {
    active->attach_42e696(active->actor, &active->states[state_index(state)]);
    store(native_actor.data(), 0x340, state);
}
void __attribute__((thiscall)) enter_adapter(void* state, void*) {
    active->enter_state_08(&active->states[state_index(state)], active->actor);
}
void __attribute__((thiscall)) move_adapter(void*, double y) {
    active->move_y_42ee74(active->actor, y);
    store(native_actor.data(), 0x10, active->actor.geometry.position[1]);
}
void jump(std::uintptr_t address, std::uintptr_t target) {
    auto* destination = reinterpret_cast<std::uint8_t*>(address);
    destination[0] = 0xE9;
    store(destination, 1, static_cast<std::uint32_t>(target - address - 5));
}
void write_native(const Fixture& f) {
    const auto& actor = f.actor;
    native_actor.fill(0);
    std::memcpy(native_actor.data() + 8, actor.geometry.position.data(), 24);
    store(native_actor.data(), 0x40, actor.geometry.direction_40);
    store(native_actor.data(), 0x44, actor.geometry.direction_44);
    store(native_actor.data(), 0x48, actor.climbable_48);
    store(native_actor.data(), 0x4A, actor.special_4a);
    store(native_actor.data(), 0x4C, actor.geometry.category_4c);
    std::memcpy(native_actor.data() + 0x350, actor.geometry.pair_350.data(), 16);
    std::memcpy(native_actor.data() + 0x360, actor.geometry.pair_360.data(), 16);
    for (std::size_t i = 0; i < native_gadgets.size(); ++i) {
        native_gadgets[i].fill(0);
        store(native_gadgets[i].data(), 0, gadget_vtable.data());
        store(native_gadgets[i].data(), 0x128, f.gadgets[i].mode);
        gadget_entries[i] = native_gadgets[i].data();
        if (actor.gadget_54 == &f.gadgets[i]) store(native_actor.data(), 0x54, native_gadgets[i].data());
    }
    std::memcpy(reinterpret_cast<void*>(0x004C6A10), &f.grid, sizeof(f.grid));
}
void copy_initial(const Fixture& source, Fixture& target) {
    target.actor = source.actor;
    target.cells = source.cells;
    target.grid.height = source.grid.height;
    target.gadgets = source.gadgets;
    target.gadget_count = source.gadget_count;
    target.query_result = source.query_result;
    for (std::size_t i = 0; i < source.gadgets.size(); ++i)
        if (source.actor.gadget_54 == &source.gadgets[i]) target.actor.gadget_54 = &target.gadgets[i];
}
bool equal(const Fixture& a, const Fixture& b) {
    return a.trace == b.trace && a.created == b.created && a.types == b.types &&
        a.actor.special_4a == b.actor.special_4a &&
        a.state_id(a.actor.primary_340) == b.state_id(b.actor.primary_340) &&
        a.actor.geometry.direction_40 == b.actor.geometry.direction_40 &&
        a.actor.geometry.direction_44 == b.actor.geometry.direction_44 &&
        bits(a.actor.geometry.position[0]) == bits(b.actor.geometry.position[0]) &&
        bits(a.actor.geometry.position[1]) == bits(b.actor.geometry.position[1]) &&
        bits(a.actor.geometry.position[2]) == bits(b.actor.geometry.position[2]) &&
        bits(a.actor.geometry.pair_350[0]) == bits(b.actor.geometry.pair_350[0]) &&
        bits(a.actor.geometry.pair_360[0]) == bits(b.actor.geometry.pair_360[0]);
}
void print(const Fixture& f) {
    for (const auto& item : f.trace) {
        for (auto value : item) std::cerr << std::hex << value << ' ';
        std::cerr << '\n';
    }
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
    for (std::uint32_t i = 0; i < blocks; ++i) {
        std::uint32_t address = 0, size = 0;
        std::cin.read(reinterpret_cast<char*>(&address), 4);
        std::cin.read(reinterpret_cast<char*>(&size), 4);
        if (address < 0x400000 || size > 0xD0000 || address - 0x400000 > 0xD0000 - size) return 2;
        std::cin.read(reinterpret_cast<char*>(address), size);
        if (!std::cin) return 2;
    }
    jump(0x42ECC9, reinterpret_cast<std::uintptr_t>(&query_adapter));
    jump(0x41CB60, reinterpret_cast<std::uintptr_t>(&count_adapter));
    jump(0x41CB40, reinterpret_cast<std::uintptr_t>(&at_adapter));
    jump(0x430BA1, reinterpret_cast<std::uintptr_t>(&contains_adapter));
    jump(0x41CBA0, reinterpret_cast<std::uintptr_t>(&enabled_adapter));
    jump(0x406500, reinterpret_cast<std::uintptr_t>(&name_adapter));
    jump(0x42FD25, reinterpret_cast<std::uintptr_t>(&animation_adapter));
    jump(0x42E78A, reinterpret_cast<std::uintptr_t>(&clear_adapter));
    jump(0x49C1A8, reinterpret_cast<std::uintptr_t>(&allocate_adapter));
    jump(0x41AD90, reinterpret_cast<std::uintptr_t>(&construct_gadget));
    jump(0x415FA0, reinterpret_cast<std::uintptr_t>(&construct_falling));
    jump(0x4041FD, reinterpret_cast<std::uintptr_t>(&construct_other));
    jump(0x413750, reinterpret_cast<std::uintptr_t>(&construct_climbing));
    jump(0x42E696, reinterpret_cast<std::uintptr_t>(&attach_adapter));
    jump(0x42EE74, reinterpret_cast<std::uintptr_t>(&move_adapter));
    gadget_vtable[0x20 / 4] = reinterpret_cast<std::uintptr_t>(&activate_adapter);
    state_vtable[0x08 / 4] = reinterpret_cast<std::uintptr_t>(&enter_adapter);
    DWORD old_protection = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &old_protection)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    using Native = std::int32_t (__attribute__((thiscall)) *)(void*, void*);
    const auto native = reinterpret_cast<Native>(0x41BFFC);
    std::mt19937_64 random(0x41BFFC);
    constexpr std::uint32_t masks[] = {0, 1, 0x800, 0x801, 0x8000, 0x200, 0x4000, 0x80};
    std::array<std::size_t, 3> returns{};
    std::size_t creations = 0, queries = 0;
    for (int trial = 0; trial != 25000; ++trial) {
        Fixture source, original;
        auto& actor = source.actor;
        actor.geometry.direction_40 = (random() & 1) ? -1 : 1;
        actor.geometry.direction_44 = (random() & 1) ? -1 : 1;
        actor.geometry.category_4c = static_cast<int>(random() % 6) - 1;
        actor.climbable_48 = static_cast<std::uint8_t>(random() % 3 == 0 ? 0 : 255);
        actor.special_4a = static_cast<std::uint8_t>(random());
        actor.geometry.position[0] = static_cast<double>(static_cast<int>(random() % 1024) - 512) / 2.0;
        actor.geometry.position[1] = static_cast<double>(random() % 800) / 100.0;
        actor.geometry.pair_360 = {(random() & 1) ? -1.0 : 0.0, (random() & 1) ? 3.0 : 0.0};
        source.query_result = static_cast<std::uint8_t>(random() % 3 == 0 ? 0 : 255);
        source.gadget_count = 1 + random() % 3;
        for (auto& gadget : source.gadgets) {
            gadget.mode = static_cast<int>(random() % 5) - 1;
            gadget.enabled = (random() & 1) ? 255 : 0;
            gadget.contains = (random() & 1) ? 255 : 0;
        }
        if (random() % 4 == 0) actor.gadget_54 = &source.gadgets[random() % 3];
        for (std::size_t y = 0; y < 8; ++y) source.row(y, masks[random() % std::size(masks)]);
        if (trial < 4096) {
            // Exhaust all 8^4 combinations of the four selected terrain rows.
            actor.geometry.position = {2.5, 3.5, 0};
            actor.geometry.direction_44 = 1;
            actor.geometry.pair_360 = {0, 0};
            actor.gadget_54 = nullptr;
            source.row(2, masks[trial & 7]);
            source.row(3, masks[(trial >> 3) & 7]);
            source.row(4, masks[(trial >> 6) & 7]);
            source.row(5, masks[(trial >> 9) & 7]);
        }
        if (trial % 100 == 99) actor.geometry.position[1] = std::bit_cast<double>(0x7FF8000000001234ULL);
        if (trial >= 4096 && trial % 83 == 0) {
            actor.geometry.direction_44 = std::array<std::int32_t, 3>{0, INT32_MIN, INT32_MAX}[trial % 3];
        }
        if (trial >= 4096 && trial % 101 == 0) actor.geometry.direction_40 = INT32_MIN;
        if (trial >= 4096 && trial % 79 == 0) {
            actor.geometry.position[0] = std::bit_cast<double>(
                std::array<std::uint64_t, 3>{0x7FF0000000000000ULL,
                    0x7FF8000000001234ULL, 0xFFF0000000000000ULL}[trial % 3]);
        }
        copy_initial(source, original);
        active = &original;
        write_native(original);
        original.sync = &read_native;
        const auto native_result = native(nullptr, native_actor.data());
        read_native();
        const auto source_result = source.run();
        if (native_result != source_result || !equal(source, original)) {
            std::cerr << "walking collision mismatch trial " << trial << " native=" << native_result
                      << " source=" << source_result << "\nsource:\n";
            print(source); std::cerr << "native:\n"; print(original);
            return 1;
        }
        ++returns[source_result];
        creations += source.created;
        for (const auto& item : source.trace) if (item[0] == query) ++queries;
    }
    std::cout << "25000 walking collision calls matched; returns0/1/2=" << returns[0] << '/'
              << returns[1] << '/' << returns[2] << "; " << creations << " state creations; "
              << queries << " gadget-state queries; terrain, state and callback traces compared\n";
    VirtualFree(image, 0, MEM_RELEASE);
}
