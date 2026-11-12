#pragma once
#include "lemmings/actor_geometry.hpp"
#include "lemmings/actor_update.hpp"
#include <string_view>

namespace lemmings {
struct ActorWalkingState { std::uint8_t reached_boundary_18 = 0; };
struct WalkingActor {
    ActorGeometryState geometry;
    std::int32_t mode_50 = 0;
    std::uint8_t climbable_48 = 0, special_4a = 0;
    void* gadget_54 = nullptr;
    void* gadget_58 = nullptr;
    std::uint8_t depth_adjust_5e = 0;
    void* model_78 = nullptr;
    ActorBehavior* primary_340 = nullptr;
    void* context = nullptr;
};
struct ActorWalkingHooks {
    void* context = nullptr;
    void (*set_animation)(void*, WalkingActor&, std::string_view) = nullptr;
    void (*advance_animation)(void*, WalkingActor&, double) = nullptr;
    std::int32_t (*collision)(void*, ActorWalkingState&, WalkingActor&) = nullptr;
    void (*surface)(void*, ActorWalkingState&, WalkingActor&) = nullptr;
    bool (*gadget_contact)(void*, void*, WalkingActor&) = nullptr;
    void (*finish)(void*, WalkingActor&) = nullptr;
};
void enter_actor_walking(WalkingActor&, const ActorWalkingHooks&); // 0x41BDBA
double walking_boundary_time(const ActorWalkingState&, const WalkingActor&); // 0x41BF61
// 0x41BDD4. Callbacks are required. Retains the original possible no-progress
// loops; callers must supply a terminating state/time/callback combination.
std::int32_t update_actor_walking(ActorWalkingState&, WalkingActor&, double& remaining,
                                const ActorWalkingHooks&);
struct ActorWalkingSurfaceHooks {
    void* context = nullptr;
    std::int32_t (*gadget_count)(void*) = nullptr; // 0x41CB90
    void* (*gadget_at)(void*, std::int32_t) = nullptr; // 0x41CB70
    std::uint8_t (*gadget_contains)(void*, void*, const std::array<double,3>&) = nullptr; // 0x430BA1
};
// 0x41C875. Grid storage and callbacks must be valid. Finite X required by
// the native comparison domain; the original positive-infinity wrap loops.
void track_actor_walking_surface(WalkingActor&, CollisionGridHead&, const ActorWalkingSurfaceHooks&);
// 0x42F977. Targets project globals 0x4C5B30 and 0x4C5B20 respectively.
void approach_actor_walking_depth(WalkingActor&, double target_direction_minus_one, double target_other);
} // namespace lemmings
