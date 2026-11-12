#include "../tests/actor_construction_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>

namespace {
using namespace construction_fixture;
template<class T> void transfer(void* raw,unsigned offset,T& value,bool write) {
    if(write)std::memcpy(static_cast<char*>(raw)+offset,&value,sizeof(value));
    else std::memcpy(&value,static_cast<char*>(raw)+offset,sizeof(value));
}
void project_model(void* raw,ModelConstructionState& model,bool write) {
    transfer(raw,0,model.original_vtable,write);transfer(raw,4,model.unknown_04,write);
    transfer(raw,8,model.buffer_08,write);transfer(raw,0x88,model.mode_88,write);
    transfer(raw,0x8c,model.unknown_8c,write);transfer(raw,0x90,model.position_90,write);
    transfer(raw,0xa8,model.field_a8,write);transfer(raw,0xac,model.unknown_ac,write);
    auto* p=static_cast<char*>(raw)+0xb0;auto& a=model.animation_b0;
    transfer(p,0,a.quad.field_00,write);transfer(p,4,a.quad.unknown_04,write);
    for(unsigned i=0;i<4;++i) {
        auto* c=p+8+i*0x68;auto& corner=a.quad.corners_08[i];
        transfer(c,0,corner.triple_00,write);transfer(c,0x18,corner.pair_18,write);
        transfer(c,0x28,corner.pair_28,write);transfer(c,0x38,corner.triple_38,write);
        transfer(c,0x50,corner.field_50,write);transfer(c,0x58,corner.unknown_58,write);
    }
    transfer(p,0x1a8,a.field_1a8,write);transfer(p,0x1ac,a.unknown_1ac,write);
    transfer(p,0x1b0,a.field_1b0,write);transfer(p,0x1b4,a.unknown_1b4,write);
    transfer(p,0x1b8,a.field_1b8,write);transfer(p,0x1bc,a.unknown_1bc,write);
    transfer(p,0x1c0,a.playback.fraction_1c0,write);transfer(p,0x1c8,a.playback.clip_1c8,write);
    transfer(p,0x1cc,a.playback.frame_1cc,write);transfer(p,0x1d0,a.playback.direction_1d0,write);
    transfer(p,0x1d4,a.unknown_1d4,write);transfer(p,0x1d8,a.field_1d8,write);transfer(p,0x1e0,a.triple_1e0,write);
    transfer(p,0x1f8,a.playback.resource_1f8,write);transfer(p,0x1fc,a.playback.resource_id_1fc,write);
    transfer(p,0x200,a.playback.finished_200,write);transfer(p,0x201,a.playback.looping_201,write);
    transfer(raw,0x2b2,model.unknown_2b2,write);transfer(raw,0x2b8,model.flag_2b8,write);
    transfer(raw,0x2b9,model.unknown_2b9,write);transfer(raw,0x2c0,model.field_2c0,write);transfer(raw,0x2c4,model.field_2c4,write);
}
template<class T> T get(const void* p,unsigned offset) {T value;std::memcpy(&value,static_cast<const char*>(p)+offset,sizeof(value));return value;}
template<class T> void put(void* p,unsigned offset,const T& value) {std::memcpy(static_cast<char*>(p)+offset,&value,sizeof(value));}
ActorConstructionState read_actor(void* raw) {
    ActorConstructionState a;
    a.original_vtable=get<std::uint32_t>(raw,0);a.geometry.position=get<std::array<double,3>>(raw,8);
    a.geometry.direction_40=get<int>(raw,0x40);a.geometry.direction_44=get<int>(raw,0x44);
    a.climb_48=get<std::uint8_t>(raw,0x48);a.umbrella_49=get<std::uint8_t>(raw,0x49);a.flag_4a=get<std::uint8_t>(raw,0x4a);
    a.geometry.category_4c=get<int>(raw,0x4c);a.mode_50=get<int>(raw,0x50);a.gadget_54=get<void*>(raw,0x54);
    a.geometry.flag_5c=get<std::uint8_t>(raw,0x5c);a.forced_5d=get<std::uint8_t>(raw,0x5d);
    a.flag_5e=get<std::uint8_t>(raw,0x5e);a.flag_5f=get<std::uint8_t>(raw,0x5f);a.flag_60=get<std::uint8_t>(raw,0x60);
    a.pair_68=get<std::array<double,2>>(raw,0x68);a.primary_340=get<ActorBehavior*>(raw,0x340);
    a.shadow_344=get<void*>(raw,0x344);a.unknown_348=get<std::array<std::uint64_t,1>>(raw,0x348);
    a.geometry.pair_350=get<std::array<double,2>>(raw,0x350);a.geometry.pair_360=get<std::array<double,2>>(raw,0x360);
    a.flag_370=get<std::uint8_t>(raw,0x370);a.flag_38c=get<std::uint8_t>(raw,0x38c);
    a.unknown_pair_390=get<std::array<std::uint64_t,2>>(raw,0x390);return a;
}
void write_actor(void* raw,const ActorConstructionState& a) {
    put(raw,0,a.original_vtable);put(raw,8,a.geometry.position);
    put(raw,0x40,a.geometry.direction_40);put(raw,0x44,a.geometry.direction_44);
    put(raw,0x48,a.climb_48);put(raw,0x49,a.umbrella_49);put(raw,0x4a,a.flag_4a);
    put(raw,0x4c,a.geometry.category_4c);put(raw,0x50,a.mode_50);put(raw,0x54,a.gadget_54);
    put(raw,0x5c,a.geometry.flag_5c);put(raw,0x5d,a.forced_5d);put(raw,0x5e,a.flag_5e);put(raw,0x5f,a.flag_5f);put(raw,0x60,a.flag_60);
    put(raw,0x68,a.pair_68);put(raw,0x340,a.primary_340);put(raw,0x344,a.shadow_344);put(raw,0x348,a.unknown_348);
    put(raw,0x350,a.geometry.pair_350);put(raw,0x360,a.geometry.pair_360);put(raw,0x370,a.flag_370);put(raw,0x38c,a.flag_38c);put(raw,0x390,a.unknown_pair_390);
}
Effects* current;
void* raw_actor;
std::array<std::uintptr_t,3> model_vtable;
void load() {
    current->actor=read_actor(raw_actor);
    current->resources=get<std::array<std::int32_t,4>>(reinterpret_cast<void*>(0x4e335c),0);
    current->counts=get<std::array<std::uint16_t,4>>(reinterpret_cast<void*>(0x4cba78),0);
}
void save() {
    write_actor(raw_actor,current->actor);
    put(reinterpret_cast<void*>(0x4e335c),0,current->resources);
    put(reinterpret_cast<void*>(0x4cba78),0,current->counts);
}
void* __attribute__((thiscall)) base_constructor(void* a,const std::array<double,3>* p) {
    load();current->valid &= a==raw_actor;current->construct_base_4230a2(current->actor,*p);save();return a;
}
void* __attribute__((thiscall)) model_constructor(void* m) {
    load();current->valid &= m==static_cast<char*>(raw_actor)+0x78;current->construct_model_43ce00(current->actor);save();
    put(m,0,model_vtable.data());return m;
}
void* __attribute__((thiscall)) list_constructor(void* l) {
    load();current->valid &= l==static_cast<char*>(raw_actor)+0x374;current->construct_extra_list_430090(current->actor);save();return l;
}
void* allocation_adapter(unsigned n) {load();auto* p=current->allocate_49c1a8(n);save();return p;}
void* __attribute__((thiscall)) shadow_constructor(void* storage,void* a) {
    load();current->valid &= a==raw_actor;auto* p=current->construct_shadow_457f40(storage,current->actor);save();return p;
}
void __attribute__((thiscall)) bind_resource(void* resource,int id) {
    load();current->valid &= resource==static_cast<char*>(raw_actor)+0x128;current->bind_resource_46836d(current->actor,id);save();
}
void __attribute__((thiscall)) refresh_adapter(void* world) {
    load();current->valid &= world==reinterpret_cast<void*>(0x4c6a08);current->refresh_444abc();save();
}
ActorBehavior* __attribute__((thiscall)) monitor_constructor(void* storage) {
    load();auto* p=current->construct_monitor_4151f0(storage);save();return p;
}
ActorBehavior* __attribute__((thiscall)) falling_constructor(void* storage) {
    load();auto* p=current->construct_falling_415fa0(storage);save();return p;
}
void __attribute__((thiscall)) attach(void* a,ActorBehavior* state) {
    load();current->valid &= a==raw_actor;current->attach_42e696(current->actor,state);save();
}
void __attribute__((thiscall)) sync_model(void* m,void* position) {
    load();current->valid &= m==static_cast<char*>(raw_actor)+0x78 && position==static_cast<char*>(raw_actor)+8;
    current->sync_model_position_08(current->actor,current->actor.geometry.position);save();
}
void __attribute__((thiscall)) register_actor(void* world,void* a) {
    load();current->valid &= world==reinterpret_cast<void*>(0x4c6a10) && a==raw_actor;current->register_actor_43b517(current->actor);save();
}
void jump(unsigned address,void* target) {
    put(reinterpret_cast<void*>(address),0,std::uint8_t{0xe9});
    put(reinterpret_cast<void*>(address),1,static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-address-5));
}
}
int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xf0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);if(!std::cin || blocks>64)return 2;
    for(unsigned i=0;i<blocks;++i) {
        unsigned a=0,n=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);
        if(a<0x400000 || n>0xf0000 || a-0x400000>0xf0000-n)return 2;
        std::cin.read(reinterpret_cast<char*>(a),n);if(!std::cin)return 2;
    }
    const auto original_model_head=get<std::array<std::uint8_t,5>>(reinterpret_cast<void*>(0x43ce00),0);
    jump(0x4230a2,reinterpret_cast<void*>(&base_constructor));jump(0x43ce00,reinterpret_cast<void*>(&model_constructor));
    jump(0x430090,reinterpret_cast<void*>(&list_constructor));jump(0x49c1a8,reinterpret_cast<void*>(&allocation_adapter));
    jump(0x457f40,reinterpret_cast<void*>(&shadow_constructor));jump(0x46836d,reinterpret_cast<void*>(&bind_resource));
    jump(0x444abc,reinterpret_cast<void*>(&refresh_adapter));jump(0x4151f0,reinterpret_cast<void*>(&monitor_constructor));
    jump(0x415fa0,reinterpret_cast<void*>(&falling_constructor));jump(0x42e696,reinterpret_cast<void*>(&attach));
    jump(0x43b517,reinterpret_cast<void*>(&register_actor));
    // Concrete construction installs the original model vtable identity. Its
    // later position-sync slot remains the same declared external adapter.
    put(reinterpret_cast<void*>(0x4b14a8),8,reinterpret_cast<std::uintptr_t>(&sync_model));
    DWORD previous;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&previous))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    model_vtable[2]=reinterpret_cast<std::uintptr_t>(&sync_model);
    using Constructor=void*(__attribute__((thiscall))*)(void*,double,double,std::int32_t);
    const auto constructor=reinterpret_cast<Constructor>(0x42de51);
    std::mt19937_64 random(0x42de51);
    const std::array<double,7> edges{0.0,-0.0,std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::denorm_min(),std::numeric_limits<double>::max()};
    for(unsigned pass=0;pass<2;++pass) {
      if(pass==1) {
        if(!VirtualProtect(reinterpret_cast<void*>(0x43ce00),5,PAGE_EXECUTE_READWRITE,&previous))return 2;
        put(reinterpret_cast<void*>(0x43ce00),0,original_model_head);
        DWORD ignored;if(!VirtualProtect(reinterpret_cast<void*>(0x43ce00),5,previous,&ignored))return 2;
        FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(0x43ce00),5);
      }
      for(unsigned trial=0;trial<10000;++trial) {
        Effects initial;initial.actor=dirty_actor();initial.failure_mask=trial%64;
        initial.concrete_model=pass==1;
        initial.mutation_mask=trial<64?0:trial<128?0xffffffff:static_cast<unsigned>(random());
        initial.actor.mode_50=static_cast<int>(random());initial.actor.geometry.category_4c=static_cast<int>(random());
        initial.actor.unknown_348={random()};initial.actor.unknown_pair_390={random(),random()};
        const auto category=static_cast<int>(trial%4);
        const double x=trial<edges.size()?edges[trial]:double(int(random()%20000)-10000)/8;
        const double y=trial<edges.size()?edges[edges.size()-trial-1]:double(int(random()%20000)-10000)/8;
        Effects native=initial,source=initial;
        std::array<std::uint8_t,0x3a0> raw;for(auto& b:raw)b=static_cast<std::uint8_t>(random());
        raw_actor=raw.data();write_actor(raw_actor,initial.actor);auto expected=raw;
        if(pass==1)project_model(raw.data()+0x78,source.model_state,false);
        current=&native;save();
        auto* returned=constructor(raw_actor,x,y,category);load();
        auto& source_returned=construct_actor(source.actor,x,y,category,source.globals(),source);
        write_actor(expected.data(),source.actor);
        if(pass==1)project_model(expected.data()+0x78,source.model_state,true);
        else put(expected.data(),0x78,model_vtable.data());
        if(returned!=raw_actor || &source_returned!=&source.actor || !native.valid || !source.valid ||
           snapshot(native.actor)!=snapshot(source.actor) || native.events!=source.events ||
           native.counts!=source.counts || native.resources!=source.resources || raw!=expected) {
            std::cerr<<"construction mismatch "<<trial<<" pass="<<pass<<" failure="<<initial.failure_mask<<" mutations="<<initial.mutation_mask<<'\n';
            for(unsigned i=0;i<raw.size();++i)if(raw[i]!=expected[i]){std::cerr<<"byte "<<std::hex<<i<<'\n';break;}
            for(unsigned i=0;i<native.events.size() && i<source.events.size();++i)if(native.events[i]!=source.events[i]){std::cerr<<"event "<<i<<" kind "<<native.events[i].kind<<'\n';break;}
            return 1;
        }
      }
    }
    std::cout<<"native actor construction passed 10000 boundary-adapter + 10000 concrete-model traces and full actor buffers\n";
    VirtualFree(image,0,MEM_RELEASE);
}
