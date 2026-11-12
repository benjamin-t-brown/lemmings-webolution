#include "../tests/actor_destruction_fixture.hpp"


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
destruction_fixture::Fixture* whole;
std::array<void*,4> registry_slots{};
void read_meta();
void write_meta();
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
    write_meta();
}
void* __attribute__((thiscall)) destroy_adapter(void* state, std::uint32_t flags) {
    active->destroy(active->state_id(decode_state(state)), flags);
    write_links();
    write_meta();
    return state;
}
void* __attribute__((thiscall)) release_adapter(void* node, std::uint32_t flags) {
    if (flags != 1) throw std::runtime_error("unexpected node-delete flag");
    active->release(*decode_node(node));
    write_links();
    write_meta();
    return node;
}
void jump(std::uintptr_t address, std::uintptr_t target) {
    auto* destination = reinterpret_cast<std::uint8_t*>(address);
    destination[0] = 0xE9;
    store(destination, 1, static_cast<std::uint32_t>(target - address - 5));
}
void read_meta() {
    whole->actor.original_vtable=load<std::uint32_t>(native_actor.data(),0);
    whole->actor.category_4c=load<std::int32_t>(native_actor.data(),0x4c);
    whole->actor.shadow_344=load<void*>(native_actor.data(),0x344);
    whole->clear.actor.identifier_3a=load<std::int16_t>(native_actor.data(),0x3a);
    whole->counts=load<std::array<std::uint16_t,4>>(reinterpret_cast<void*>(0x4cba78),0);
    whole->slots=registry_slots;
}
void write_meta() {
    store(native_actor.data(),0,whole->actor.original_vtable);
    store(native_actor.data(),0x4c,whole->actor.category_4c);
    store(native_actor.data(),0x344,whole->actor.shadow_344);
    store(native_actor.data(),0x3a,whole->clear.actor.identifier_3a);
    store(reinterpret_cast<void*>(0x4cba78),0,whole->counts);
    registry_slots=whole->slots;
}
void read_all(){read_links();read_meta();}
void write_all(){write_links();write_meta();}
void* __attribute__((thiscall)) shadow_adapter(void* shadow,std::uint32_t flags) {
    read_all();whole->destroy_shadow_42fec0(whole->actor,shadow,flags);write_all();return shadow;
}
void __attribute__((thiscall)) refresh_adapter(void* world) {
    read_all();whole->valid&=world==reinterpret_cast<void*>(0x4c6a08);
    whole->refresh_444abc(whole->actor);write_all();
}
void __attribute__((thiscall)) list_adapter(void* list) {
    read_all();whole->valid&=list==native_actor.data()+0x374;
    whole->destroy_extra_list_430110(whole->actor);write_all();
}
void __attribute__((thiscall)) model_adapter(void* model) {
    read_all();whole->valid&=model==native_actor.data()+0x78;
    whole->destroy_model_43ce88(whole->actor);write_all();
}
void __attribute__((thiscall)) world_adapter(void* world,void* actor) {
    read_all();whole->valid&=world==reinterpret_cast<void*>(0x4c6a10) && actor==native_actor.data();
    whole->unregister_world_43b536(whole->actor);write_all();
}
}

int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xd0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);if(!std::cin || blocks>64)return 2;
    for(unsigned i=0;i<blocks;++i) {
        unsigned a=0,n=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);
        if(a<0x400000 || n>0xd0000 || a-0x400000>0xd0000-n)return 2;
        std::cin.read(reinterpret_cast<char*>(a),n);if(!std::cin)return 2;
    }
    jump(0x42ff50,reinterpret_cast<std::uintptr_t>(&release_adapter));
    jump(0x42fec0,reinterpret_cast<std::uintptr_t>(&shadow_adapter));
    jump(0x444abc,reinterpret_cast<std::uintptr_t>(&refresh_adapter));
    jump(0x430110,reinterpret_cast<std::uintptr_t>(&list_adapter));
    jump(0x43ce88,reinterpret_cast<std::uintptr_t>(&model_adapter));
    jump(0x43b536,reinterpret_cast<std::uintptr_t>(&world_adapter));
    vtable[0]=reinterpret_cast<std::uintptr_t>(&destroy_adapter);vtable[3]=reinterpret_cast<std::uintptr_t>(&leave_adapter);
    for(auto& state:native_states)store(state.data(),0,vtable.data());
    DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Destructor=void(__attribute__((thiscall))*)(void*);
    const auto destroy=reinterpret_cast<Destructor>(0x42e17c);
    const auto base_destroy=reinterpret_cast<Destructor>(0x42316c);
    std::mt19937_64 random(0x42e17c);
    for(unsigned trial=0;trial<10000;++trial) {
        destruction_fixture::Fixture source,native;
        const auto count=trial%17;
        const unsigned mode=std::array<unsigned,8>{0,1,2,3,4,11,13,16}[(trial/17)%8];
        const auto first=std::array<std::int32_t,3>{-32768,-2,32764}[trial%3];
        const auto mutations=trial<100?0:static_cast<unsigned>(random());
        source.initialize(count,mode,trial%2,mutations,first);native.initialize(count,mode,trial%2,mutations,first);
        source.clear.actor.identifier_3a=native.clear.actor.identifier_3a=static_cast<std::int16_t>(first+trial%4);
        active=&native.clear;whole=&native;
        for(auto& b:native_actor)b=static_cast<std::uint8_t>(random());
        write_all();const auto before=native_actor;
        // Bias in integer address space: original signed-ID addressing lands
        // inside this four-entry backing array, including IDs -32768/32767.
        const auto origin=reinterpret_cast<std::uintptr_t>(registry_slots.data())-static_cast<std::intptr_t>(first)*4;
        store(reinterpret_cast<void*>(0x4cbb18),0x0c,origin);
        native.clear.sync=[] {read_all();whole->record(destruction_fixture::behavior);};
        const bool base_only=(trial%5)==0;
        (base_only?base_destroy:destroy)(native_actor.data());read_all();
        const auto actual=native_actor;
        if(base_only)lemmings::destroy_actor_base(source.actor,source.registry(),source);
        else lemmings::destroy_actor(source.actor,source.counts,source.registry(),source.hooks(),source);
        const bool equal=native.valid && source.valid && native.events==source.events &&
            native.clear.trace==source.clear.trace && native.snapshot()==source.snapshot();
        active=&source.clear;whole=&source;native_actor=before;write_all();
        if(!equal || actual!=native_actor) {
            std::cerr<<"destructor mismatch trial="<<trial<<" count="<<count<<" mode="<<mode<<" base="<<base_only<<'\n';
            for(unsigned i=0;i<actual.size();++i)if(actual[i]!=native_actor[i]){std::cerr<<"byte "<<std::hex<<i<<'\n';break;}
            for(unsigned i=0;i<native.events.size() && i<source.events.size();++i)if(native.events[i]!=source.events[i]){std::cerr<<"event "<<i<<" kind "<<native.events[i].kind<<'\n';break;}
            return 1;
        }
    }
    std::cout<<"native destruction passed 8000 complete actor + 2000 base-only teardown traces and buffers\n";
    VirtualFree(image,0,MEM_RELEASE);
}
