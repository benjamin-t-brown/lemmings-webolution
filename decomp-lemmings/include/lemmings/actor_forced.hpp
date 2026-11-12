#pragma once
#include "lemmings/actor_terminal.hpp"

namespace lemmings {
// Type 0x25 selected by actor update when actor+0x5D is set. The original
// gameplay name is unknown; graphics/resource ownership remains explicit.
struct ActorForcedCallbacks {
    virtual ~ActorForcedCallbacks() = default;
    virtual long double random_404080() = 0;
    virtual void* create_effect_416ed0() = 0; // allocation 0x148; nonnull required
    virtual void set_category_d8(void*, std::int32_t) = 0;
    virtual void set_count_417150(void*, std::int32_t) = 0;
    virtual std::array<double,3> world_position_43d56c(const std::array<double,3>&) = 0;
    virtual void set_direction_54(void*, const std::array<double,3>&) = 0;
    virtual void set_position_08(void*, const std::array<double,3>&) = 0;
    virtual void set_pair_4171f0(void*, double, double) = 0;
    virtual void set_mode_417230(void*, std::int32_t) = 0;
    virtual void advance_effect_2c(void*, double) = 0;
    virtual std::array<double,3> sample_triple_403f40(std::uint32_t, std::int32_t) = 0;
    virtual void* child_4018b0(void*, std::int32_t) = 0;
    virtual void set_child_triple_403820(void*, const std::array<double,3>&) = 0;
    virtual void* create_entity_432080(const std::array<double,3>&, void*, std::uint8_t) = 0;
    virtual void insert_entity_401890(void*&) = 0;
};
void initialize_actor_forced(ActorTerminalState&, ActorBehavior&); // 0x416BF0
void enter_actor_forced(); // 0x416C3C, empty
// 0x416C49: captures a randomized offset, creates one effect/entity and returns
// 2. The supplied time budget is read after effect setup and is not consumed.
std::uint32_t update_actor_forced(TerminalActor&, double& remaining, ActorForcedCallbacks&);
void leave_actor_forced(); // 0x401320, empty
} // namespace lemmings
