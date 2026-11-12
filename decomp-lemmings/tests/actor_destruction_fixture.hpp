#pragma once
#include "actor_clear_fixture.hpp"
#include "lemmings/actor_destruction.hpp"

namespace destruction_fixture {
using namespace lemmings;
enum EventKind : unsigned { shadow=1, refresh, list, model, unregistration, behavior };
struct Event {unsigned kind;std::vector<std::uintptr_t> values;bool operator==(const Event&)const=default;};
struct Fixture : ActorDestructionCallbacks {
    actor_clear_test::Fixture clear;
    ActorDestructionState actor{clear.actor};
    std::array<std::uint16_t,4> counts{0,1,32768,65535};
    std::array<void*,4> slots{};
    std::int32_t first_id=-2;
    unsigned mutations=0;
    std::vector<Event> events;
    bool valid=true;
    void initialize(unsigned size,unsigned mode,bool has_shadow,unsigned changes,std::int32_t first=-2) {
        clear.initialize(size,mode);first_id=first;mutations=changes;
        actor.original_vtable=0xaaaa0000;actor.category_4c=0;
        actor.shadow_344=has_shadow?reinterpret_cast<void*>(0x12340000):nullptr;
        clear.actor.identifier_3a=static_cast<std::int16_t>(first+2);
        for(unsigned i=0;i<4;++i)slots[i]=reinterpret_cast<void*>(0x4000000+i*16);
        clear.sync=[this]{record(behavior);};
    }
    std::vector<std::uintptr_t> snapshot()const {
        std::vector<std::uintptr_t> result{actor.original_vtable,static_cast<std::uint32_t>(actor.category_4c),
            reinterpret_cast<std::uintptr_t>(actor.shadow_344),static_cast<std::uint16_t>(clear.actor.identifier_3a)};
        for(auto n:counts)result.push_back(n);
        for(auto p:slots)result.push_back(reinterpret_cast<std::uintptr_t>(p));
        const auto links=clear.snapshot();result.insert(result.end(),links.begin(),links.end());return result;
    }
    void record(unsigned kind,std::initializer_list<std::uintptr_t> args={}) {
        auto values=snapshot();values.insert(values.end(),args);events.push_back({kind,std::move(values)});
        if(!(mutations&(1u<<kind)))return;
        actor.original_vtable=0x55550000+kind;
        actor.category_4c=static_cast<std::int32_t>(events.size()%4);
        actor.shadow_344=reinterpret_cast<void*>(0x7000000+events.size()*16);
        clear.actor.identifier_3a=static_cast<std::int16_t>(first_id+events.size()%4);
        for(unsigned i=0;i<4;++i)counts[i]=static_cast<std::uint16_t>(events.size()+i);
    }
    ActorBehaviorClearHooks hooks() {return {this,[](void* context,ActorBehaviorNode& node){static_cast<Fixture*>(context)->clear.release(node);}};}
    ActorIdRegistry registry(){return {slots,first_id};}
    void destroy_shadow_42fec0(ActorDestructionState& a,void* p,std::uint32_t flags)override{valid&=&a==&actor;record(shadow,{reinterpret_cast<std::uintptr_t>(p),flags});}
    void refresh_444abc(ActorDestructionState& a)override{valid&=&a==&actor;record(refresh);}
    void destroy_extra_list_430110(ActorDestructionState& a)override{valid&=&a==&actor;record(list);}
    void destroy_model_43ce88(ActorDestructionState& a)override{valid&=&a==&actor;record(model);}
    void unregister_world_43b536(ActorDestructionState& a)override{valid&=&a==&actor;record(unregistration);}
};
} // namespace destruction_fixture
