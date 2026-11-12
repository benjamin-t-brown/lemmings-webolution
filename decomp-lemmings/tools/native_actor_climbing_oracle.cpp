#include "lemmings/actor_climbing.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <array>
#include <bit>
#include <cmath>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
using namespace lemmings;
namespace {
template<class T> T get(const void* object,unsigned offset){T value;std::memcpy(&value,static_cast<const char*>(object)+offset,sizeof(value));return value;}
template<class T> void put(void* object,unsigned offset,T value){std::memcpy(static_cast<char*>(object)+offset,&value,sizeof(value));}
std::uint64_t bits(double value){return std::bit_cast<std::uint64_t>(value);}
struct Event {
    unsigned kind,parameter;
    std::uint64_t x,y,time;
    int horizontal,vertical;
    unsigned flag;
    bool operator==(const Event&)const=default;
};
std::vector<Event> trace;
void* native_actor;
void* native_state;
ClimbingActor* source_actor=nullptr;
ActorClimbingState* source_state;
bool valid;
unsigned script;
std::array<std::uint8_t,0x28> allocation{};
unsigned allocated_size;
ActorBehavior created{};
void record(unsigned kind,unsigned parameter=0,double time=0){
    if(source_actor)trace.push_back({kind,parameter,bits(source_actor->geometry.position[0]),bits(source_actor->geometry.position[1]),bits(time),
        source_actor->geometry.direction_40,source_actor->geometry.direction_44,source_state->reached_boundary_18});
    else trace.push_back({kind,parameter,bits(get<double>(native_actor,8)),bits(get<double>(native_actor,0x10)),bits(time),
        get<int>(native_actor,0x40),get<int>(native_actor,0x44),get<std::uint8_t>(native_state,0x18)});
}
void __attribute__((thiscall)) select_animation(void* actor,const char* name){
    valid &= actor==native_actor && std::strcmp(name,"climb")==0;record(1);
}
void __attribute__((thiscall)) advance_animation(void* model,double time){
    valid &= static_cast<char*>(model)-0x78==native_actor;record(2,0,time);
    if(script&1)put(native_actor,0x44,-get<int>(native_actor,0x44));
}
void __cdecl log_notice(const char* format,...){
    const auto address=reinterpret_cast<std::uintptr_t>(format);
    unsigned kind=99;
    switch(address){
    case 0x4BF2DC:kind=0;break;case 0x4BF2FC:kind=1;break;case 0x4BF310:kind=2;break;
    case 0x4BF324:kind=3;break;case 0x4BF340:kind=4;break;
    default:valid=false;
    }
    if(kind==0 || kind==3){
        va_list arguments;va_start(arguments,format);
        valid &= bits(va_arg(arguments,double))==bits(get<double>(native_actor,8));
        valid &= bits(va_arg(arguments,double))==bits(get<double>(native_actor,0x10));va_end(arguments);
    }
    record(3,kind);
}
void* __cdecl allocate(unsigned size){valid &= size==0x18 || size==0x28;allocated_size=size;allocation.fill(0xA5);record(4,size);return allocation.data();}
void __attribute__((thiscall)) attach(void* actor,void* state){
    valid &= actor==native_actor && state==allocation.data();
    const auto type=get<int>(state,0x14);
    valid &= type==(allocated_size==0x28?0x13:0x1E) && get<std::uint8_t>(state,0x10)==0 && get<std::uint8_t>(state,0x11)==0;
    record(5,type);
    if(script&2){put(native_state,0x18,std::uint8_t{99});put(native_actor,8,get<double>(native_actor,8)+0.25);}
}
void jump(std::uintptr_t address,std::uintptr_t target){put(reinterpret_cast<void*>(address),0,std::uint8_t{0xE9});put(reinterpret_cast<void*>(address),1,static_cast<std::uint32_t>(target-address-5));}
ActorClimbingHooks hooks(){
    ActorClimbingHooks value{};
    value.set_animation=[](void*,ClimbingActor&,std::string_view name){valid &= name=="climb";record(1);};
    value.advance_animation=[](void*,ClimbingActor& actor,double time){record(2,0,time);if(script&1)actor.geometry.direction_44=-actor.geometry.direction_44;};
    value.notice=[](void*,ClimbingActor&,ClimbingNotice notice){record(3,static_cast<unsigned>(notice));};
    value.create_state=[](void*,ClimbingTransition transition){created.type_14=transition==ClimbingTransition::falling_415fa0?0x13:0x1E;record(4,created.type_14==0x13?0x28:0x18);return &created;};
    value.attach_state=[](void*,ClimbingActor& actor,ActorBehavior* state){valid &= state==&created;record(5,state->type_14);if(script&2){source_state->reached_boundary_18=99;actor.geometry.position[0]+=0.25;}};
    return value;
}
}
int main(){
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xD0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);if(!std::cin || blocks>64)return 2;
    for(unsigned i=0;i<blocks;++i){std::uint32_t address,length;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&length),4);
        if(address<0x400000 || length>0xD0000 || address-0x400000>0xD0000-length)return 2;
        std::cin.read(reinterpret_cast<char*>(address),length);if(!std::cin)return 2;
    }
    jump(0x42FD25,reinterpret_cast<std::uintptr_t>(&select_animation));jump(0x40F860,reinterpret_cast<std::uintptr_t>(&log_notice));
    jump(0x49C1A8,reinterpret_cast<std::uintptr_t>(&allocate));jump(0x42E696,reinterpret_cast<std::uintptr_t>(&attach));
    DWORD previous;if(!VirtualProtect(image,0xC0000,PAGE_EXECUTE_READ,&previous))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xC0000);
    using Update=int(__attribute__((thiscall))*)(void*,void*,double*);
    using Collision=int(__attribute__((thiscall))*)(void*,void*);
    using Boundary=double(__attribute__((thiscall))*)(void*,void*);
    using Enter=void(__attribute__((thiscall))*)(void*,void*);
    using Sample=void*(__attribute__((thiscall))*)(void*,void*);
    using Construct=void*(__attribute__((thiscall))*)(void*);
    const auto update=reinterpret_cast<Update>(0x4137CA);
    const auto collision=reinterpret_cast<Collision>(0x4139BF);
    const auto boundary=reinterpret_cast<Boundary>(0x413924);
    const auto enter=reinterpret_cast<Enter>(0x4137A3);
    const auto sample=reinterpret_cast<Sample>(0x42F35F);
    std::array<std::uint8_t,0x20> constructed{};constructed.fill(0xA5);
    reinterpret_cast<Construct>(0x413750)(constructed.data());
    if(get<int>(constructed.data(),0x14)!=0 || get<std::uint8_t>(constructed.data(),0x10)!=1 ||
       get<std::uint8_t>(constructed.data(),0x11)!=1 || get<std::uint8_t>(constructed.data(),0x18)!=0xA5)return 1;
    std::array<std::uintptr_t,12> vtable{};vtable[11]=reinterpret_cast<std::uintptr_t>(&advance_animation);
    std::array<std::int32_t,8*256> cells{};CollisionGridHead grid{};grid.height=8;grid.cells.data=cells.data();
    std::memcpy(reinterpret_cast<void*>(0x4C6A10),&grid,sizeof(grid));
    unsigned cases=0;
    auto compare=[&](ClimbingActor actor,double time,std::uint8_t flag,bool only_collision){
        ActorClimbingState state{flag};
        std::array<std::uint8_t,0x3A8> original{};std::array<std::uint8_t,0x20> original_state{};
        for(unsigned i=0;i<3;++i)put(original.data(),8+8*i,actor.geometry.position[i]);
        for(unsigned i=0;i<2;++i){put(original.data(),0x350+8*i,actor.geometry.pair_350[i]);put(original.data(),0x360+8*i,actor.geometry.pair_360[i]);}
        put(original.data(),0x40,actor.geometry.direction_40);put(original.data(),0x44,actor.geometry.direction_44);
        put(original.data(),0x50,actor.mode_50);put(original.data(),0x78,vtable.data());put(original_state.data(),0x18,flag);
        const auto before=original;const auto state_before=original_state;
        native_actor=original.data();native_state=original_state.data();source_actor=nullptr;
        std::array<double,3> native_point{};sample(native_actor,native_point.data());const auto source_point=actor_sample_position_350(actor.geometry);
        if(std::memcmp(native_point.data(),source_point.data(),24)!=0 || bits(boundary(native_state,native_actor))!=bits(climbing_boundary_time(state,actor)))return false;
        trace.clear();valid=true;double native_time=time;
        if(!only_collision)enter(native_state,native_actor);
        const auto native_result=only_collision?collision(native_state,native_actor):update(native_state,native_actor,&native_time);
        const auto native_trace=trace;const auto native_valid=valid;
        source_actor=&actor;source_state=&state;trace.clear();valid=true;
        if(!only_collision)enter_actor_climbing(actor,hooks());
        const auto source_result=only_collision?resolve_actor_climbing_collision(actor,grid,hooks()):update_actor_climbing(state,actor,time,grid,hooks());
        ++cases;
        if(!valid || !native_valid || native_trace!=trace || native_result!=source_result || bits(native_time)!=bits(time) ||
           bits(get<double>(original.data(),8))!=bits(actor.geometry.position[0]) || bits(get<double>(original.data(),0x10))!=bits(actor.geometry.position[1]) ||
           get<int>(original.data(),0x40)!=actor.geometry.direction_40 || get<int>(original.data(),0x44)!=actor.geometry.direction_44 ||
           get<std::uint8_t>(original_state.data(),0x18)!=state.reached_boundary_18){
            std::cerr<<"climbing mismatch case="<<cases<<" script="<<script<<" native="<<native_result<<" source="<<source_result<<" events="<<native_trace.size()<<'/'<<trace.size()<<'\n';
            for(unsigned i=0;i<native_trace.size()&&i<trace.size();++i)if(!(native_trace[i]==trace[i]))std::cerr<<"event="<<i<<" kind="<<native_trace[i].kind<<'/'<<trace[i].kind<<" x="<<native_trace[i].x<<'/'<<trace[i].x<<" y="<<native_trace[i].y<<'/'<<trace[i].y<<" time="<<native_trace[i].time<<'/'<<trace[i].time<<'\n';
            return false;
        }
        for(unsigned i=0;i<2;++i)if(bits(get<double>(original.data(),0x350+8*i))!=bits(actor.geometry.pair_350[i]) || bits(get<double>(original.data(),0x360+8*i))!=bits(actor.geometry.pair_360[i]))return false;
        put(original.data(),8,get<double>(before.data(),8));put(original.data(),0x10,get<double>(before.data(),0x10));
        put(original.data(),0x40,get<int>(before.data(),0x40));put(original.data(),0x44,get<int>(before.data(),0x44));
        for(unsigned i=0;i<2;++i){put(original.data(),0x350+8*i,get<double>(before.data(),0x350+8*i));put(original.data(),0x360+8*i,get<double>(before.data(),0x360+8*i));}
        put(original_state.data(),0x18,flag);return original==before && original_state==state_before;
    };
    const double nan=std::numeric_limits<double>::quiet_NaN();
    unsigned helper_edges=0;
    for(double y:{nan,std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),
                  2147483647.0,2147483648.0,-2147483649.0,4294967296.0,
                  9223372036854775808.0,-9223372036854775808.0})
        for(int direction:{-1,0,1,2})for(std::uint8_t flag:{std::uint8_t{0},std::uint8_t{0x80}}){
            std::array<std::uint8_t,0x80> raw_actor{};std::array<std::uint8_t,0x20> raw_state{};
            put(raw_actor.data(),0x10,y);put(raw_actor.data(),0x44,direction);put(raw_state.data(),0x18,flag);
            ClimbingActor actor{};actor.geometry.position[1]=y;actor.geometry.direction_44=direction;
            if(bits(boundary(raw_state.data(),raw_actor.data()))!=bits(climbing_boundary_time(ActorClimbingState{flag},actor)))return 1;
            ++helper_edges;
        }
    ClimbingActor actor{};actor.geometry.position={3,2.5,0.5};
    for(script=0;script<4;++script)for(int mask:{0,1,0x80,0x2000})for(int direction:{-1,1})
        for(double y:{-1.0,0.0,2.5,7.0,8.0,9.0,nan})for(double time:{-1.0,-0.0,0.01,0.1,0.2,1.0,nan}){
            cells.fill(mask);actor.geometry.position[1]=y;actor.geometry.direction_44=direction;actor.mode_50=script;
            if(!compare(actor,time,script%3,false))return 1;
        }
    std::mt19937_64 random(0x4137CA);
    for(unsigned i=0;i<20000;++i){
        script=random()%4;
        for(auto& cell:cells)cell=(random()%4)==0?static_cast<int>(random()&0xFFFF):0;
        actor.geometry.position={static_cast<double>(static_cast<int>(random()%1600)-300)/4.0,static_cast<double>(static_cast<int>(random()%80)-20)/4.0,nan};
        actor.geometry.pair_350={static_cast<double>(static_cast<int>(random()%8)-4)/4.0,static_cast<double>(static_cast<int>(random()%16)-4)/4.0};
        actor.geometry.pair_360={0.25,0.5};actor.geometry.direction_40=static_cast<int>(random()%5)-2;
        actor.geometry.direction_44=random()&1?1:-1;actor.mode_50=random()%5;
        if(!compare(actor,static_cast<double>(random()%2000)/1000.0,random()%256,i&1))return 1;
    }
    std::cout<<"native climbing passed "<<cases<<" entry/update/collision/sample/trace comparisons and "<<helper_edges<<" conversion edges\n";
    VirtualFree(image,0,MEM_RELEASE);
}
