#include "lemmings/actor_bouncing.hpp"
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

namespace {
using namespace lemmings;
template<class T> T get(const void* p, unsigned n) { T v; std::memcpy(&v, static_cast<const char*>(p)+n, sizeof(v)); return v; }
template<class T> void put(void* p, unsigned n, T v) { std::memcpy(static_cast<char*>(p)+n, &v, sizeof(v)); }
std::uint64_t bits(double x) { return std::isnan(x) ? 0x7ff8000000000000ULL : std::bit_cast<std::uint64_t>(x); }
struct Event {
    unsigned kind, argument;
    std::uint64_t a, b, x, y, elapsed;
    unsigned flag;
    bool operator==(const Event&) const = default;
};
void *raw_actor, *raw_state;
double* budget;
BouncingActor* source_actor;
ActorBouncingState* source_state;
std::vector<Event> events;
unsigned script = 0;
bool valid = true;
std::int16_t identifier;
std::uint8_t wall, has_state;
BounceStateKind created_kind;
std::array<std::uint8_t,0x110> allocated{};
std::array<BounceNearbyActor,4> nearby{};
std::array<std::array<std::uint8_t,0x348>,4> raw_nearby{};
std::array<std::array<std::uint8_t,0x20>,4> raw_nearby_state{};
std::vector<std::int32_t> ids;
void record(unsigned kind, unsigned argument = 0, double a = 0, double b = 0) {
    const auto x = source_actor ? source_actor->geometry.position[0] : get<double>(raw_actor,8);
    const auto y = source_actor ? source_actor->geometry.position[1] : get<double>(raw_actor,0x10);
    const auto elapsed = source_actor ? source_state->elapsed_08 : get<double>(raw_state,8);
    const auto flag = source_actor ? source_state->reached_boundary_28 : get<std::uint8_t>(raw_state,0x28);
    events.push_back({kind,argument,bits(a),bits(b),bits(x),bits(y),bits(elapsed),flag});
}
unsigned size(BounceStateKind k) { return k == BounceStateKind::effect_40e3c0 ? 0x110 : k == BounceStateKind::bounce_40ec7e ? 0x30 : 0x28; }
int type(BounceStateKind k) { return k == BounceStateKind::effect_40e3c0 ? 0x2d : k == BounceStateKind::bounce_40ec7e ? 0x21 : 0x13; }
void mutate_model() {
    if (!(script & 1)) return;
    *budget = 0.0625;
    if (source_actor) { source_state->elapsed_08 = 0.125; source_actor->effect_60 = 1; }
    else { put(raw_state,8,0.125); put(raw_actor,0x60,std::uint8_t{1}); }
}
void mutate_attach() {
    if (!(script & 2)) return;
    if (source_actor) source_state->reached_boundary_28 = 99;
    else put(raw_state,0x28,std::uint8_t{99});
}
struct Effects : ActorBouncingCallbacks {
    void advance_model_2c(void*, double dt) override { record(1,0,dt); mutate_model(); }
    std::int16_t actor_id_406a00(BouncingActor&) override { return identifier; }
    void select_animation_42fd25(BouncingActor& actor, std::string_view name) override {
        valid &= name == "fall"; record(2);
        if (script & 4) actor.geometry.direction_44 = -actor.geometry.direction_44;
    }
    void notice_40f860(BounceNotice n, double a, double b, std::int32_t id) override { record(3, static_cast<unsigned>(n),a,b); if (n == BounceNotice::started) valid &= id == identifier; }
    std::int32_t nearby_count_404d30() override { return static_cast<std::int32_t>(ids.size()); }
    std::int32_t nearby_id_404d10(std::int32_t i) override { return ids.at(i); }
    BounceNearbyActor* find_actor_444ba1(std::int32_t id) override { record(4,static_cast<unsigned>(id)); return id < 0 ? nullptr : &nearby.at(id); }
    void* create_state(BounceStateKind k) override { created_kind = k; record(5,size(k)); return script & 8 ? nullptr : allocated.data(); }
    void attach_state_42e696(BouncingActor&, void* state) override { valid &= state == (script & 8 ? nullptr : allocated.data()); record(6, state ? type(created_kind) : 0); mutate_attach(); }
    void select_landed_42f8b9(BouncingActor&) override { record(7); mutate_attach(); }
    std::uint8_t wall_contact_42eb48(BouncingActor&) override { record(8,wall); return wall; }
    std::uint8_t has_state_42ecc9(BouncingActor&, std::int32_t t) override { record(9,t); return has_state; }
    void remove_state_42e7f3(BouncingActor&, std::int32_t t) override { record(10,t); }
} effects;
void __attribute__((thiscall)) advance(void* model, double dt) {
    valid &= static_cast<char*>(model) == static_cast<char*>(raw_actor)+0x78;
    record(1,0,dt); mutate_model();
}
void __attribute__((thiscall)) native_select(void* actor, const char* name) {
    valid &= actor == raw_actor && std::strcmp(name,"fall") == 0; record(2);
    if (script & 4) put(raw_actor,0x44,-get<int>(raw_actor,0x44));
}
void __cdecl notice(const char* format, ...) {
    const auto address = reinterpret_cast<std::uintptr_t>(format);
    BounceNotice n; double a = 0, b = 0;
    va_list args; va_start(args,format);
    switch(address) {
    case 0x4bf150: n = BounceNotice::started; valid &= va_arg(args,int) == identifier; a = va_arg(args,double); b = va_arg(args,double); break;
    case 0x4bf18c: n = BounceNotice::updating_to_integer; break;
    case 0x4bf1a0: n = BounceNotice::updated_by; a = va_arg(args,double); break;
    case 0x4bf1bc: n = BounceNotice::new_position; a = va_arg(args,double); b = va_arg(args,double); break;
    case 0x4bf1dc: n = BounceNotice::checking_floor; break;
    case 0x4bf1f8: n = BounceNotice::hit_floor; break;
    case 0x4bf20c: n = BounceNotice::hit_trampoline; break;
    case 0x4bf224: n = BounceNotice::hit_head; break;
    case 0x4bf238: n = BounceNotice::checking_wall; break;
    case 0x4bf254: n = BounceNotice::hit_wall; break;
    default: valid = false; va_end(args); return;
    }
    va_end(args); record(3,static_cast<unsigned>(n),a,b);
}
void* __cdecl allocate(unsigned bytes) {
    valid &= bytes == 0x110 || bytes == 0x30 || bytes == 0x28;
    created_kind = bytes == 0x110 ? BounceStateKind::effect_40e3c0 : bytes == 0x30 ? BounceStateKind::bounce_40ec7e : BounceStateKind::falling_415fa0;
    record(5,bytes); allocated.fill(0xa5); return script & 8 ? nullptr : allocated.data();
}
void* __attribute__((thiscall)) effect_constructor(void* state) {
    // Explicit allocation/effect-construction adapter; other state constructors run unchanged.
    put(state,0x14,0x2d); return state;
}
void __attribute__((thiscall)) attach(void* actor, void* state) {
    valid &= actor == raw_actor && state == (script & 8 ? nullptr : allocated.data());
    if (state) {
        valid &= get<int>(state,0x14) == type(created_kind);
        if (created_kind != BounceStateKind::effect_40e3c0)
            valid &= get<std::uint8_t>(state,0x10) == 0 && get<std::uint8_t>(state,0x11) == 0;
    }
    record(6,state ? get<int>(state,0x14) : 0); mutate_attach();
}
void __attribute__((thiscall)) landed(void* actor) { valid &= actor == raw_actor; record(7); mutate_attach(); }
std::uint8_t __attribute__((thiscall)) wall_contact(void* actor) { valid &= actor == raw_actor; record(8,wall); return wall; }
std::uint8_t __attribute__((thiscall)) contains(void* actor, int t) { valid &= actor == raw_actor; record(9,t); return has_state; }
void __attribute__((thiscall)) native_remove(void* actor, int t) { valid &= actor == raw_actor; record(10,t); }
void* __attribute__((thiscall)) find(void* world, int id) {
    valid &= world == reinterpret_cast<void*>(0x4c6a08); record(4,static_cast<unsigned>(id));
    return id < 0 ? nullptr : raw_nearby.at(id).data();
}
double __cdecl root(double x) { return static_cast<double>(std::sqrt(static_cast<long double>(x))); }
void jump(unsigned address, void* target) { put(reinterpret_cast<void*>(address),0,std::uint8_t{0xe9}); put(reinterpret_cast<void*>(address),1,static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-address-5)); }
}
int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image = VirtualAlloc(reinterpret_cast<void*>(0x400000),0xd0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    unsigned count = 0; std::cin.read(reinterpret_cast<char*>(&count),4); if (!std::cin || count > 100) return 2;
    for (unsigned i=0;i<count;++i) {
        unsigned address=0,length=0; std::cin.read(reinterpret_cast<char*>(&address),4); std::cin.read(reinterpret_cast<char*>(&length),4);
        if (address < 0x400000 || length > 0xd0000 || address-0x400000 > 0xd0000-length) return 2;
        std::cin.read(reinterpret_cast<char*>(address),length); if (!std::cin) return 2;
    }
    jump(0x42fd25,reinterpret_cast<void*>(&native_select)); jump(0x40f860,reinterpret_cast<void*>(&notice));
    jump(0x49c1a8,reinterpret_cast<void*>(&allocate)); jump(0x40e3c0,reinterpret_cast<void*>(&effect_constructor));
    jump(0x42e696,reinterpret_cast<void*>(&attach)); jump(0x42f8b9,reinterpret_cast<void*>(&landed));
    jump(0x42eb48,reinterpret_cast<void*>(&wall_contact)); jump(0x42ecc9,reinterpret_cast<void*>(&contains));
    jump(0x42e7f3,reinterpret_cast<void*>(&native_remove)); jump(0x444ba1,reinterpret_cast<void*>(&find));
    jump(0x49c9c4,reinterpret_cast<void*>(&root));
    DWORD old; if (!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old)) return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Enter = void(__attribute__((thiscall))*)(void*,void*);
    using Update = int(__attribute__((thiscall))*)(void*,void*,double*);
    using Collision = int(__attribute__((thiscall))*)(void*,void*,unsigned);
    using TimeX = double(__attribute__((thiscall))*)(void*,void*);
    using TimeY = double(__attribute__((thiscall))*)(void*,int*);
    using GlobalInit = void(__cdecl*)();
    const auto enter = reinterpret_cast<Enter>(0x40ecca), leave = reinterpret_cast<Enter>(0x40ee9a);
    const auto update = reinterpret_cast<Update>(0x40eec4);
    const auto collision = reinterpret_cast<Collision>(0x40f1e9);
    const auto tx = reinterpret_cast<TimeX>(0x40f55d); const auto ty = reinterpret_cast<TimeY>(0x40f5f8);
    reinterpret_cast<GlobalInit>(0x40ec2a)();
    BounceTimeTable table; valid &= bits(table.apex_time) == bits(get<double>(reinterpret_cast<void*>(0x4c55a8),0));
    unsigned tables = 0;
    for (double apex : {1.4,0.0,-1.0,100.0,std::numeric_limits<double>::quiet_NaN()}) {
        table.apex_time = apex; initialize_bounce_times(table);
        put(reinterpret_cast<void*>(0x4c55a8),0,apex); reinterpret_cast<GlobalInit>(0x40f700)();
        for (unsigned i=0;i<50;++i) if (bits(table.times[i]) != bits(get<double>(reinterpret_cast<void*>(0x4c5410),8*i))) { std::cerr << "bounce table mismatch " << i << '\n'; return 1; }
        ++tables;
    }
    table.apex_time = 1.4; initialize_bounce_times(table);
    std::memcpy(reinterpret_cast<void*>(0x4c5410),table.times.data(),400); put(reinterpret_cast<void*>(0x4c55a8),0,table.apex_time);
    std::array<std::uint8_t,0x3a8> actor_buffer{}; std::array<std::uint8_t,0x30> state_buffer{};
    raw_actor=actor_buffer.data(); raw_state=state_buffer.data();
    std::array<std::uintptr_t,12> model_vtable{}; model_vtable[11]=reinterpret_cast<std::uintptr_t>(&advance);
    std::array<std::int32_t,32*256> cells{}; CollisionGridHead grid{}; grid.height=32;grid.cells.data=cells.data();
    std::memcpy(reinterpret_cast<void*>(0x4c6a10),&grid,sizeof(grid));
    std::mt19937_64 random(0x40eec4);
    unsigned cases=0,entries=0,helpers=0;
    auto prepare = [&](const BouncingActor& actor,const ActorBouncingState& state) {
        actor_buffer.fill(0xa5);state_buffer.fill(0xa5);
        put(raw_actor,8,actor.geometry.position);put(raw_actor,0x40,actor.geometry.direction_40);put(raw_actor,0x44,actor.geometry.direction_44);
        put(raw_actor,0x350,actor.geometry.pair_350);put(raw_actor,0x360,actor.geometry.pair_360);
        put(raw_actor,0x60,actor.effect_60);put(raw_actor,0x78,model_vtable.data());put(raw_actor,0x3a,identifier);
        put(raw_state,8,state.elapsed_08);put(raw_state,0x18,state.origin_y_18);put(raw_state,0x20,state.origin_x_20);put(raw_state,0x28,state.reached_boundary_28);
        events.clear(); valid=true; source_actor=nullptr;
    };
    auto compare = [&](BouncingActor actor,ActorBouncingState state,double time,int horizontal,bool only_collision) {
        const double original_budget = time;
        prepare(actor,state); const auto before=actor_buffer;const auto before_state=state_buffer;
        budget=&time;
        const auto result=only_collision?collision(raw_state,raw_actor,horizontal):update(raw_state,raw_actor,&time);
        const auto native_events=events;const bool native_valid=valid;const double native_time=time;
        auto expected_actor=before;auto expected_state=before_state;
        double source_time = original_budget;
        source_actor=&actor;source_state=&state;budget=&source_time;events.clear();valid=true;
        const auto expected=only_collision?resolve_actor_bounce_collision(state,actor,horizontal,grid,table,effects):update_actor_bouncing(state,actor,source_time,grid,table,effects);
        put(expected_actor.data(),8,actor.geometry.position);put(expected_actor.data(),0x40,actor.geometry.direction_40);put(expected_actor.data(),0x44,actor.geometry.direction_44);
        put(expected_actor.data(),0x350,actor.geometry.pair_350);put(expected_actor.data(),0x360,actor.geometry.pair_360);put(expected_actor.data(),0x60,actor.effect_60);
        put(expected_state.data(),8,state.elapsed_08);put(expected_state.data(),0x18,state.origin_y_18);put(expected_state.data(),0x20,state.origin_x_20);put(expected_state.data(),0x28,state.reached_boundary_28);
        ++cases;
        if (!valid || !native_valid || result!=expected || bits(native_time)!=bits(source_time) || native_events!=events || actor_buffer!=expected_actor || state_buffer!=expected_state) {
            std::cerr << "bounce mismatch case=" << cases << " script=" << script << " collision=" << only_collision << " result=" << result << '/' << expected << " events=" << native_events.size() << '/' << events.size() << " time=" << native_time << '/' << source_time << '\n';
            for(unsigned i=0;i<native_events.size()&&i<events.size();++i)if(!(native_events[i]==events[i])) {std::cerr << "event " << i << " kind=" << native_events[i].kind << '/' << events[i].kind << " a=" << native_events[i].a << '/' << events[i].a << " b=" << native_events[i].b << '/' << events[i].b << " y=" << native_events[i].y << '/' << events[i].y << '\n';break;}
            return false;
        }
        return true;
    };
    // Test entry separately, including nearest-actor selection and tied distances.
    for(unsigned i=0;i<5000;++i) {
        BouncingActor actor; ActorBouncingState state;
        actor.geometry.position={double(int(random()%2000)-500)/4,double(random()%100)/4,0.5};
        actor.geometry.pair_360={double(int(random()%9)-4)/4,double(int(random()%9)-4)/4};
        actor.geometry.direction_44=random()&1?1:-1;state.elapsed_08=0.25;state.reached_boundary_28=random();
        script=random()%16;identifier=static_cast<std::int16_t>(random());ids={-1,0,1,2,3};
        if(i<16) {
            actor.geometry.position={4,3,0.5};actor.geometry.pair_360={0,0};
            actor.geometry.direction_44=1;script=i;
        }
        for(unsigned j=0;j<4;++j) {
            nearby[j]={{double(int(random()%200000)-100000),double(int(random()%200000)-100000),0},int(random()%20),static_cast<std::uint8_t>(random())};
            if(i<16) {
                const double target_y=(script&4)?3:1;
                nearby[j]={{std::array<double,4>{5,3,20,30}[j],target_y,0},0x0e,0};
                if(i&1)nearby[0].primary_type_14=0;
                // Wrapped squared distance becomes negative and wins the
                // signed comparison, despite this actor being much farther.
                if((i&2) && j==3)nearby[j].position[0]=4+46341;
            }
            raw_nearby[j].fill(0xa5);raw_nearby_state[j].fill(0xa5);
            put(raw_nearby[j].data(),8,nearby[j].position);put(raw_nearby[j].data(),0x340,raw_nearby_state[j].data());
            put(raw_nearby_state[j].data(),0x14,nearby[j].primary_type_14);put(raw_nearby_state[j].data(),0x18,nearby[j].primary_flag_18);
        }
        NativeIntArray list{int(ids.size()),int(ids.size()),1,ids.data()};std::memcpy(reinterpret_cast<void*>(0x4c89e8),&list,sizeof(list));
        prepare(actor,state);auto expected_actor=actor_buffer;auto expected_state=state_buffer;
        enter(raw_state,raw_actor);const auto native_events=events;const bool native_valid=valid;
        source_actor=&actor;source_state=&state;events.clear();valid=true;enter_actor_bouncing(state,actor,grid,effects);
        put(expected_actor.data(),0x44,actor.geometry.direction_44);
        put(expected_state.data(),0x18,state.origin_y_18);put(expected_state.data(),0x20,state.origin_x_20);
        if(!valid || !native_valid || native_events!=events || actor_buffer!=expected_actor || state_buffer!=expected_state) {std::cerr<<"bounce entry mismatch "<<i<<'\n';return 1;}
        for(unsigned j=0;j<4;++j)if(get<std::uint8_t>(raw_nearby_state[j].data(),0x18)!=nearby[j].primary_flag_18)return 1;
        if(i<16)for(unsigned j=0;j<4;++j) {
            const unsigned selected=(i&2)?3:0;
            const bool expected_flag=j==selected && !((i&1) && selected==0);
            if(nearby[j].primary_flag_18!=expected_flag)return 1;
        }
        events.clear();source_actor=nullptr;has_state=static_cast<std::uint8_t>(random());leave(raw_state,raw_actor);const auto leave_events=events;
        events.clear();source_actor=&actor;leave_actor_bouncing(actor,effects);if(leave_events!=events || !valid)return 1;
        ++entries;
    }
    const double nan=std::numeric_limits<double>::quiet_NaN();
    for(double time:{-1.0,-0.0,0.0,nan})for(std::uint8_t flag:{std::uint8_t{0},std::uint8_t{1},std::uint8_t{0x80}})
        for(unsigned mode:{0u,1u,2u,3u,8u,9u,10u}) {
            cells.fill(0);BouncingActor actor;ActorBouncingState state;
            actor.geometry.position={128,15,0.5};actor.effect_60=flag;
            state.origin_y_18=5;state.reached_boundary_28=0x80;script=mode;wall=0;
            if(!compare(actor,state,time,0,false))return 1;
        }
    // Collision priority and the exact final-time/unordered branch are rare
    // in random tests. Cover them explicitly with distinct head/foot cells.
    for(int floor : {0,1,0x80,0x1000,0x1080,-1}) for(int head : {0,1})
        for(double elapsed : {0.0,1.4,std::nextafter(1.4,2.0),table.times[49],nan})
            for(unsigned horizontal : {0u,1u,0x80u}) for(unsigned mode : {0u,2u,8u,10u}) {
                cells.fill(0); cells[8*256+4]=floor; cells[12*256+4]=head;
                BouncingActor actor; ActorBouncingState state;
                actor.geometry.position={4,9,0.5};actor.geometry.pair_350={0,3};
                state.elapsed_08=elapsed;state.origin_y_18=9;state.reached_boundary_28=0x80;
                script=mode;wall=horizontal == 1 ? 0 : 0x80;
                if(!compare(actor,state,0.25,horizontal,true))return 1;
            }
    // Every vertical boundary, both sides of the apex, exact duration, zero
    // X steps, and partial steps. Use known valid intervals for the update.
    for(unsigned interval=0;interval<49;++interval)for(int dir:{-1,1})
        for(double time:{0.001,0.02,0.25}) {
            cells.fill(0);BouncingActor actor;ActorBouncingState state;
            actor.geometry.position={128,15,0.5};actor.geometry.direction_40=dir;
            state.elapsed_08=table.times[interval];state.origin_y_18=5;
            actor.effect_60=1;script=0;wall=0;
            if(!compare(actor,state,time,0,false))return 1;
        }
    for(double x:{nan,std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),-0.0,1.0,2147483647.0,2147483648.0,4294967296.0,9223372036854775808.0,-9223372036854775808.0})
        for(int dir:{-1,0,1,2})for(std::uint8_t flag:{std::uint8_t{0},std::uint8_t{255}}) {
            BouncingActor actor;ActorBouncingState state;actor.geometry.position[0]=x;actor.geometry.direction_40=dir;state.reached_boundary_28=flag;prepare(actor,state);
            if(bits(tx(raw_state,raw_actor))!=bits(static_cast<double>(actor_bounce_time_to_x(state,actor)))) {std::cerr<<"bounce X helper mismatch\n";return 1;}++helpers;
        }
    for(unsigned i=0;i<50;++i)for(double elapsed:{table.times[i],std::nextafter(table.times[i],-INFINITY),std::nextafter(table.times[i],INFINITY),nan})for(std::uint8_t flag:{std::uint8_t{0},std::uint8_t{255}}) {
        BouncingActor actor;ActorBouncingState state;state.elapsed_08=elapsed;state.reached_boundary_28=flag;prepare(actor,state);
        int native_height=99,source_height=99;const auto native_time=ty(raw_state,&native_height);const auto source_time=actor_bounce_time_to_y(state,table,source_height);
        if(bits(native_time)!=bits(static_cast<double>(source_time)) || native_height!=source_height) {std::cerr<<"bounce Y helper mismatch "<<i<<'\n';return 1;}++helpers;
    }
    // The two x87 interval comparisons differ on unordered input: an
    // unordered lower bound is rejected, but an unordered upper bound matches.
    // Also exercise the zero/unordered retry against the following table entry.
    const auto normal_times = table.times;
    for(unsigned slot : {0u,1u,24u,25u,48u,49u})
        for(double value : {nan,std::numeric_limits<double>::infinity(),
                            -std::numeric_limits<double>::infinity(),0.0})
            for(double elapsed : {0.0,normal_times[23],normal_times[24],normal_times[47]})
                for(std::uint8_t flag : {std::uint8_t{0},std::uint8_t{255}}) {
                    table.times = normal_times;
                    table.times[slot] = value;
                    std::memcpy(reinterpret_cast<void*>(0x4c5410),table.times.data(),400);
                    BouncingActor actor;ActorBouncingState state;
                    state.elapsed_08=elapsed;state.reached_boundary_28=flag;prepare(actor,state);
                    int native_height=99,source_height=99;
                    const auto native_time=ty(raw_state,&native_height);
                    const auto source_time=actor_bounce_time_to_y(state,table,source_height);
                    if(bits(native_time)!=bits(static_cast<double>(source_time)) || native_height!=source_height) {
                        std::cerr<<"bounce mutated Y table mismatch slot="<<slot<<'\n';return 1;
                    }
                    ++helpers;
                }
    table.times = normal_times;
    std::memcpy(reinterpret_cast<void*>(0x4c5410),table.times.data(),400);
    for(unsigned i=0;i<20000;++i) {
        BouncingActor actor;ActorBouncingState state;
        for(auto& cell:cells)cell=random()%16==0?std::array<int,6>{1,0x80,0x1000,0x1080,0x2000,-1}[random()%6]:0;
        actor.geometry.position={double(random()%1024)/4,double(random()%96)/4,0.5};
        actor.geometry.direction_40=random()&1?1:-1;actor.geometry.direction_44=random()&1?1:-1;
        actor.geometry.pair_350={double(int(random()%5)-2)/4,double(random()%13)/4};actor.geometry.pair_360={double(int(random()%5)-2)/4,double(random()%5)/4};
        actor.effect_60=random()%256;state.elapsed_08=double(random()%2700)/1000;state.origin_y_18=16;state.origin_x_20=99;state.reached_boundary_28=random();
        script=random()%16;wall=static_cast<std::uint8_t>(random());
        const double original_budget=i<4?std::array<double,4>{-1,-0.0,nan,0}[i]:double(random()%400)/1000;
        if(!compare(actor,state,original_budget,random()%256,i&1))return 1;
    }
    std::cout<<"native bouncing passed "<<cases<<" update/collision traces, "<<entries<<" entry/leave pairs, "<<helpers<<" helpers and "<<tables<<" tables\n";
    VirtualFree(image,0,MEM_RELEASE);
}
