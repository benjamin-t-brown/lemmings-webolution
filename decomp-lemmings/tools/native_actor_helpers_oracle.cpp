#include "../tests/actor_clear_fixture.hpp"
#include "lemmings/actor_motion.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <bit>
#include <cstring>
#include <iostream>
#include <random>

namespace {
using actor_clear_test::Fixture;
Fixture* active;
std::array<std::uint8_t, 0x3A8> native_actor{};
std::array<std::array<std::uint8_t, 12>, 18> native_nodes{};
std::array<std::array<std::uint8_t, 24>, 18> native_states{};
std::array<std::uintptr_t, 4> vtable{};
template<class T> void store(void* address, std::size_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(address) + offset, &value, sizeof(value));
}
template<class T> T load(const void* address, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(address) + offset, sizeof(value));
    return value;
}
void* encode_node(lemmings::ActorBehaviorNode* node) {
    const auto id = active->node_id(node);
    if (id < 18) return native_nodes[id].data();
    if (id == 100) return native_actor.data() + 0x374;
    if (id == 101) return native_actor.data() + 0x380;
    return nullptr;
}
lemmings::ActorBehaviorNode* decode_node(void* node) {
    if (node == native_actor.data() + 0x374) return &active->actor.extra.begin;
    if (node == native_actor.data() + 0x380) return &active->actor.extra.end;
    for (std::size_t i = 0; i < native_nodes.size(); ++i)
        if (node == native_nodes[i].data()) return &active->nodes[i];
    return nullptr;
}
void* encode_state(lemmings::ActorBehavior* state) {
    const auto id = active->state_id(state);
    return id < 18 ? native_states[id].data() : nullptr;
}
lemmings::ActorBehavior* decode_state(void* state) {
    for (std::size_t i = 0; i < native_states.size(); ++i)
        if (state == native_states[i].data()) return &active->states[i];
    return nullptr;
}
void write_node(lemmings::ActorBehaviorNode& node) {
    auto* native = encode_node(&node);
    store(native, 0, encode_state(node.behavior));
    store(native, 4, encode_node(node.next));
    store(native, 8, encode_node(node.previous));
}
void read_node(lemmings::ActorBehaviorNode& node) {
    auto* native = encode_node(&node);
    node.behavior = decode_state(load<void*>(native, 0));
    node.next = decode_node(load<void*>(native, 4));
    node.previous = decode_node(load<void*>(native, 8));
}
void write_links() {
    write_node(active->actor.extra.begin);
    write_node(active->actor.extra.end);
    for (auto& node : active->nodes) write_node(node);
    store(native_actor.data(), 0x340, encode_state(active->actor.primary));
    for (std::size_t i=0;i<native_states.size();++i) store(native_states[i].data(),0x14,active->states[i].type_14);
}
void read_links() {
    read_node(active->actor.extra.begin);
    read_node(active->actor.extra.end);
    for (auto& node : active->nodes) read_node(node);
    active->actor.primary = decode_state(load<void*>(native_actor.data(), 0x340));
    for (std::size_t i=0;i<native_states.size();++i) active->states[i].type_14=load<std::int32_t>(native_states[i].data(),0x14);
}
void __attribute__((thiscall)) leave_adapter(void* state, void* actor) {
    if (actor != native_actor.data()) throw std::runtime_error("unexpected leave actor");
    active->leave(active->state_id(decode_state(state)));
    write_links();
}
void* __attribute__((thiscall)) destroy_adapter(void* state, std::uint32_t flags) {
    active->destroy(active->state_id(decode_state(state)), flags);
    write_links();
    return state;
}
void* __attribute__((thiscall)) release_adapter(void* node, std::uint32_t flags) {
    if (flags != 1) throw std::runtime_error("unexpected node-delete flag");
    active->release(*decode_node(node));
    write_links();
    return node;
}
void jump(std::uintptr_t address, std::uintptr_t target) {
    auto* destination = reinterpret_cast<std::uint8_t*>(address);
    destination[0] = 0xE9;
    store(destination, 1, static_cast<std::uint32_t>(target - address - 5));
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
    jump(0x42FF50, reinterpret_cast<std::uintptr_t>(&release_adapter));
    vtable[0] = reinterpret_cast<std::uintptr_t>(&destroy_adapter);
    vtable[3] = reinterpret_cast<std::uintptr_t>(&leave_adapter);
    for (auto& state : native_states) store(state.data(), 0, vtable.data());
    DWORD old_protection = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &old_protection)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    using NativeMove = void (__attribute__((thiscall)) *)(void*, double);
    using NativeClear = void (__attribute__((thiscall)) *)(void*);
    const auto native_move = reinterpret_cast<NativeMove>(0x42EE74);
    const auto native_clear = reinterpret_cast<NativeClear>(0x42E78A);
    using NativeFind = void* (__attribute__((thiscall)) *)(void*,std::int32_t);
    using NativeRemove = std::uint8_t (__attribute__((thiscall)) *)(void*,std::int32_t);
    const auto native_find = reinterpret_cast<NativeFind>(0x42ED4D);
    const auto native_remove = reinterpret_cast<NativeRemove>(0x42E7F3);
    std::mt19937_64 random(0x42EE74);
    constexpr std::uint64_t special[] = {0, 0x8000000000000000ULL, 1,
        0x8000000000000001ULL, 0x3FF0000000000000ULL, 0xBFF0000000000000ULL,
        0x7FEFFFFFFFFFFFFFULL, 0xFFEFFFFFFFFFFFFFULL, 0x7FF0000000000000ULL,
        0xFFF0000000000000ULL, 0x7FF8000000001234ULL, 0xFFF8000000004321ULL,
        0x7FF0000000000001ULL, 0xFFF0000000001234ULL};
    for (int trial = 0; trial != 20000; ++trial) {
        lemmings::ActorGeometryState source;
        lemmings::CollisionGridHead grid{};
        grid.height = std::array<int, 4>{1, 8, 96, 65535}[random() % 4];
        const double target = std::bit_cast<double>(trial < 196 ? special[trial % 14] : random());
        source.pair_360[1] = std::bit_cast<double>(trial < 196 ? special[trial / 14] : random());
        native_actor.fill(0xCD);
        store(native_actor.data(), 0x368, source.pair_360[1]);
        store(reinterpret_cast<void*>(0x4C6A10), 0x418, grid.height);
        auto expected = native_actor;
        native_move(native_actor.data(), target);
        lemmings::move_actor_sample_y(source, target, grid);
        store(expected.data(), 0x10, source.position[1]);
        if (expected != native_actor) {
            std::cerr << "move Y mismatch trial " << trial << '\n';
            return 1;
        }
    }
    std::size_t clears = 0;
    for (std::size_t count = 0; count <= 16; ++count) {
        for (unsigned mode = 0; mode <= 4; ++mode) {
            Fixture source, original;
            source.initialize(count, mode);
            original.initialize(count, mode);
            active = &original;
            native_actor.fill(0);
            write_links();
            original.sync = &read_links;
            native_clear(native_actor.data());
            read_links();
            source.run();
            if (source.trace != original.trace || !source.reset_correctly() || !original.reset_correctly()) {
                std::cerr << "extra behavior clear mismatch count=" << count << " mode=" << mode << '\n';
                for (const auto& row : source.trace) {
                    for (auto value : row) std::cerr << value << ' ';
                    std::cerr << '\n';
                }
                std::cerr << "native:\n";
                for (const auto& row : original.trace) {
                    for (auto value : row) std::cerr << value << ' ';
                    std::cerr << '\n';
                }
                return 1;
            }
            ++clears;
        }
    }
    unsigned removals=0;
    for(std::size_t count=0;count<=16;++count)for(unsigned mode=0;mode<8;++mode)for(int type=-1;type<=4;++type) {
        Fixture source,original;source.initialize_removal(count,mode);original.initialize_removal(count,mode);
        active=&original;native_actor.fill(0);write_links();original.sync=&read_links;
        auto* found=native_find(native_actor.data(),type);
        if(original.state_id(decode_state(found))!=source.state_id(lemmings::find_actor_behavior(source.actor,type))) {
            std::cerr<<"behavior lookup mismatch\n";return 1;
        }
        const bool native_result=native_remove(native_actor.data(),type)!=0;read_links();
        const bool source_result=source.remove(type);
        if(native_result!=source_result || source.trace!=original.trace || source.snapshot()!=original.snapshot()) {
            std::cerr<<"behavior removal mismatch count="<<count<<" mode="<<mode<<" type="<<type<<'\n';return 1;
        }
        ++removals;
    }
    unsigned primary_clears=0;
    for(std::size_t count=0;count<=16;++count)for(unsigned mode=0;mode<8;++mode) {
        Fixture source,original;source.initialize_removal(count,mode);original.initialize_removal(count,mode);
        active=&original;native_actor.fill(0);write_links();original.sync=&read_links;
        reinterpret_cast<NativeClear>(0x42E725)(native_actor.data());read_links();
        lemmings::clear_actor_primary_behavior(source.actor);
        if(source.trace!=original.trace || source.snapshot()!=original.snapshot() || source.actor.primary!=nullptr) {
            std::cerr<<"primary clear mismatch count="<<count<<" mode="<<mode<<'\n';return 1;
        }
        ++primary_clears;
    }
    std::cout << "20000 actor Y moves, " << primary_clears << " primary clears, " << clears << " extra-state clears and "<<removals
              <<" behavior lookup/removals matched original bytes; mutation-sensitive callbacks compared\n";
    VirtualFree(image, 0, MEM_RELEASE);
}
