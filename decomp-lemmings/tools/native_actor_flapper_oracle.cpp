#include "../tests/actor_flapper_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cstring>
#include <cstdarg>
#include <iostream>
#include <limits>
#include <random>

namespace {
using namespace flapper_test;
Fixture* active;
std::array<unsigned char, 0x18> state;
std::array<unsigned char, 0x370> actor;
std::array<std::uintptr_t, 20> model;
double remaining;
template<class T> void put(void* p, unsigned offset, const T& value) { std::memcpy(static_cast<char*>(p)+offset, &value, sizeof(value)); }
template<class T> void get(const void* p, unsigned offset, T& value) { std::memcpy(&value, static_cast<const char*>(p)+offset, sizeof(value)); }
void transfer(bool read) {
    auto& f = *active;
    auto field = [&](void* raw, unsigned offset, auto& value) { if(read)get(raw,offset,value);else put(raw,offset,value); };
    field(state.data(), 8, f.state.elapsed_08);
    field(actor.data(), 8, f.actor.geometry.position);
    field(actor.data(), 0x3a, f.actor.identifier_3a);
    field(actor.data(), 0x40, f.actor.geometry.direction_40);
    field(actor.data(), 0x44, f.actor.geometry.direction_44);
    field(actor.data(), 0x49, f.actor.umbrella_49);
    field(actor.data(), 0x4b, f.actor.floating_4b);
    field(actor.data(), 0x4c, f.actor.geometry.category_4c);
    field(actor.data(), 0x5c, f.actor.geometry.flag_5c);
    field(actor.data(), 0x350, f.actor.geometry.pair_350);
    field(actor.data(), 0x360, f.actor.geometry.pair_360);
    field(&remaining, 0, f.remaining);
    field(reinterpret_cast<void*>(0x4cbf88), 0, f.global);
}
void __attribute__((thiscall)) advance(void* p,double dt){transfer(true);active->valid &= p==actor.data()+0x78;active->advance_model_2c(active->actor,dt);transfer(false);}
std::uint8_t __attribute__((thiscall)) query(void* p,int type){transfer(true);active->valid &= p==actor.data();auto result=active->has_state_42ecc9(active->actor,type);transfer(false);return result;}
std::uint8_t __attribute__((thiscall)) finished(void* p){transfer(true);active->valid &= p==actor.data()+0x78;auto result=active->finished_model_44(active->actor);transfer(false);return result;}
long double __attribute__((thiscall)) elapsed(void* p){transfer(true);active->valid &= p==actor.data()+0x78;auto result=active->elapsed_model_48(active->actor);transfer(false);return result;}
void __attribute__((thiscall)) animation(void* p,const char* name){transfer(true);active->valid &= p==actor.data();active->select_animation_42fd25(active->actor,name);transfer(false);}
void __attribute__((thiscall)) attach(void* p,ActorBehavior* value){transfer(true);active->valid &= p==actor.data();active->attach_42e696(active->actor,value);transfer(false);}
void* __cdecl allocate(unsigned size){transfer(true);active->valid &= size==0x18;auto* result=active->create_state_4081b0();transfer(false);return result;}
void* __attribute__((thiscall)) construct(void* p){active->valid &= p==&active->next;return p;}
void __cdecl log_adapter(const char* format,int id,unsigned context,...){
    transfer(true);
    if(reinterpret_cast<std::uintptr_t>(format)==0x4bf3bc)active->notice_enter(static_cast<std::int16_t>(id),context);
    else {va_list args;va_start(args,context);const double dt=va_arg(args,double);va_end(args);active->notice_finished(static_cast<std::int16_t>(id),context,dt);}
    transfer(false);
}
void jump(unsigned address,void* target){put(reinterpret_cast<void*>(address),0,std::uint8_t{0xe9});put(reinterpret_cast<void*>(address),1,static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-address-5));}
}
int main(){
    _setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xd0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);if(!std::cin||blocks>64)return 2;
    for(unsigned i=0;i<blocks;++i){unsigned address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0xd0000||address-0x400000>0xd0000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);if(!std::cin)return 2;}
    jump(0x42ecc9,reinterpret_cast<void*>(&query));jump(0x42fd25,reinterpret_cast<void*>(&animation));
    jump(0x42e696,reinterpret_cast<void*>(&attach));jump(0x49c1a8,reinterpret_cast<void*>(&allocate));
    jump(0x4081b0,reinterpret_cast<void*>(&construct));jump(0x40f860,reinterpret_cast<void*>(&log_adapter));
    model[0x2c/4]=reinterpret_cast<std::uintptr_t>(&advance);model[0x44/4]=reinterpret_cast<std::uintptr_t>(&finished);model[0x48/4]=reinterpret_cast<std::uintptr_t>(&elapsed);
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Update=unsigned(__attribute__((thiscall))*)(void*,void*,double*);
    using Action=void(__attribute__((thiscall))*)(void*,void*);
    using Constructor=void*(__attribute__((thiscall))*)(void*);
    auto update=reinterpret_cast<Update>(0x41699e);auto enter=reinterpret_cast<Action>(0x41695c);auto leave=reinterpret_cast<Action>(0x401320);
    auto constructor=reinterpret_cast<Constructor>(0x416910);
    std::mt19937 random(0x41699e);
    for(unsigned test=0;test<16000;++test){
        Fixture f;active=&f;f.script=test%128;f.has=(test%5==0)?0x80:0;f.finished=(test%3)?0xff:0;f.fail=test%7==0;
        f.remaining=(static_cast<int>(random()%2000)-500)/4096.0;f.state.elapsed_08=double(random()%100)/16;
        f.actor.identifier_3a=static_cast<std::int16_t>(random());f.global=random();
        f.actor.geometry.position={double(static_cast<int>(random()%1400)-600)/2,double(static_cast<int>(random()%80)-20)/4,0.5};
        f.actor.geometry.direction_40=(test%2)?1:-1;f.actor.geometry.direction_44=static_cast<int>(test%5)-2;
        f.actor.geometry.category_4c=test%5;f.actor.floating_4b=(test%3)?0:0x80;
        f.actor.geometry.pair_360={double(test%3)/4,double(test%4)/4};
        f.model_elapsed=static_cast<long double>(static_cast<int>(random()%1000)-100)/7000;
        for(auto& cell:f.cells)cell=(test%4==0)?0:((random()%7==0)?0x80:0);
        if(test%101==0)f.remaining=std::numeric_limits<double>::quiet_NaN();
        if(test%103==0)f.model_elapsed=std::numeric_limits<long double>::infinity();
        if(test%107==0)f.actor.geometry.position[1]=std::numeric_limits<double>::quiet_NaN();
        if(test%109==0)f.remaining=std::numeric_limits<double>::infinity();
        if(test%113==0) {
            const std::array<double,8> edge{4294967296.5,-4294967296.5,9223372036854775808.0,
                -9223372036854775808.0, std::numeric_limits<double>::infinity(),
                -std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN(),-0.0};
            f.actor.geometry.position[0]=edge[(test/113)%edge.size()];
        }
        if(test%127==0)f.model_elapsed=std::numeric_limits<long double>::quiet_NaN();
        state.fill(0xa5);actor.fill(0xa5);put(actor.data(),0x78,model.data());transfer(false);
        put(reinterpret_cast<void*>(0x4c6a10),0,f.grid);
        auto initial=f;auto initial_actor=actor;auto initial_state=state;
        const unsigned result=update(state.data(),actor.data(),&remaining);transfer(true);
        const auto native_trace=f.trace;const auto native_actor=actor;const auto native_state=state;const auto native_remaining=bits(remaining);const auto native_global=f.global;const bool valid=f.valid;
        f=initial;f.grid.cells.data=f.cells.data();
        const unsigned expected=update_actor_flapper(f.state,f.actor,f.remaining,f.grid,f.global,f);
        actor=initial_actor;state=initial_state;transfer(false);
        // NaN payload/sign choices of floating arithmetic are not claimed.
        for(unsigned off:{8u}){double a,b;get(native_state.data(),off,a);get(state.data(),off,b);if(std::isnan(a)&&std::isnan(b))put(state.data(),off,a);}
        for(unsigned off:{8u,16u,24u}){double a,b;get(native_actor.data(),off,a);get(actor.data(),off,b);if(std::isnan(a)&&std::isnan(b))put(actor.data(),off,a);}
        if(result!=expected||!valid||!f.valid||native_trace!=f.trace||native_actor!=actor||native_state!=state||native_remaining!=bits(f.remaining)||native_global!=f.global){
            std::cerr<<"flapper update mismatch case="<<test<<" script="<<f.script<<" native/source y=";double y;get(native_actor.data(),16,y);std::cerr<<y<<'/'<<f.actor.geometry.position[1]<<'\n';return 1;}
        // Entry diagnostic callback may change the category before animation.
        f=initial;f.grid.cells.data=f.cells.data();f.trace.clear();transfer(false);enter(state.data(),actor.data());transfer(true);
        const auto entry_trace=f.trace;const auto entry_actor=actor;const bool entry_valid=f.valid;
        f=initial;f.grid.cells.data=f.cells.data();f.trace.clear();enter_actor_flapper(f.actor,f.global,f);transfer(false);
        if(!entry_valid||!f.valid||entry_trace!=f.trace||entry_actor!=actor)return 1;
        const auto before_state=state;const auto before_actor=actor;leave(state.data(),actor.data());leave_actor_flapper();
        if(before_state!=state||before_actor!=actor)return 1;
        state.fill(static_cast<unsigned char>(test));auto expected_state=state;
        ActorBehavior behavior;ActorFlapperState logical;initialize_actor_flapper(logical,behavior);
        put(expected_state.data(),0,std::uint32_t{0x4b0a58});put(expected_state.data(),8,logical.elapsed_08);
        put(expected_state.data(),0x10,behavior.phase_10);put(expected_state.data(),0x11,behavior.extra_11);put(expected_state.data(),0x14,behavior.type_14);
        if(constructor(state.data())!=state.data()||state!=expected_state)return 1;
    }
    std::cout<<"flapper: 16000 updates, entries, leaves and constructor comparisons passed\n";
}
