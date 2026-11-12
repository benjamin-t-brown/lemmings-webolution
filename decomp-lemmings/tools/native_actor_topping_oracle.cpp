#include "lemmings/actor_topping.hpp"
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
using namespace lemmings;
template<class T> T get(const void* p,unsigned n) { T v;std::memcpy(&v,static_cast<const char*>(p)+n,sizeof(v));return v; }
template<class T> void put(void* p,unsigned n,T v) { std::memcpy(static_cast<char*>(p)+n,&v,sizeof(v)); }
std::uint64_t bits(double x) { return std::isnan(x)?0x7ff8000000000000ULL:std::bit_cast<std::uint64_t>(x); }
struct Event { unsigned kind;std::uint64_t x,y,time,budget;int dx,dy;bool operator==(const Event&)const=default; };
std::vector<Event> events;
void* raw_actor;
ToppingActor* source_actor;
double* remaining;
unsigned script;
std::uint8_t finished;
long double model_time;
bool valid;
void record(unsigned kind,double dt=0) {
    const auto x=source_actor?source_actor->geometry.position[0]:get<double>(raw_actor,8);
    const auto y=source_actor?source_actor->geometry.position[1]:get<double>(raw_actor,0x10);
    const auto dx=source_actor?source_actor->geometry.direction_40:get<int>(raw_actor,0x40);
    const auto dy=source_actor?source_actor->geometry.direction_44:get<int>(raw_actor,0x44);
    events.push_back({kind,bits(x),bits(y),bits(dt),bits(*remaining),dx,dy});
}
void mutate(unsigned bit) {
    if(!(script&bit))return;
    *remaining=0.75;
    if(source_actor) {source_actor->geometry.position={255.5,7.5,0.25};source_actor->geometry.direction_40=-1;source_actor->geometry.direction_44=-1;}
    else {put(raw_actor,8,255.5);put(raw_actor,0x10,7.5);put(raw_actor,0x18,0.25);put(raw_actor,0x40,-1);put(raw_actor,0x44,-1);}
}
struct Effects : ActorToppingCallbacks {
    void select_animation_42fd25(ToppingActor&,std::string_view name)override{valid&=name=="flip";record(1);}
    void advance_model_2c(ToppingActor&,double dt)override{record(2,dt);mutate(1);}
    std::uint8_t finished_model_44(ToppingActor&)override{record(3);mutate(2);return finished;}
    long double model_time_48(ToppingActor&)override{record(4);mutate(4);return model_time;}
    void select_landed_42f8b9(ToppingActor& actor)override{record(5);if(script&8)leave_actor_topping(actor);}
} effects;
void __attribute__((thiscall)) animation(void* actor,const char* name){valid&=actor==raw_actor && std::strcmp(name,"flip")==0;record(1);}
void __attribute__((thiscall)) advance(void* model,double dt){valid&=static_cast<char*>(model)==static_cast<char*>(raw_actor)+0x78;record(2,dt);mutate(1);}
std::uint8_t __attribute__((thiscall)) completion(void* model){valid&=static_cast<char*>(model)==static_cast<char*>(raw_actor)+0x78;record(3);mutate(2);return finished;}
long double __attribute__((thiscall)) time_query(void* model){valid&=static_cast<char*>(model)==static_cast<char*>(raw_actor)+0x78;record(4);mutate(4);return model_time;}
void __attribute__((thiscall)) landed(void* actor){valid&=actor==raw_actor;record(5);if(script&8)reinterpret_cast<void(__attribute__((thiscall))*)(void*,void*)>(0x417dd7)(nullptr,actor);}
void jump(unsigned address,void* target){put(reinterpret_cast<void*>(address),0,std::uint8_t{0xe9});put(reinterpret_cast<void*>(address),1,static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-address-5));}
}
int main(){
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xd0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);if(!std::cin || blocks>64)return 2;
    for(unsigned i=0;i<blocks;++i) {
        unsigned a=0,n=0;
        std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);
        if(a<0x400000 || n>0xd0000 || a-0x400000>0xd0000-n)return 2;
        std::cin.read(reinterpret_cast<char*>(a),n);if(!std::cin)return 2;
    }
    jump(0x42fd25,reinterpret_cast<void*>(&animation));jump(0x42f8b9,reinterpret_cast<void*>(&landed));
    DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Enter=void(__attribute__((thiscall))*)(void*,void*);
    using Update=unsigned(__attribute__((thiscall))*)(void*,void*,double*);
    const auto enter=reinterpret_cast<Enter>(0x417ccc),leave=reinterpret_cast<Enter>(0x417dd7);
    const auto update=reinterpret_cast<Update>(0x417ce6);
    std::array<std::uintptr_t,19> vtable{};vtable[11]=reinterpret_cast<std::uintptr_t>(&advance);vtable[17]=reinterpret_cast<std::uintptr_t>(&completion);vtable[18]=reinterpret_cast<std::uintptr_t>(&time_query);
    CollisionGridHead grid{};grid.height=128;std::memcpy(reinterpret_cast<void*>(0x4c6a10),&grid,sizeof(grid));
    std::mt19937_64 random(0x417ce6);unsigned cases=0;
    const double nan=std::numeric_limits<double>::quiet_NaN(),inf=std::numeric_limits<double>::infinity();
    const std::array<double,13> edges{nan,inf,-inf,0,-0.0,-1,255.75,2147483647.0,2147483648.0,-2147483649.0,4294967296.0,9223372036854775808.0,-9223372036854775808.0};
    for(unsigned trial=0;trial<20000;++trial){
        ToppingActor actor;actor.geometry.position={trial<edges.size()?edges[trial]:double(int(random()%10000)-5000)/4,
            trial<edges.size()?edges[edges.size()-trial-1]:double(int(random()%10000)-5000)/4,0.5};
        // Keep the wrapped X integer bounded; conversion edge values above are
        // multiples of 2^32 or within a few steps of the 32-bit boundary.
        actor.geometry.direction_40=int(random()%5)-2;actor.geometry.direction_44=static_cast<int>(random());
        script=trial%16;finished=trial%3?static_cast<std::uint8_t>(random()|1):0;
        model_time=trial<edges.size()?static_cast<long double>(edges[trial]):static_cast<long double>(random()%10000)/3000;
        double time=trial<edges.size()?edges[trial]:double(random()%1000)/1000;
        const double initial_time=time;
        std::array<std::uint8_t,0x340> original{};original.fill(0xa5);raw_actor=original.data();
        put(raw_actor,8,actor.geometry.position);put(raw_actor,0x40,actor.geometry.direction_40);put(raw_actor,0x44,actor.geometry.direction_44);put(raw_actor,0x78,vtable.data());
        auto expected_bytes=original;
        source_actor=nullptr;remaining=&time;events.clear();valid=true;enter(nullptr,raw_actor);
        const auto result=update(nullptr,raw_actor,&time);leave(nullptr,raw_actor);
        const auto native_events=events;const auto native_valid=valid;const double native_time=time;
        time=initial_time;
        source_actor=&actor;remaining=&time;events.clear();valid=true;enter_actor_topping(actor,effects);
        const auto source_result=update_actor_topping(actor,time,grid,effects);leave_actor_topping(actor);
        put(expected_bytes.data(),8,actor.geometry.position);put(expected_bytes.data(),0x40,actor.geometry.direction_40);put(expected_bytes.data(),0x44,actor.geometry.direction_44);
        bool equal_bytes=true;
        for(unsigned i=0;i<original.size();++i)if(i<8 || i>=32)equal_bytes &= original[i]==expected_bytes[i];
        for(unsigned i=0;i<3;++i)equal_bytes &= bits(get<double>(raw_actor,8+8*i))==bits(actor.geometry.position[i]);
        if(!valid || !native_valid || result!=source_result || bits(native_time)!=bits(time) || events!=native_events || !equal_bytes){
            std::cerr<<"topping mismatch "<<trial<<" script="<<script<<" result="<<result<<'/'<<source_result<<'\n';return 1;}
        ++cases;
    }
    std::cout<<"native topping passed "<<cases<<" entry/update/leave traces\n";VirtualFree(image,0,MEM_RELEASE);
}
