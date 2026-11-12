#pragma once
#include "lemmings/actor_geometry.hpp"
#include "lemmings/actor_update.hpp"
#include <string_view>

namespace lemmings {
// Original constructor does not initialize +18. Zero is an explicit source
// fixture default; replay adapters can supply the observed byte instead.
struct ActorClimbingState { std::uint8_t reached_boundary_18 = 0; };
struct ClimbingActor {
    ActorGeometryState geometry;
    std::int32_t mode_50 = 0;
    void* model_78 = nullptr;
    void* context = nullptr;
};
enum class ClimbingTransition { falling_415fa0, topping_417c80 };
enum class ClimbingNotice { boundary_position, hit_head, topping_out, topping_position, climb_over };
struct ActorClimbingHooks {
    void* context = nullptr;
    void (*set_animation)(void*, ClimbingActor&, std::string_view) = nullptr;
    void (*advance_animation)(void*, ClimbingActor&, double) = nullptr;
    void (*notice)(void*, ClimbingActor&, ClimbingNotice) = nullptr;
    ActorBehavior* (*create_state)(void*, ClimbingTransition) = nullptr;
    void (*attach_state)(void*, ClimbingActor&, ActorBehavior*) = nullptr;
};
void enter_actor_climbing(ClimbingActor&, const ActorClimbingHooks&); // 0x4137A3
double climbing_boundary_time(const ActorClimbingState&, const ClimbingActor&); // 0x413924
std::int32_t resolve_actor_climbing_collision(ClimbingActor&, CollisionGridHead&,
                                             const ActorClimbingHooks&); // 0x4139BF
// 0x4137CA. Effects callbacks required; no artificial loop-progress limit.
std::int32_t update_actor_climbing(ActorClimbingState&, ClimbingActor&, double& remaining,
                                  CollisionGridHead&, const ActorClimbingHooks&);
} // namespace lemmings
