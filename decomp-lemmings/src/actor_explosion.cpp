#include "lemmings/actor_explosion.hpp"
#include "lemmings/actor_spawn_monitor.hpp"
#include <bit>
#include <cmath>
namespace lemmings {
namespace {
double rounded(long double v){volatile double r=static_cast<double>(v);return r;}
std::uint32_t ftol(double v){if(!std::isfinite(v)||static_cast<long double>(v)<-9223372036854775808.0L||static_cast<long double>(v)>=9223372036854775808.0L)return 0;return static_cast<std::uint32_t>(static_cast<std::int64_t>(v));}
}
void initialize_actor_explosion(ActorTerminalState& s,ActorBehavior& b){s.elapsed_08=0;b.phase_10=0;b.extra_11=0;b.type_14=2;}
void enter_actor_explosion(ExplosionActor& a,std::int32_t event,ActorExplosionCallbacks& c){c.event_44535b(a,a.identifier_3a,event,0);}
void leave_actor_explosion(){}
std::uint32_t update_actor_explosion(ExplosionActor& a,double& remaining,ActorExplosionCallbacks& c){
    ActorGeometryState geometry;geometry.position=a.position_08;geometry.pair_360=a.pair_360;
    auto sample=actor_monitor_sample_position(geometry);
    c.nearby_effect_41dd86(sample,3.0);
    auto position=a.position_08;
    position[0]=rounded(static_cast<long double>(position[0])+1.0L);
    position[1]=rounded(static_cast<long double>(position[1])+1.5L);
    c.world_notice_43b31e(0,position);
    position[0]=rounded(static_cast<long double>(position[0])-1.0L);
    c.terrain_blast_43f510(position,0,remaining);
    auto* effect=c.create_effect(a.category_4c==3?TerminalEffectKind::effect_4072a0:TerminalEffectKind::effect_4018e0);
    c.set_count(effect,6);c.set_position_08(effect,position);c.set_pair(effect,4.0,0.0);c.set_mode(effect,6);
    const auto first=c.world_position_43d56c(a.position_08);
    const auto second=c.world_position_43d56c(position);
    std::array<double,3> direction;
    for(unsigned i=0;i<3;++i)direction[i]=rounded(static_cast<long double>(second[i])-first[i]);
    c.set_direction_54(effect,direction);c.advance_effect_2c(effect,remaining);
    const auto y=std::bit_cast<std::int32_t>(ftol(position[1]));const auto x=ftol(position[0]);
    const auto triple=c.sample_triple_403f40(x,y);
    for(std::int32_t i=0;i<c.child_count_4018d0(effect);++i){auto* child=c.child_4018b0(effect,i);c.set_child_triple_403820(child,triple);}
    auto* entity=c.create_entity_432080(position,effect,1);c.insert_entity_401890(entity);
    remaining=0;return 2;
}
}
