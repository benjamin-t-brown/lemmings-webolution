#include "lemmings/actor_drowning.hpp"
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
#include <string>

namespace {
double* active_elapsed=nullptr;
double* active_remaining=nullptr;
double advanced=0;
bool mutate=false,pointers_ok=true;
std::uint8_t* actor_pointer=nullptr;
std::string selected;
void select(void*,const char* name){selected=name;}
void advance(void*,double elapsed){advanced=elapsed;if(mutate){*active_elapsed=1.5;*active_remaining=0.75;}}
void __attribute__((thiscall)) native_select(void* self,const char* name){pointers_ok&=self==actor_pointer;select(nullptr,name);}
void __attribute__((thiscall)) native_advance(void* self,double elapsed){pointers_ok&=self==actor_pointer+0x78;advance(nullptr,elapsed);}
template<class T,class B>void put(B& bytes,unsigned offset,T value){std::memcpy(bytes.data()+offset,&value,sizeof(value));}
template<class T,class B>T get(const B& bytes,unsigned offset){T value;std::memcpy(&value,bytes.data()+offset,sizeof(value));return value;}
bool same(double a,double b){return(std::isnan(a)&&std::isnan(b))||std::bit_cast<std::uint64_t>(a)==std::bit_cast<std::uint64_t>(b);}
void jump(std::uintptr_t address,void* target){auto* code=reinterpret_cast<std::uint8_t*>(address);code[0]=0xe9;
    const auto delta=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-address-5);std::memcpy(code+1,&delta,4);}
bool load(){std::uint32_t count=0;std::cin.read(reinterpret_cast<char*>(&count),4);
    for(unsigned i=0;i<count;++i){std::uint32_t address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||address>0x4c0000||size>0x4c0000-address)return false;
        std::cin.read(reinterpret_cast<char*>(address),size);}return static_cast<bool>(std::cin);}
}
int main(){
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xc0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    if(!load())return 2;
    jump(0x42fd25,reinterpret_cast<void*>(&native_select));
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    std::array<std::uint8_t,0x100> actor{};actor_pointer=actor.data();std::array<void*,12> table{};
    table[11]=reinterpret_cast<void*>(&native_advance);put(actor,0x78,table.data());
    alignas(double)std::array<std::uint8_t,0x18> state{};
    using Constructor=void*(__attribute__((thiscall))*)(void*);
    using Entry=void(__attribute__((thiscall))*)(void*,void*);
    using Update=std::uint32_t(__attribute__((thiscall))*)(void*,void*,double*);
    const lemmings::ActorDrowningHooks hooks{nullptr,select,advance};
    std::mt19937_64 random(0x408766);
    const double special[]={0,-0.0,1,2,std::nextafter(2.0,0.0),std::nextafter(2.0,3.0),-1,
        std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()};
    for(unsigned trial=0;trial<20000;++trial){
        state.fill(0xa5);reinterpret_cast<Constructor>(0x408700)(state.data());
        lemmings::ActorDrowningState source{99};lemmings::ActorBehavior behavior;lemmings::initialize_actor_drowning(source,behavior);
        auto expected=state;expected.fill(0xa5);put(expected,0,std::uint32_t{0x4b0570});put(expected,8,source.elapsed_08);
        put(expected,0x10,behavior.phase_10);put(expected,0x11,behavior.extra_11);put(expected,0x14,behavior.type_14);
        if(state!=expected)return 1;
        lemmings::enter_actor_drowning(hooks);const auto source_name=selected;selected.clear();
        reinterpret_cast<Entry>(0x40874c)(state.data(),actor.data());if(!pointers_ok||selected!=source_name)return 1;
        reinterpret_cast<Entry>(0x401320)(state.data(),actor.data());if(state!=expected)return 1;
        source.elapsed_08=trial<100?special[(trial/10)%10]:std::bit_cast<double>(random());
        double source_time=trial<100?special[trial%10]:std::bit_cast<double>(random());double native_time=source_time;
        put(state,8,source.elapsed_08);mutate=trial%7==0;const bool alias=trial%4==0;
        double& source_remaining=alias?source.elapsed_08:source_time;
        active_elapsed=&source.elapsed_08;active_remaining=&source_remaining;
        const auto source_result=lemmings::update_actor_drowning(source,source_remaining,hooks);const auto source_advanced=advanced;
        active_elapsed=reinterpret_cast<double*>(state.data()+8);active_remaining=alias?active_elapsed:&native_time;
        const auto native_result=reinterpret_cast<Update>(0x408766)(state.data(),actor.data(),active_remaining);
        if(!pointers_ok||native_result!=source_result||!same(source_advanced,advanced)||
            !same(source_remaining,*active_remaining)||!same(source.elapsed_08,get<double>(state,8))){
            std::cerr<<"drowning mismatch "<<trial<<'\n';return 1;}
    }
    VirtualFree(image,0,MEM_RELEASE);std::cout<<"native drowning passed 20000 constructor/entry/update traces including aliased time\n";
}
