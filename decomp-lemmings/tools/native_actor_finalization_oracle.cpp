#include "lemmings/actor_finalization.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <bit>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

namespace {
using namespace lemmings;
template<class T> T get(const void* p,std::size_t n) {T v;std::memcpy(&v,static_cast<const char*>(p)+n,sizeof(v));return v;}
template<class T> void put(void* p,std::size_t n,T v) {std::memcpy(static_cast<char*>(p)+n,&v,sizeof(v));}
struct Event {int kind,orientation,dir;SampleTriple position;bool operator==(const Event&)const=default;};
struct Scenario {int direction,clip,model_clip;std::uint8_t first,second;bool mutate_query,mutate_model;std::array<double,3> position,next_position;};
Scenario scenario;
std::vector<Event> events;
unsigned queries;
bool valid;
std::array<std::uint8_t,0x3a0> native_actor;
std::array<std::uint8_t,16> native_resource;
std::array<std::uint8_t,0x98*4> native_clips;
void record(int kind,int orientation,int dir,const std::array<double,3>& p) {
    events.push_back({kind,orientation,dir,std::bit_cast<SampleTriple>(p)});
}
std::uint8_t query(ActorFinalizationState& a) {
    record(1,a.orientation_2d8,a.geometry.direction_44,a.geometry.position);
    if(scenario.mutate_query) a.geometry.direction_44=-1;
    return queries++ == 0 ? scenario.first : scenario.second;
}
void position(ActorFinalizationState& a) {
    record(2,a.orientation_2d8,a.geometry.direction_44,a.geometry.position);
    if(scenario.mutate_model) {
        a.geometry.position=scenario.next_position;
        a.animation.clip_1c8=scenario.model_clip;
    }
}
void effect(ActorFinalizationState& a) {record(3,a.orientation_2d8,a.geometry.direction_44,a.geometry.position);}
ActorFinalizationState native_view() {
    ActorFinalizationState a;
    a.geometry.position=get<std::array<double,3>>(native_actor.data(),8);
    a.geometry.direction_44=get<int>(native_actor.data(),0x44);
    a.orientation_2d8=get<int>(native_actor.data(),0x2d8);
    a.animation.clip_1c8=get<int>(native_actor.data(),0x2f0);
    return a;
}
std::uint32_t __attribute__((thiscall)) native_query(void* p,int type) {
    valid &= p==native_actor.data() && type==0x2b;
    auto a=native_view();const auto result=query(a);
    put(native_actor.data(),0x44,a.geometry.direction_44);
    return 0x87654300|result; // Caller checks only low byte.
}
void __attribute__((thiscall)) native_position(void* p,const void* xyz) {
    valid &= p==native_actor.data()+0x78 && xyz==native_actor.data()+8;
    auto a=native_view();position(a);
    put(native_actor.data(),8,a.geometry.position);put(native_actor.data(),0x2f0,a.animation.clip_1c8);
}
std::uint32_t __attribute__((thiscall)) native_effect(void* p) {
    valid &= p==reinterpret_cast<void*>(0x12345678);
    auto a=native_view();effect(a);return 0xabcdef00;
}
void jump(std::uintptr_t from,std::uintptr_t to) {
    *reinterpret_cast<std::uint8_t*>(from)=0xe9;
    put(reinterpret_cast<void*>(from),1,static_cast<std::uint32_t>(to-from-5));
}
bool read_blocks() {
    std::uint32_t count;std::cin.read(reinterpret_cast<char*>(&count),4);
    while(count--) {
        std::uint32_t addr,size;std::cin.read(reinterpret_cast<char*>(&addr),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(addr<0x400000 || size>0xf0000 || addr+size>0x4f0000) return false;
        std::cin.read(reinterpret_cast<char*>(addr),size);
    }
    return !!std::cin;
}
}
int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xf0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000)) return 3;
    if(!read_blocks())return 2;
    jump(0x42ecc9,reinterpret_cast<std::uintptr_t>(&native_query));
    jump(0x458186,reinterpret_cast<std::uintptr_t>(&native_effect));
    DWORD previous; if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&previous))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    std::array<std::uintptr_t,3> vtable{0,0,reinterpret_cast<std::uintptr_t>(&native_position)};
    std::array<AnimationClip,4> clips{{{1,1,"walk"},{1,1,"drown"},{1,1,"Drown"},{1,1,std::string("drown\0ignored",13)}}};
    AnimationResource resource{clips};
    for(unsigned i=0;i<clips.size();++i)std::memcpy(native_clips.data()+i*0x98+0x18,clips[i].name.c_str(),clips[i].name.size()+1);
    put(native_resource.data(),0,4);put(native_resource.data(),12,native_clips.data());
    WorldGeometryPrefix world;world.byte_array.resize(128*8);
    std::mt19937 random(0x42e58c);
    for(auto& b:world.byte_array)b=static_cast<std::uint8_t>(random());
    for(auto& b:world.fixed_bytes)b=static_cast<std::uint8_t>(random());
    put(reinterpret_cast<void*>(0x4c6a10),0x670,world.byte_array.data());
    std::memcpy(reinterpret_cast<void*>(0x4c6a10+0x678),world.fixed_bytes.data(),6144);
    const auto table=world_sample_table(world);
    world.leading_value=128;world.first_blocks.resize(8);
    for(auto& row:world.first_blocks)for(auto& point:row)for(auto& bits:point)
        bits=std::bit_cast<std::uint64_t>(static_cast<double>(static_cast<int>(random()%200000)-100000)/317.0);
    const auto mesh=world_coordinate_mesh(world);
    put(reinterpret_cast<void*>(0x4c6a10),0x24,world.first_blocks.data());
    put(reinterpret_cast<void*>(0x4c6a10),0x28,mesh.height);
    const double special[]={0.0,-0.0,10.75,255.99,-1.75,65537.125,
        std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),9223372036854775808.0};
    const int dirs[]={1,-1,0,2,std::numeric_limits<int>::min()};
    const std::size_t offsets[]={0x168,0x1d0,0x238,0x2a0};
    for(unsigned trial=0;trial<20000;++trial) {
        scenario.direction=dirs[trial%5];scenario.clip=trial%4;scenario.model_clip=(trial/3)%4;
        scenario.first=trial%3==0?0:trial%3==1?1:255;scenario.second=trial%2?0:128;
        scenario.mutate_query=trial%7==0;scenario.mutate_model=trial%3==0;
        scenario.position={special[trial%10],static_cast<double>(trial%8)+0.125,0.5};
        scenario.next_position={special[(trial/10)%10],trial%9==0?special[6]:static_cast<double>((trial/7)%8),0.75};
        const bool concrete_model=trial%2==0;
        if(concrete_model) {
            scenario.position[0]=static_cast<double>(static_cast<int>(random()%300000)-150000)/271.0;
            scenario.mutate_model=false;
        }
        vtable[2]=concrete_model ? 0x4658d6 : reinterpret_cast<std::uintptr_t>(&native_position);
        native_actor.fill(0xa5);
        put(native_actor.data(),8,scenario.position);put(native_actor.data(),0x44,scenario.direction);
        put(native_actor.data(),0x50,trial%2?1:2);put(native_actor.data(),0x78,vtable.data());
        put(native_actor.data(),0x2d8,-99);put(native_actor.data(),0x2f0,scenario.clip);
        put(native_actor.data(),0x320,native_resource.data());put(native_actor.data(),0x344,std::uint32_t{0x12345678});
        put(native_actor.data(),0x378,native_actor.data()+0x380);
        auto expected=native_actor;
        ActorFinalizationState a;
        a.geometry.position=scenario.position;a.geometry.direction_44=scenario.direction;
        a.orientation_2d8=-99;a.animation.resource_1f8=&resource;a.animation.clip_1c8=scenario.clip;
        a.model_position_108=get<std::array<double,3>>(native_actor.data(),0x108);
        for(unsigned i=0;i<4;++i)a.model_triples[i]=get<SampleTriple>(native_actor.data(),offsets[i]);
        queries=0;events.clear();valid=true;
        using Update=unsigned char(__attribute__((thiscall))*)(void*,double);
        const auto returned=reinterpret_cast<Update>(0x42e2bf)(native_actor.data(),0.0);
        const auto native_events=events;const auto native_queries=queries;
        queries=0;events.clear();
        ActorFinalizationHooks hooks;
        hooks.context=const_cast<WorldCoordinateMesh*>(&mesh);
        hooks.has_behavior=[](void*,ActorFinalizationState& a,int type)->std::uint8_t{valid &= type==0x2b;return query(a);};
        hooks.model_position_08=[](void*,ActorFinalizationState& a){position(a);};
        if(concrete_model) hooks.model_position_08=[](void* p,ActorFinalizationState& a){position_actor_model(a,*static_cast<WorldCoordinateMesh*>(p));};
        hooks.effect_458186=[](void*,ActorFinalizationState& a){effect(a);};
        finalize_actor_update(a,table,hooks);
        put(expected.data(),8,a.geometry.position);put(expected.data(),0x44,a.geometry.direction_44);
        put(expected.data(),0x2d8,a.orientation_2d8);put(expected.data(),0x2f0,a.animation.clip_1c8);
        put(expected.data(),0x108,a.model_position_108);
        for(unsigned i=0;i<4;++i)put(expected.data(),offsets[i],a.model_triples[i]);
        if(returned!=1 || !valid || native_events!=events || queries!=native_queries || native_actor!=expected) {
            std::cerr << "finalization mismatch " << trial << " events " << native_events.size() << '/' << events.size() << '\n';return 1;
        }
    }
    using Sample=const SampleTriple*(__attribute__((thiscall))*)(void*,std::uint32_t,std::int32_t);
    for(unsigned trial=0;trial<20000;++trial) {
        const auto x=static_cast<std::uint32_t>(random());
        const auto y=std::bit_cast<std::int32_t>((static_cast<std::uint32_t>(random())&0xff000000u)|(random()%8));
        const auto* native=reinterpret_cast<Sample>(0x403f40)(reinterpret_cast<void*>(0x4c6a10),x,y);
        if(*native!=world_sample_triple(table,x,y)) {std::cerr << "world sample mismatch " << trial << '\n';return 1;}
    }
    VirtualFree(image,0,MEM_RELEASE);
    std::cout << "native actor finalization passed 20000 full-tail traces (10000 original model positions) and 20000 world samples\n";
}
