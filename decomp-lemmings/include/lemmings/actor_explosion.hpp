#pragma once
#include "lemmings/actor_terminal.hpp"
namespace lemmings {
struct ExplosionActor:TerminalActor{std::array<double,2> pair_360{};};
// Reuses the established effect interface; this state uses only the first
// two effect kinds and child sample setters, not the second terminal effect.
struct ActorExplosionCallbacks:ActorTerminalEntryCallbacks {
    virtual void nearby_effect_41dd86(std::array<double,3>& sample,double radius)=0;
    virtual void world_notice_43b31e(std::int32_t,std::array<double,3>& position)=0;
    virtual void terrain_blast_43f510(std::array<double,3>& position,std::int32_t,double elapsed)=0;
    virtual void advance_effect_2c(void*,double elapsed)=0;
};
void initialize_actor_explosion(ActorTerminalState&,ActorBehavior&); //4081B0,type2
void enter_actor_explosion(ExplosionActor&,std::int32_t event_4e32c0,ActorExplosionCallbacks&); //4081FC
std::uint32_t update_actor_explosion(ExplosionActor&,double& remaining,ActorExplosionCallbacks&); //408227
void leave_actor_explosion(); //401320
}
