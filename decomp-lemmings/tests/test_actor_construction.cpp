#include "actor_construction_fixture.hpp"
#include <iostream>

int main() {
    using namespace construction_fixture;
    Effects effects;effects.actor=dirty_actor();
    const auto original=effects.actor;
    auto& result=construct_actor(effects.actor,-0.0,12.5,2,effects.globals(),effects);
    assert(&result==&effects.actor && effects.valid);
    assert(bits(result.geometry.position[0])==bits(-0.0));
    assert((result.geometry.position==std::array<double,3>{-0.0,12.5,0.5}));
    assert(result.geometry.category_4c==2 && result.mode_50==1);
    assert(result.original_vtable==0x4b10e8 && result.shadow_344==token(0x200000));
    assert(result.primary_340==token<ActorBehavior>(0x202000) && !result.gadget_54);
    assert(result.flag_5e==1 && result.flag_370==1 && !result.flag_38c);
    assert(!result.climb_48 && !result.umbrella_49 && !result.flag_4a && !result.forced_5d);
    assert(!result.geometry.flag_5c && !result.flag_5f && !result.flag_60);
    assert((result.geometry.pair_350==std::array<double,2>{0,3}));
    assert((result.geometry.pair_360==std::array<double,2>{0,0}));
    assert((result.pair_68==std::array<double,2>{0,0}));
    assert(result.unknown_348==original.unknown_348 && result.unknown_pair_390==original.unknown_pair_390);
    assert(effects.counts[2]==0 && effects.counts[1]==32767);
    const std::vector<unsigned> expected{base,model,list,allocation,shadow,resource,refresh,
        allocation,monitor,attachment,allocation,falling,attachment,sync_position,registration};
    std::vector<unsigned> actual;for(auto& event:effects.events)actual.push_back(event.kind);
    assert(actual==expected);
    // First attach sees the old category and primary. The second sees their
    // constructor writes. Resource binding happens before count increment.
    assert(effects.events[9].values[6]==static_cast<unsigned>(original.geometry.category_4c));
    assert(effects.events[9].values[19]==reinterpret_cast<std::uintptr_t>(original.primary_340));
    assert(effects.events[12].values[6]==2 && effects.events[12].values[19]==0);
    assert(effects.events[5].values.back()==0);
    for(unsigned failure=0;failure<64;++failure) {
        Effects trial;trial.actor=dirty_actor();trial.failure_mask=failure;
        construct_actor(trial.actor,1,2,1,trial.globals(),trial);
        assert(trial.valid && trial.allocation_index==3 && trial.attachment_index==2);
        assert(trial.actor.shadow_344==((failure&9)?nullptr:token(0x200000)));
        assert(trial.actor.primary_340==((failure&36)?nullptr:token<ActorBehavior>(0x202000)));
    }
    // Final register can replace fields, but mode50 is always written after it.
    Effects mutated;mutated.actor=dirty_actor();mutated.mutation_mask=1u<<registration;
    construct_actor(mutated.actor,1,2,0,mutated.globals(),mutated);
    assert(mutated.actor.mode_50==1 && mutated.actor.geometry.category_4c==712);
    assert(mutated.actor.climb_48==22 && mutated.actor.flag_38c==42);
    Effects composed;composed.actor=dirty_actor();composed.concrete_model=true;
    composed.model_state.buffer_08.fill(0xa5);composed.model_state.position_90={11,22,33};
    construct_actor(composed.actor,1,2,0,composed.globals(),composed);
    assert(composed.model_state.original_vtable==0x4b14a8 && composed.model_state.mode_88==1);
    assert(composed.model_state.buffer_08[0]==0 && composed.model_state.buffer_08[1]==0xa5);
    assert((composed.model_state.position_90==std::array<std::uint64_t,3>{11,22,33}));
    assert(composed.model_state.animation_b0.playback.resource_id_1fc==-1);
    for(const auto& event:composed.events)assert(event.kind!=model);
    std::cout<<"actor construction units passed\n";
}
