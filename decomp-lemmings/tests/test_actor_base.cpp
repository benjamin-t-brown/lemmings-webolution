#include "lemmings/actor_base.hpp"
#include <bit>
#include <cassert>
#include <iostream>

namespace {
using namespace lemmings;
using Bits = std::array<std::uint64_t,3>;
struct Registry {
    ActorBaseState* expected;
    std::uint16_t* next;
    ActorBaseState seen{};
    std::uint16_t seen_next = 0;
    unsigned calls = 0;
    bool mutate = false;
    static void insert(void* context,ActorBaseState*& argument) {
        auto& self=*static_cast<Registry*>(context);
        assert(argument==self.expected);self.seen=*argument;self.seen_next=*self.next;++self.calls;
        if(self.mutate) {
            argument->id_3a=44;argument->flag_28=17;argument->field_20=-1;
            *self.next=98;argument=nullptr;
        }
    }
};
}
int main() {
    for(auto initial:{std::uint16_t{0},std::uint16_t{32767},std::uint16_t{65535}}) {
        ActorBaseState actor;actor.position={9,8,7};actor.flag_28=99;actor.flag_38=201;
        actor.field_20=-0.0;actor.field_30=987;
        auto next=initial;Registry registry{&actor,&next};
        const Bits input_bits{0x8000000000000000ULL,0x7ff0000000000001ULL,0xfff0000000000000ULL};
        const auto input=std::bit_cast<std::array<double,3>>(input_bits);
        auto& result=construct_actor_base(actor,input,next,{&registry,Registry::insert});
        assert(&result==&actor && registry.calls==1);
        assert(std::bit_cast<Bits>(actor.position)==input_bits);
        assert(actor.original_vtable==0x4b0e90 && !actor.flag_38 && actor.flag_28==1);
        assert(std::bit_cast<std::uint64_t>(actor.field_20)==0 && std::bit_cast<std::uint64_t>(actor.field_30)==0);
        assert(actor.id_3a==initial && next==static_cast<std::uint16_t>(initial+1u));
        assert(registry.seen.id_3a==initial && registry.seen_next==next);
    }
    ActorBaseState actor;actor.position={11,12,13};std::uint16_t next=65535;
    Registry registry{&actor,&next};registry.mutate=true;
    auto& result=construct_actor_base(actor,actor.position,next,{&registry,Registry::insert});
    assert(&result==&actor && registry.calls==1);
    assert((std::bit_cast<Bits>(registry.seen.position)==Bits{0,0,0}));
    assert(registry.seen.id_3a==65535 && registry.seen_next==0);
    assert(next==98 && actor.id_3a==44 && actor.flag_28==17 && actor.field_20==-1);
    std::cout<<"actor base construction units passed\n";
}
