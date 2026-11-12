#pragma once

#include "lemmings/actor_terminal.hpp"

namespace lemmings {
// Primary behavior type0x2E, selected by the spawned monitor when a region
// matches. Its gameplay name remains unestablished.
using ActorRegionTerminalState = ActorTerminalState;
struct ActorRegionTerminalEntryCallbacks : ActorTerminalEntryCallbacks {
    // This entry shares the effect API but never issues terminal1A's event.
    void event_44535b(TerminalActor&,std::int16_t,std::int32_t,std::uint8_t) override {}
    virtual void set_flag(void* effect,std::uint8_t flag) = 0; // 0x401F30/0x407710
};
void initialize_actor_region_terminal(ActorRegionTerminalState&,ActorBehavior&); //0x406C20
// 0x406C6C: captured offset=position+(0,4,1); effect/entity positions read
// the live actor position at each call. Resource/effect internals are callbacks.
void enter_actor_region_terminal(TerminalActor&,ActorRegionTerminalEntryCallbacks&);
// 0x4071F6 has the same update sequence as terminal1A's0x40B9F2.
std::uint32_t update_actor_region_terminal(ActorRegionTerminalState&,TerminalActor&,
    double& remaining,ActorTerminalUpdateCallbacks&);
void leave_actor_region_terminal(); //0x407260, empty
} // namespace lemmings
