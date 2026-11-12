#pragma once

#include "lemmings/actor_walking.hpp"

#include <string_view>

namespace lemmings {

enum class WalkingCollisionTransition {
    gadget_41ad90, // 0x20-byte allocation.
    falling_415fa0, // 0x28-byte allocation.
    gadget_4041fd, // 0x20-byte allocation.
    climbing_413750 // 0x20-byte allocation; replaces primary directly.
};

struct ActorWalkingCollisionCallbacks {
    virtual ~ActorWalkingCollisionCallbacks() = default;
    virtual std::uint8_t has_state_42ecc9(WalkingActor& actor, std::int32_t id) = 0;
    virtual std::int32_t gadget_count_41cb60() = 0;
    virtual void* gadget_at_41cb40(std::int32_t index) = 0;
    virtual std::uint8_t gadget_contains_430ba1(void* gadget, const std::array<double, 3>& point) = 0;
    virtual std::uint8_t gadget_enabled_41cba0(void* gadget) = 0; // Reads gadget +38.
    virtual void activate_gadget_20(void* gadget) = 0;
    // Projections of direct object field accesses, not original calls.
    virtual void set_gadget_actor_a4(void* gadget, WalkingActor& actor) = 0;
    virtual std::int32_t gadget_mode_128(void* gadget) = 0;
    virtual std::string_view gadget_animation_406500(void* gadget) = 0;
    virtual void select_animation_42fd25(WalkingActor& actor, std::string_view name) = 0;
    virtual void clear_states_42e78a(WalkingActor& actor) = 0;
    // Explicit allocation/constructor boundary; successful allocation required.
    virtual ActorBehavior* create_state(WalkingCollisionTransition type) = 0;
    virtual void attach_42e696(WalkingActor& actor, ActorBehavior* state) = 0;
    virtual void enter_state_08(ActorBehavior* state, WalkingActor& actor) = 0;
    virtual void move_y_42ee74(WalkingActor& actor, double y) = 0;
};

// 0x0041BFFC. Returns 2 outside vertical bounds, 1 for a state replacement,
// otherwise 0. Original sampling/coordinate helpers and terrain decisions are
// implemented directly; gadget systems and state/animation effects are callbacks.
// Requires positive grid height/backing storage and valid callback handles.
// The 0x8000 path requires a nonempty gadget list if state 0x2B is absent: the
// original tests the last visited gadget even when no point containment matched.
std::int32_t resolve_actor_walking_collision(WalkingActor& actor,
    CollisionGridHead& grid, ActorWalkingCollisionCallbacks& callbacks);

} // namespace lemmings
