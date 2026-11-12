#include "lemmings/actor_base.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <bit>
#include <cstring>
#include <iostream>
#include <random>

namespace {
using namespace lemmings;
using PositionBits=std::array<std::uint64_t,3>;
template<class T> T get(const void* p,unsigned n){T v;std::memcpy(&v,static_cast<const char*>(p)+n,sizeof(v));return v;}
template<class T> void put(void* p,unsigned n,const T& v){std::memcpy(static_cast<char*>(p)+n,&v,sizeof(v));}
ActorBaseState read(void* p) {
    ActorBaseState a;a.original_vtable=get<std::uint32_t>(p,0);
    a.position=get<std::array<double,3>>(p,8);a.field_20=get<double>(p,0x20);a.flag_28=get<std::uint8_t>(p,0x28);
    a.field_30=get<double>(p,0x30);a.flag_38=get<std::uint8_t>(p,0x38);a.id_3a=get<std::uint16_t>(p,0x3a);return a;
}
void write(void* p,const ActorBaseState& a) {
    put(p,0,a.original_vtable);put(p,8,a.position);put(p,0x20,a.field_20);put(p,0x28,a.flag_28);
    put(p,0x30,a.field_30);put(p,0x38,a.flag_38);put(p,0x3a,a.id_3a);
}
struct Snapshot {
    std::uint32_t vtable;PositionBits position;std::uint64_t field20,field30;
    std::uint8_t flag28,flag38;std::uint16_t id,next;
    bool operator==(const Snapshot&)const=default;
};
Snapshot snapshot(const ActorBaseState& a,std::uint16_t next) {
    return {a.original_vtable,std::bit_cast<PositionBits>(a.position),std::bit_cast<std::uint64_t>(a.field_20),
        std::bit_cast<std::uint64_t>(a.field_30),a.flag_28,a.flag_38,a.id_3a,next};
}
struct Registry {
    ActorBaseState* expected=nullptr;std::uint16_t* next=nullptr;Snapshot seen{};
    unsigned script=0,calls=0;bool valid=true;
    void insert(ActorBaseState*& p) {
        valid &= p==expected;seen=snapshot(*p,*next);++calls;
        if(script&1) {
            p->original_vtable=0x11110000;p->position={-0.0,9.25,-8.0};
            p->field_20=44.5;p->field_30=-7.5;p->flag_28=19;p->flag_38=25;p->id_3a=567;
        }
        if(script&2)*next=89;
        if(script&4)p=nullptr;
    }
    static void callback(void* context,ActorBaseState*& p){static_cast<Registry*>(context)->insert(p);}
};
void* native_actor;
Registry* native_registry;
void __attribute__((thiscall)) registry_adapter(void* registry,void** p) {
    native_registry->valid &= registry==reinterpret_cast<void*>(0x4cbb18) && *p==native_actor;
    auto actor=read(native_actor);auto next=get<std::uint16_t>(reinterpret_cast<void*>(0x4c55ec),0);
    native_registry->expected=&actor;native_registry->next=&next;auto* argument=&actor;
    native_registry->insert(argument);write(native_actor,actor);put(reinterpret_cast<void*>(0x4c55ec),0,next);
    if(!argument)*p=nullptr;
}
void jump(unsigned address,void* target) {
    put(reinterpret_cast<void*>(address),0,std::uint8_t{0xe9});
    put(reinterpret_cast<void*>(address),1,static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-address-5));
}
}
int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xd0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);if(!std::cin || blocks>16)return 2;
    for(unsigned i=0;i<blocks;++i) {
        unsigned a=0,n=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);
        if(a<0x400000 || n>0xd0000 || a-0x400000>0xd0000-n)return 2;
        std::cin.read(reinterpret_cast<char*>(a),n);if(!std::cin)return 2;
    }
    jump(0x4233a0,reinterpret_cast<void*>(&registry_adapter));
    DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Constructor=void*(__attribute__((thiscall))*)(void*,const void*);
    const auto constructor=reinterpret_cast<Constructor>(0x4230a2);
    std::mt19937_64 random(0x4230a2);
    for(unsigned trial=0;trial<20000;++trial) {
        std::array<std::uint8_t,0x40> raw;for(auto& byte:raw)byte=static_cast<std::uint8_t>(random());
        auto actor=read(raw.data()); // Initially dirty, including padding.
        // These fields are always overwritten before callback observation.
        actor.field_20=3.5;actor.field_30=-9;write(raw.data(),actor);
        auto expected=raw;native_actor=raw.data();
        PositionBits position_bits{random(),random(),random()};
        if(trial==0)position_bits={0x8000000000000000ULL,0x7ff0000000000001ULL,0xfff0000000000000ULL};
        const auto position=std::bit_cast<std::array<double,3>>(position_bits);
        const bool alias=trial&8;
        std::uint16_t next=trial<3?std::array<std::uint16_t,3>{0,32767,65535}[trial]:static_cast<std::uint16_t>(random());
        put(reinterpret_cast<void*>(0x4c55ec),0,next);
        Registry native;native.script=trial%8;native_registry=&native;
        auto* returned=constructor(raw.data(),alias?static_cast<const void*>(raw.data()+8):static_cast<const void*>(position.data()));
        Registry source;source.script=native.script;source.expected=&actor;source.next=&next;
        auto& source_return=construct_actor_base(actor,alias?actor.position:position,next,{&source,Registry::callback});
        write(expected.data(),actor);
        if(returned!=raw.data() || &source_return!=&actor || !native.valid || !source.valid ||
            native.calls!=1 || source.calls!=1 || native.seen!=source.seen || raw!=expected ||
            get<std::uint16_t>(reinterpret_cast<void*>(0x4c55ec),0)!=next) {
            std::cerr<<"base constructor mismatch "<<trial<<" alias="<<alias<<" script="<<native.script<<'\n';return 1;
        }
    }
    std::cout<<"native actor base passed 20000 constructions, registry traces and full-buffer comparisons\n";
    VirtualFree(image,0,MEM_RELEASE);
}
