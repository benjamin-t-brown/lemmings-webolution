#include "lemmings/actor_finalization.hpp"
#include <cassert>
#include <bit>
#include <limits>
#include <stdexcept>
#include <vector>
using namespace lemmings;
int main() {
    WorldGeometryPrefix world;
    world.byte_array.resize(128*8);
    world.byte_array[128*3+5]=7;
    for(unsigned i=0;i<24;++i) world.fixed_bytes[7*24+i]=static_cast<std::uint8_t>(i+1);
    const auto table=world_sample_table(world);
    const auto expected=SampleTriple{0x0807060504030201,0x100f0e0d0c0b0a09,0x1817161514131211};
    assert(world_sample_triple(table,10,3)==expected);
    assert(world_sample_triple(table,267,3)==expected);
    assert(world_sample_triple(table,10,0x1000003)==expected); // 32-bit shift wrap.
    std::array<AnimationClip,3> clips{{{1,1,"walk"},{1,1,"drown"},{1,1,"Drown"}}};
    AnimationResource resource{clips};
    ActorFinalizationState actor;
    actor.geometry.position={10.75,3.75,0.5}; actor.animation.resource_1f8=&resource;
    std::vector<int> events;
    ActorFinalizationHooks hooks;
    hooks.context=&events;
    hooks.has_behavior=[](void* p,ActorFinalizationState&,int type)->std::uint8_t{
        assert(type==0x2B);static_cast<std::vector<int>*>(p)->push_back(1);return 0;
    };
    hooks.model_position_08=[](void* p,ActorFinalizationState&){static_cast<std::vector<int>*>(p)->push_back(2);};
    hooks.effect_458186=[](void* p,ActorFinalizationState&){static_cast<std::vector<int>*>(p)->push_back(3);};
    finalize_actor_update(actor,table,hooks);
    assert(events==std::vector<int>({1,2,3}));
    for(const auto& triple:actor.model_triples) assert(triple==expected);
    actor.animation.clip_1c8=1; actor.model_triples[0]={1,2,3};
    actor.geometry.position[1]=-100; // Drown skips otherwise invalid table lookup.
    actor.geometry.direction_44=-1;
    finalize_actor_update(actor,table,hooks);
    assert(actor.orientation_2d8==1 && actor.model_triples[0]==SampleTriple({1,2,3}));
    actor.animation.clip_1c8=2; actor.geometry.position={10.75,3.75,0.5};
    finalize_actor_update(actor,table,hooks);
    assert(actor.model_triples[0]==expected); // Case-sensitive comparison.
    events.clear();actor.geometry.direction_44=1;
    hooks.has_behavior=[](void* p,ActorFinalizationState& a,int)->std::uint8_t {
        static_cast<std::vector<int>*>(p)->push_back(1);a.geometry.direction_44=-1;return 0;
    };
    finalize_actor_update(actor,table,hooks);
    assert(events==std::vector<int>({1,1,2,3}) && actor.orientation_2d8==1);
    bool rejected=false;
    try {world_sample_triple(table,0,-1);} catch(const std::out_of_range&) {rejected=true;}
    assert(rejected);
}
