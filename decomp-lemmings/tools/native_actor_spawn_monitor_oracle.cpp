#include "lemmings/actor_spawn_monitor.hpp"
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
using Triple = std::array<double,3>;
using Pair = std::array<double,2>;
using ActorBytes = std::array<std::uint8_t,0x370>;
template<class T,class B> void put(B& bytes,unsigned offset,T value) { std::memcpy(bytes.data()+offset,&value,sizeof(value)); }
template<class T,class B> T get(const B& bytes,unsigned offset) { T value; std::memcpy(&value,bytes.data()+offset,sizeof(value)); return value; }
SpawnMonitorActor read_actor(const ActorBytes& bytes) {
    SpawnMonitorActor actor;
    actor.geometry.position=get<Triple>(bytes,8); actor.geometry.direction_44=get<std::int32_t>(bytes,0x44);
    actor.geometry.pair_360=get<Pair>(bytes,0x360); actor.last_pair_68=get<Pair>(bytes,0x68); actor.flag_5f=bytes[0x5f]; return actor;
}
void write_actor(ActorBytes& bytes,const SpawnMonitorActor& actor) {
    put(bytes,8,actor.geometry.position); put(bytes,0x44,actor.geometry.direction_44);
    put(bytes,0x360,actor.geometry.pair_360); put(bytes,0x68,actor.last_pair_68); put(bytes,0x5f,actor.flag_5f);
}
struct Trace : ActorSpawnMonitorCallbacks {
    std::vector<std::uint64_t> events;
    double* remaining=nullptr;
    bool continuation=true, present=false, mutate=false;
    int regions=0, match=-1;
    ActorBehavior behavior;
    SpawnMonitorRequest request;
    void* expected_context=nullptr;
    void n(std::uint64_t value) { events.push_back(value); }
    void d(double value) { n(std::isnan(value)?0x7ff8000000000000ULL:std::bit_cast<std::uint64_t>(value)); }
    std::uint8_t can_continue_42f6ec(SpawnMonitorActor& actor) override {
        n(1);d(*remaining);
        if(mutate) {actor.geometry.position[0]=123.5;actor.geometry.direction_44=99;*remaining=2.5;}
        return continuation?0x80:0;
    }
    std::int32_t region_count_4157f0() override {n(2);return regions;}
    std::uint8_t region_contains_420a52(std::int32_t index,Triple point) override {
        n(3);n(index);for(auto v:point)d(v);return index==match?0xff:0;
    }
    std::uint8_t has_state_42ecc9(SpawnMonitorActor&,std::int32_t type) override {n(4);n(type);return present?0x80:0;}
    void clear_primary_42e725(SpawnMonitorActor&) override {n(5);}
    ActorBehavior* create_state(SpawnMonitorTransition kind) override {n(6);n(static_cast<unsigned>(kind));return &behavior;}
    void attach_state_42e696(SpawnMonitorActor&,ActorBehavior* value) override {n(7);n(value==&behavior);}
    SpawnMonitorRequest* create_request_415710(void* context) override {n(8);n(context==expected_context);return &request;}
    void enqueue_request_43f0c0(SpawnMonitorRequest& value) override {n(9);for(auto v:value.pair_10)d(v);value.pair_10={99,99};}
} trace;
ActorBytes* raw_actor=nullptr;
std::array<std::uint8_t,0x20> raw_allocation{};
std::array<std::uint8_t,6*0x178> raw_regions{};
bool pointers_ok=true;
std::uint8_t __attribute__((thiscall)) gate(void* self) {
    pointers_ok&=self==raw_actor->data();auto actor=read_actor(*raw_actor);
    const auto result=trace.can_continue_42f6ec(actor);write_actor(*raw_actor,actor);return result;
}
std::int32_t __attribute__((thiscall)) region_count(void*) {return trace.region_count_4157f0();}
void* __attribute__((thiscall)) region_at(void*,std::int32_t index) {return raw_regions.data()+index*0x178;}
std::uint8_t __attribute__((thiscall)) region_test(void* self,Triple point) {
    const auto index=(static_cast<std::uint8_t*>(self)-raw_regions.data())/0x178;
    return trace.region_contains_420a52(index,point);
}
std::uint8_t __attribute__((thiscall)) has_state(void* self,std::int32_t type) {
    pointers_ok&=self==raw_actor->data();auto actor=read_actor(*raw_actor);return trace.has_state_42ecc9(actor,type);
}
void __attribute__((thiscall)) clear_primary(void* self) {pointers_ok&=self==raw_actor->data();auto actor=read_actor(*raw_actor);trace.clear_primary_42e725(actor);}
void* __cdecl allocate(std::uint32_t size) {pointers_ok&=(size==0x18||size==0x20);raw_allocation.fill(0xa5);return raw_allocation.data();}
void* __attribute__((thiscall)) constructor_2e(void* self) {trace.create_state(SpawnMonitorTransition::state_406c20);return self;}
void* __attribute__((thiscall)) constructor_22(void* self) {trace.create_state(SpawnMonitorTransition::state_408700);return self;}
void __attribute__((thiscall)) attach(void* self,void* behavior) {
    pointers_ok&=self==raw_actor->data()&&behavior==raw_allocation.data();auto actor=read_actor(*raw_actor);trace.attach_state_42e696(actor,&trace.behavior);
}
void* __attribute__((thiscall)) request_constructor(void* self,void* context) {trace.create_request_415710(context);return self;}
void __attribute__((thiscall)) enqueue(void*,void* request) {
    pointers_ok&=request==raw_allocation.data();SpawnMonitorRequest typed{get<Pair>(raw_allocation,0x10)};
    trace.enqueue_request_43f0c0(typed);put(raw_allocation,0x10,typed.pair_10);
}
void notification(void*,std::int32_t x1,std::int32_t y1,std::int32_t x2,std::int32_t y2) {
    trace.n(10);for(auto v:{x1,y1,x2,y2})trace.n(static_cast<std::uint32_t>(v));
}
void __attribute__((thiscall)) native_notification(void*,std::int32_t x1,std::int32_t y1,std::int32_t x2,std::int32_t y2) {notification(nullptr,x1,y1,x2,y2);}
void jump(std::uintptr_t address,void* target) {
    auto* code=reinterpret_cast<std::uint8_t*>(address);code[0]=0xe9;
    const auto delta=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-address-5);std::memcpy(code+1,&delta,4);
}
bool load() {
    std::uint32_t count=0;std::cin.read(reinterpret_cast<char*>(&count),4);
    for(unsigned i=0;i<count;++i){std::uint32_t address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||address>0x4d0000||size>0x4d0000-address)return false;
        std::cin.read(reinterpret_cast<char*>(address),size);}
    return static_cast<bool>(std::cin);
}
bool same(double a,double b) {return (std::isnan(a)&&std::isnan(b))||std::bit_cast<std::uint64_t>(a)==std::bit_cast<std::uint64_t>(b);}
}
int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xd0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    if(!load())return 2;
    jump(0x42f6ec,reinterpret_cast<void*>(&gate));jump(0x4157f0,reinterpret_cast<void*>(&region_count));jump(0x4157d0,reinterpret_cast<void*>(&region_at));
    jump(0x420a52,reinterpret_cast<void*>(&region_test));jump(0x42ecc9,reinterpret_cast<void*>(&has_state));jump(0x42e725,reinterpret_cast<void*>(&clear_primary));
    jump(0x49c1a8,reinterpret_cast<void*>(&allocate));jump(0x406c20,reinterpret_cast<void*>(&constructor_2e));jump(0x408700,reinterpret_cast<void*>(&constructor_22));
    jump(0x42e696,reinterpret_cast<void*>(&attach));jump(0x415710,reinterpret_cast<void*>(&request_constructor));jump(0x43f0c0,reinterpret_cast<void*>(&enqueue));
    jump(0x43b0a1,reinterpret_cast<void*>(&native_notification));
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Update=std::uint32_t(__attribute__((thiscall))*)(void*,void*,double*);
    using Construct=void*(__attribute__((thiscall))*)(void*);
    using Empty=void(__attribute__((thiscall))*)(void*,void*);
    using Sample=Triple*(__attribute__((thiscall))*)(void*,Triple*);
    using Compare=std::uint8_t(__attribute__((thiscall))*)(void*,void*);
    using Clear=std::uint32_t(__attribute__((thiscall))*)(void*);
    ActorBytes bytes;raw_actor=&bytes;
    std::array<std::uint8_t,0x28> state;
    std::array<std::int32_t,256*8> cells{}, native_cells{};
    CollisionGridHead grid{};grid.height=8;grid.cells.data=cells.data();
    CollisionGridHead native_grid=grid;native_grid.cells.data=native_cells.data();
    std::memcpy(reinterpret_cast<void*>(0x4c6a10),&native_grid,sizeof(native_grid));
    const std::uint32_t request_base=0x4cc000;std::memcpy(reinterpret_cast<void*>(0x4cbf88),&request_base,4);
    trace.expected_context=reinterpret_cast<void*>(request_base+0x190);
    std::mt19937_64 random(0x41527b);
    const double nan=std::numeric_limits<double>::quiet_NaN();
    const double special[]={0.,-0.,1.,-1.,2147483648.,-2147483649.,4294967297.,nan,
        std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),9223372036854775808.};
    for(unsigned trial=0;trial<20000;++trial) {
        SpawnMonitorActor actor;
        actor.geometry.position={double(static_cast<int>(random()%6000)-3000)/8,double(static_cast<int>(random()%160)-80)/8,0.25};
        actor.geometry.pair_360={double(static_cast<int>(random()%16)-8)/4,double(static_cast<int>(random()%16)-8)/4};
        actor.geometry.direction_44=trial%9==0 ? INT32_MIN : trial%9==1 ? INT32_MAX : trial%2?1:-1;
        actor.last_pair_68={double(random()%256),double(random()%8)};
        if(trial%5==0)actor.last_pair_68={nan,nan};
        actor.flag_5f=static_cast<std::uint8_t>(random());
        bytes.fill(0xa5);write_actor(bytes,actor);
        Triple native_sample;reinterpret_cast<Sample>(0x42f183)(bytes.data(),&native_sample);
        const auto source_sample=actor_monitor_sample_position(actor.geometry);
        for(unsigned i=0;i<3;++i)if(!same(source_sample[i],native_sample[i]))return 1;
        const auto cell=std::array<std::int32_t,4>{0,0x40,0x100,0x140}[trial%4];
        std::fill(cells.begin(),cells.end(),cell);native_cells=cells;
        trace.regions=random()%7;trace.match=static_cast<int>(random()%8)-1;trace.present=trial%3==0;
        trace.continuation=trial%7!=0;trace.mutate=trial%11==0;
        double source_time=3.0,native_time=3.0;trace.remaining=&source_time;trace.events.clear();
        const auto source_result=update_actor_spawn_monitor(actor,source_time,grid,trace.expected_context,trace);
        const auto expected_events=trace.events;ActorBytes expected=bytes;write_actor(expected,actor);
        trace.remaining=&native_time;trace.events.clear();
        const auto native_result=reinterpret_cast<Update>(0x41527b)(state.data(),bytes.data(),&native_time);
        if(!pointers_ok||native_result!=source_result||source_time!=native_time||bytes!=expected||trace.events!=expected_events) {
            std::cerr<<"monitor mismatch "<<trial<<" events "<<trace.events.size()<<'/'<<expected_events.size()<<'\n';
            for(unsigned i=0;i<std::min(trace.events.size(),expected_events.size());++i)if(trace.events[i]!=expected_events[i]){std::cerr<<"first "<<i<<' '<<trace.events[i]<<' '<<expected_events[i]<<'\n';break;}
            return 1;
        }
        state.fill(0xa5);reinterpret_cast<Construct>(0x4151f0)(state.data());
        std::array<std::uint8_t,0x28> expected_state;expected_state.fill(0xa5);
        put(expected_state,0,std::uint32_t{0x4b09c0});put(expected_state,8,0.0);put(expected_state,0x10,std::uint8_t{1});put(expected_state,0x11,std::uint8_t{1});put(expected_state,0x14,std::uint32_t{0x2f});
        if(state!=expected_state)return 1;
        reinterpret_cast<Empty>(0x409310)(state.data(),bytes.data());reinterpret_cast<Empty>(0x401320)(state.data(),bytes.data());if(state!=expected_state)return 1;
        Pair first{special[trial%11],special[(trial+1)%11]}, second{special[(trial+2)%11],special[(trial+3)%11]};
        if(trial%4==0)second=first;
        else if(trial%4==1)second[0]=first[0];
        else if(trial%4==2)second[1]=first[1];
        if(bool(reinterpret_cast<Compare>(0x415800)(first.data(),second.data()))!=monitor_pairs_differ(first,second))return 1;
        SpawnMonitorRequest request{{double(static_cast<int>(random()%800)-400)/4,double(static_cast<int>(random()%80)-40)/4}};
        put(raw_allocation,0x10,request.pair_10);trace.events.clear();
        const auto source_clear=clear_spawn_monitor_request(request,grid,{nullptr,notification});const auto expected_clear=trace.events;
        trace.events.clear();const auto native_clear=reinterpret_cast<Clear>(0x41565c)(raw_allocation.data());
        if(source_clear!=native_clear||trace.events!=expected_clear||cells!=native_cells)return 1;
    }
    // Isolate extreme input sampling so wrap's repeated subtraction cannot
    // dominate the full-monitor scenarios.
    for(double x:special)for(double y:special){SpawnMonitorActor actor;actor.geometry.position={x,y,nan};actor.geometry.pair_360={0.25,-0.75};
        write_actor(bytes,actor);Triple observed;reinterpret_cast<Sample>(0x42f183)(bytes.data(),&observed);const auto expected=actor_monitor_sample_position(actor.geometry);
        for(unsigned i=0;i<3;++i)if(!same(observed[i],expected[i]))return 1;}
    VirtualFree(image,0,MEM_RELEASE);
    std::cout<<"native spawn monitor passed 20000 update/constructor/clear/pair comparisons and 20121 samples\n";
}
