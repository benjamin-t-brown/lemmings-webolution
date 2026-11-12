#include "lemmings/actor_region_terminal.hpp"
#include <bit>
#include <cmath>

namespace lemmings {
namespace {
double rounded(long double value){volatile double result=static_cast<double>(value);return result;}
std::int32_t ftol_low(double value){
    if(!std::isfinite(value)||static_cast<long double>(value)<-9223372036854775808.0L||
        static_cast<long double>(value)>=9223372036854775808.0L)return 0;
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
std::array<double,3> direction(TerminalActor& actor,const std::array<double,3>& offset,
                              ActorRegionTerminalEntryCallbacks& callbacks){
    const auto first=callbacks.world_position_43d56c(actor.position_08);
    const auto second=callbacks.world_position_43d56c(offset);
    std::array<double,3> result;
    for(unsigned i=0;i<3;++i)result[i]=rounded(static_cast<long double>(second[i])-first[i]);
    return result;
}
}
void initialize_actor_region_terminal(ActorRegionTerminalState& state,ActorBehavior& behavior){
    state.elapsed_08=0;behavior.phase_10=0;behavior.extra_11=0;behavior.type_14=0x2e;
}
void enter_actor_region_terminal(TerminalActor& actor,ActorRegionTerminalEntryCallbacks& callbacks){
    auto offset=actor.position_08;
    offset[2]=rounded(static_cast<long double>(offset[2])+1.0L);
    offset[1]=rounded(static_cast<long double>(offset[1])+4.0L);
    auto* first=callbacks.create_effect(actor.category_4c==3?TerminalEffectKind::effect_4072a0:TerminalEffectKind::effect_4018e0);
    callbacks.set_count(first,6);
    callbacks.set_position_08(first,actor.position_08);
    callbacks.set_pair(first,4.0,0.5);
    callbacks.set_flag(first,1);
    callbacks.set_mode(first,12);
    callbacks.set_direction_54(first,direction(actor,offset,callbacks));
    const auto triple=callbacks.sample_triple_403f40(static_cast<std::uint32_t>(ftol_low(offset[0])),ftol_low(offset[1]));
    for(std::int32_t i=0;i<callbacks.child_count_4018d0(first);++i){
        auto* child=callbacks.child_4018b0(first,i);callbacks.set_child_triple_403820(child,triple);
    }
    auto* entity=callbacks.create_entity_432080(actor.position_08,first,1);
    callbacks.insert_entity_401890(entity);
    auto* second=callbacks.create_effect(TerminalEffectKind::effect_402870);
    callbacks.set_count(second,10);
    callbacks.set_triple_4017d0(second,{255,127,127});
    callbacks.set_position_08(second,actor.position_08);
    callbacks.set_pair(second,4.0,0.5);
    callbacks.set_mode(second,4);
    callbacks.set_direction_54(second,direction(actor,offset,callbacks));
    entity=callbacks.create_entity_432080(actor.position_08,second,1);
    callbacks.insert_entity_401890(entity);
}
std::uint32_t update_actor_region_terminal(ActorRegionTerminalState& state,TerminalActor& actor,
    double& remaining,ActorTerminalUpdateCallbacks& callbacks){return update_actor_terminal(state,actor,remaining,callbacks);}
void leave_actor_region_terminal(){}
} // namespace lemmings
