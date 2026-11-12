#include "actor_destruction_fixture.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace destruction_fixture;
    Fixture f;f.initialize(2,0,true,0);
    destroy_actor(f.actor,f.counts,f.registry(),f.hooks(),f);
    assert(f.valid && f.actor.original_vtable==0x4b0e90 && f.clear.actor.primary==nullptr);
    assert(f.actor.shadow_344==reinterpret_cast<void*>(0x12340000));
    assert(f.counts[0]==65535 && f.counts[1]==1 && f.slots[2]==nullptr && f.slots[1]!=nullptr);
    const std::vector<std::array<std::size_t,5>> expected_trace{
        {1,0,0,0,17},{1,1,0,0,17},{2,0,1,0,17},{3,0,0,0,17},
        {2,1,1,0,17},{3,1,0,0,17},{1,17,0,101,17},{2,17,1,101,17}};
    assert(f.clear.trace==expected_trace);
    std::vector<unsigned> order;for(auto& e:f.events)if(e.kind!=behavior)order.push_back(e.kind);
    assert((order==std::vector<unsigned>{shadow,refresh,list,model,unregistration}));
    assert(f.events.front().values[0]==0x4b10e8 && f.events.back().values[0]==0x4b0e90);
    assert(f.events.back().values[10]==0); // Registry slot2 cleared before world callback.
    // Cleanup callbacks can change the category selected for the decrement.
    Fixture changed;changed.initialize(0,0,false,1u<<behavior);
    destroy_actor(changed.actor,changed.counts,changed.registry(),changed.hooks(),changed);
    assert(changed.actor.category_4c==2 && changed.counts[2]==3);
    // Signed IDs at both extrema map through an explicitly biased registry.
    for(auto first:{-32768,32764}) {
        Fixture base;base.initialize(0,0,false,0,first);
        base.clear.actor.identifier_3a=static_cast<std::int16_t>(first+(first<0?0:3));
        destroy_actor_base(base.actor,base.registry(),base);
        assert(base.slots[first<0?0:3]==nullptr && base.events.size()==1);
        assert(base.clear.actor.primary==&base.clear.states[17]); // Base teardown never clears behaviors.
    }
    std::cout<<"actor destruction units passed\n";
}
