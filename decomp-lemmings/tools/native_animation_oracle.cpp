#include "lemmings/animation_playback.hpp"
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
using Bytes=std::array<std::uint8_t,0x2d0>;
std::array<lemmings::AnimationResource,2> resources;
std::array<std::array<lemmings::AnimationClip,4>,2> clips;
std::array<std::array<std::uint8_t,0x10>,2> native_resources;
std::array<std::array<std::uint8_t,0x98*4>,2> native_clips;
std::vector<int> trace;
template<class T,class B> void put(B& bytes,std::size_t offset,T value) {
    std::memcpy(bytes.data()+offset,&value,sizeof(value));
}
const lemmings::AnimationResource* acquire(void*,std::int32_t id) {
    trace.push_back(100+id);return &resources.at(id);
}
void release(void*,std::int32_t id) {trace.push_back(200+id);}
void* __attribute__((fastcall)) native_acquire(void*,void*,std::int32_t id,void*) {
    trace.push_back(100+id);return native_resources.at(id).data();
}
void __attribute__((fastcall)) native_release(void*,void*,std::int32_t id) {
    trace.push_back(200+id);
}
void jump(std::uintptr_t address,void* destination) {
    auto* bytes=reinterpret_cast<std::uint8_t*>(address);bytes[0]=0xe9;
    const auto displacement=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(destination)-address-5);
    std::memcpy(bytes+1,&displacement,4);
}
Bytes mapped(const lemmings::AnimationPlayback& state) {
    Bytes bytes{};
    put(bytes,0x270,state.fraction_1c0);put(bytes,0x278,state.clip_1c8);
    put(bytes,0x27c,state.frame_1cc);put(bytes,0x280,state.direction_1d0);
    void* resource=nullptr;
    if(state.resource_1f8) resource=native_resources[state.resource_1f8-resources.data()].data();
    put(bytes,0x2a8,resource);put(bytes,0x2ac,state.resource_id_1fc);
    put(bytes,0x2b0,state.finished_200);put(bytes,0x2b1,state.looping_201);
    return bytes;
}
bool same(const Bytes& native,const lemmings::AnimationPlayback& state,int trial) {
    auto expected=mapped(state);
    // NaN payload propagation is host dependent; compare NaNs as a class.
    double fraction=0;std::memcpy(&fraction,native.data()+0x270,8);
    if(std::isnan(fraction)&&std::isnan(state.fraction_1c0)) put(expected,0x270,fraction);
    if(native!=expected) {
        for(unsigned i=0;i<native.size();++i) if(native[i]!=expected[i]) {
            std::cerr<<"animation state mismatch trial "<<trial<<" offset "<<std::hex<<i<<"\n";break;
        }
        return false;
    }
    return true;
}
bool same_number(long double a,long double b) {
    return (std::isnan(a)&&std::isnan(b)) ||
        (a==b && (a!=0 || std::signbit(a)==std::signbit(b)));
}
}
int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=static_cast<std::uint8_t*>(VirtualAlloc(reinterpret_cast<void*>(0x400000),
        0xC0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
    if(image!=reinterpret_cast<void*>(0x400000)) return 3;
    std::uint32_t blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);
    if(!std::cin||blocks>64) return 2;
    for(std::uint32_t i=0;i<blocks;++i) {
        std::uint32_t address=0,size=0;
        std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0xC0000||address-0x400000>0xC0000-size) return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);if(!std::cin) return 2;
    }
    jump(0x42ff90,reinterpret_cast<void*>(&native_acquire));
    jump(0x430050,reinterpret_cast<void*>(&native_release));
    DWORD old=0;if(!VirtualProtect(image,0xC0000,PAGE_EXECUTE_READ,&old)) return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xC0000);
    using Advance=void(__attribute__((thiscall))*)(void*,double);
    using Select=void(__attribute__((thiscall))*)(void*,const char*);
    using SetIndex=void(__attribute__((thiscall))*)(void*,std::int32_t);
    using SetLoop=void(__attribute__((thiscall))*)(void*,std::uint8_t);
    using Query=long double(__attribute__((thiscall))*)(void*);
    using Flag=std::uint8_t(__attribute__((thiscall))*)(void*);
    auto advance=reinterpret_cast<Advance>(0x43cf30);
    auto select=reinterpret_cast<Select>(0x43cf53);
    auto index=reinterpret_cast<SetIndex>(0x43cf72);
    auto fraction=reinterpret_cast<Advance>(0x43d030);
    auto bind=reinterpret_cast<SetIndex>(0x46836d);
    auto loop=reinterpret_cast<SetLoop>(0x43cfc8);
    auto duration=reinterpret_cast<Query>(0x43cefe);
    auto time=reinterpret_cast<Query>(0x43cf17);
    auto finished=reinterpret_cast<Flag>(0x43cfaf);
    std::mt19937_64 rng(0x468833);
    const lemmings::AnimationResourceHooks hooks{nullptr,acquire,release};
    const double nan=std::numeric_limits<double>::quiet_NaN();
    const double inf=std::numeric_limits<double>::infinity();
    const std::array<double,10> elapsed_values{0,-0.0,-1,0.06,std::nextafter(0.06,inf),
        std::nextafter(0.06,0.0),0.12,1.2,nan,-2.0};
    const std::array<const char*,6> names{"walk","fall","walk","", "WALK", "missing"};
    for(int trial=0;trial<30000;++trial) {
        for(int resource=0;resource<2;++resource) {
            native_clips[resource].fill(0);native_resources[resource].fill(0);
            for(int entry=0;entry<4;++entry) {
                auto& c=clips[resource][entry];
                c.frame_count=1+static_cast<int>(rng()%50);
                c.speed=static_cast<double>(rng()%301)/100.0-0.5;c.name=names[entry];
                const auto offset=entry*0x98;
                put(native_clips[resource],offset,c.frame_count);
                put(native_clips[resource],offset+0x10,c.speed);
                std::memcpy(native_clips[resource].data()+offset+0x18,c.name.c_str(),c.name.size()+1);
            }
            resources[resource].clips=clips[resource];
            put(native_resources[resource],0,std::int32_t{4});
            put(native_resources[resource],12,native_clips[resource].data());
        }
        lemmings::AnimationPlayback state;
        state.resource_id_1fc=trial%2;
        state.resource_1f8=trial%7 ? &resources[trial%2] : nullptr;
        state.clip_1c8=trial%4;state.frame_1cc=static_cast<std::int32_t>(rng()%55)-2;
        state.direction_1d0=trial%3-1; // include zero and exact +1
        state.looping_201=static_cast<std::uint8_t>(trial%3);
        state.finished_200=42;
        state.fraction_1c0=static_cast<double>(rng()%700)/10000.0;
        if(trial%100==0) state.frame_1cc=INT32_MAX;
        if(trial%100==1) state.frame_1cc=INT32_MIN;
        auto native=mapped(state);
        const double dt=trial%4 ? static_cast<double>(rng()%2000)/1000.0 : elapsed_values[(trial/4)%elapsed_values.size()];
        trace.clear();advance(native.data(),dt);const auto nt=trace;
        trace.clear();lemmings::advance_animation(state,dt,hooks);
        if(nt!=trace || !same(native,state,trial)) return 1;
        if(!same_number(duration(native.data()),lemmings::animation_duration(state)) ||
           !same_number(time(native.data()),lemmings::animation_time(state)) ||
           finished(native.data())!=lemmings::animation_finished(state)) {
            std::cerr<<"animation query mismatch "<<trial<<"\n";return 1;
        }
        const char* name=names[trial%names.size()];
        select(native.data(),name);lemmings::select_animation(state,name);
        if(!same(native,state,trial)) return 1;
        index(native.data(),trial%4);lemmings::select_animation_index(state,trial%4);
        fraction(native.data(),dt);lemmings::set_animation_fraction(state,dt);
        loop(native.data(),static_cast<std::uint8_t>(trial));
        lemmings::set_animation_looping(state,static_cast<std::uint8_t>(trial));
        if(!same(native,state,trial)) return 1;
        if(trial%13==0) {
            state.resource_id_1fc=trial%2 ? -1 : 0;native=mapped(state);
            trace.clear();bind(native.data()+0xb0,1);const auto bt=trace;
            trace.clear();lemmings::bind_animation_resource(state,1,hooks);
            if(bt!=trace || !same(native,state,trial)) return 1;
        }
    }
    std::cout<<"30000 animation advances, query sets and selections match original bytes; resource trace matches\n";
}
