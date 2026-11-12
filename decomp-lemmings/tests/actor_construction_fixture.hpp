#pragma once
#include "lemmings/actor_construction.hpp"
#include "lemmings/model_construction.hpp"
#include <bit>
#include <cassert>
#include <vector>

namespace construction_fixture {
using namespace lemmings;
enum EventKind : unsigned { base = 1, model, list, allocation, shadow, resource,
    refresh, monitor, falling, attachment, sync_position, registration };
template<class T = void> T* token(std::uintptr_t value) { return reinterpret_cast<T*>(value); }
inline std::uint64_t bits(double value) { return std::bit_cast<std::uint64_t>(value); }
inline std::vector<std::uint64_t> snapshot(const ActorConstructionState& a) {
    std::vector<std::uint64_t> result{a.original_vtable};
    for(auto value : a.geometry.position) result.push_back(bits(value));
    result.insert(result.end(), {static_cast<std::uint32_t>(a.geometry.direction_40),
        static_cast<std::uint32_t>(a.geometry.direction_44),static_cast<std::uint32_t>(a.geometry.category_4c),
        a.climb_48,a.umbrella_49,a.flag_4a,static_cast<std::uint32_t>(a.mode_50),
        reinterpret_cast<std::uintptr_t>(a.gadget_54),a.geometry.flag_5c,a.forced_5d,
        a.flag_5e,a.flag_5f,a.flag_60,bits(a.pair_68[0]),bits(a.pair_68[1]),
        reinterpret_cast<std::uintptr_t>(a.primary_340),reinterpret_cast<std::uintptr_t>(a.shadow_344),a.unknown_348[0],
        bits(a.geometry.pair_350[0]),bits(a.geometry.pair_350[1]),
        bits(a.geometry.pair_360[0]),bits(a.geometry.pair_360[1]),a.flag_370,a.flag_38c,
        a.unknown_pair_390[0],a.unknown_pair_390[1]});
    return result;
}
struct Event {
    unsigned kind;
    std::vector<std::uint64_t> values;
    bool operator==(const Event&) const = default;
};
struct Effects : ActorConstructionCallbacks {
    ActorConstructionState actor;
    ModelConstructionState model_state;
    bool concrete_model = false;
    std::array<std::int32_t,4> resources{51,-8,0,903};
    std::array<std::uint16_t,4> counts{0,32767,65535,218};
    std::vector<Event> events;
    unsigned mutation_mask = 0, failure_mask = 0, allocation_index = 0, attachment_index = 0;
    bool valid = true;
    ActorConstructionGlobals globals() { return {resources,counts}; }
    void record(unsigned kind,std::initializer_list<std::uint64_t> args={}) {
        auto values=snapshot(actor);
        for(auto value:resources)values.push_back(static_cast<std::uint32_t>(value));
        for(auto value:counts)values.push_back(value);
        values.insert(values.end(),args);
        events.push_back({kind,std::move(values)});
        if(!(mutation_mask & (1u<<kind)))return;
        // Effects deliberately mutate fields the constructor later overwrites
        // or rereads. This is a hostile but returning callback fixture.
        actor.geometry.position={kind+0.25,-double(kind),kind+0.75};
        actor.original_vtable=0x77770000+kind;
        actor.geometry.direction_40=-static_cast<int>(kind);
        actor.geometry.direction_44=static_cast<int>(kind)+3;
        actor.geometry.category_4c=700+kind;
        actor.climb_48=actor.umbrella_49=actor.flag_4a=static_cast<std::uint8_t>(kind+10);
        actor.mode_50=static_cast<int>(kind)+50;
        actor.gadget_54=token(0x80000+kind);
        actor.geometry.flag_5c=actor.forced_5d=actor.flag_5e=actor.flag_5f=actor.flag_60=static_cast<std::uint8_t>(kind+20);
        actor.pair_68={kind+1.25,kind+2.5};
        actor.primary_340=token<ActorBehavior>(0x90000+kind);
        actor.shadow_344=token(0xa0000+kind);
        actor.unknown_348={0x1234567800000000ULL+kind};
        actor.geometry.pair_350={kind+3.25,kind+4.5};
        actor.geometry.pair_360={kind+5.25,kind+6.5};
        actor.flag_370=actor.flag_38c=static_cast<std::uint8_t>(kind+30);
        actor.unknown_pair_390={kind,kind+1};
        for(unsigned i=0;i<4;++i){resources[i]=static_cast<int>(kind*100+i);counts[i]=static_cast<std::uint16_t>(65535-kind+i);}
    }
    void construct_base_4230a2(ActorConstructionState& a,const std::array<double,3>& p) override {
        valid &= &a==&actor; record(base,{bits(p[0]),bits(p[1]),bits(p[2])});a.geometry.position=p;
    }
    void construct_model_43ce00(ActorConstructionState& a) override {
        valid&=&a==&actor;
        if(concrete_model)initialize_actor_model(model_state);
        else record(model);
    }
    void construct_extra_list_430090(ActorConstructionState& a) override {valid&=&a==&actor;record(list);}
    void* allocate_49c1a8(std::uint32_t n) override {
        record(allocation,{n,allocation_index});const auto index=allocation_index++;
        return failure_mask&(1u<<index)?nullptr:token(0x100000+index*0x1000);
    }
    void* construct_shadow_457f40(void* p,ActorConstructionState& a) override {
        valid&=&a==&actor;record(shadow,{reinterpret_cast<std::uintptr_t>(p)});
        return failure_mask&8?nullptr:token(0x200000);
    }
    void bind_resource_46836d(ActorConstructionState& a,std::int32_t id) override {valid&=&a==&actor;record(resource,{static_cast<std::uint32_t>(id)});}
    void refresh_444abc() override {record(refresh);}
    ActorBehavior* construct_monitor_4151f0(void* p) override {
        record(monitor,{reinterpret_cast<std::uintptr_t>(p)});return failure_mask&16?nullptr:token<ActorBehavior>(0x201000);
    }
    ActorBehavior* construct_falling_415fa0(void* p) override {
        record(falling,{reinterpret_cast<std::uintptr_t>(p)});return failure_mask&32?nullptr:token<ActorBehavior>(0x202000);
    }
    void attach_42e696(ActorConstructionState& a,ActorBehavior* state) override {
        valid&=&a==&actor;record(attachment,{reinterpret_cast<std::uintptr_t>(state),attachment_index});
        if(attachment_index++==1)a.primary_340=state;
    }
    void sync_model_position_08(ActorConstructionState& a,std::array<double,3>& p) override {
        valid&=&a==&actor && &p==&actor.geometry.position;record(sync_position,{bits(p[0]),bits(p[1]),bits(p[2])});
    }
    void register_actor_43b517(ActorConstructionState& a) override {valid&=&a==&actor;record(registration);}
};
inline ActorConstructionState dirty_actor() {
    Effects fixture;fixture.mutation_mask=1u<<registration;fixture.record(registration);
    fixture.actor.geometry.position={-0.0,42.25,99};
    return fixture.actor;
}
} // namespace construction_fixture
