#pragma once

#include "lemmings/actor_geometry.hpp"

#include <array>
#include <string_view>

namespace lemmings {

// Logical views; suffixes identify original offsets, not host layout.
// The original constructor initializes elapsed through its base, but leaves
// origins and +28 untouched. Their zero defaults are explicit fixture choices.
struct ActorBouncingState {
    double elapsed_08 = 0;
    double origin_y_18 = 0;
    double origin_x_20 = 0; // Written by entry; not read by this update.
    std::uint8_t reached_boundary_28 = 0;
};
struct BouncingActor {
    ActorGeometryState geometry;
    std::uint8_t effect_60 = 0;
    void* model_78 = nullptr; // Original model is inline at +78.
};
struct BounceTimeTable {
    std::array<double, 50> times{}; // Original 0x4C5410.
    // times[49] is also the duration global at 0x4C5598.
    double apex_time = 1.4;        // Original 0x4C55A8, initialized by 0x40EC2A.
};
struct BounceNearbyActor {
    std::array<double, 3> position{};
    std::int32_t primary_type_14 = 0;
    std::uint8_t primary_flag_18 = 0;
};
enum class BounceStateKind { effect_40e3c0, bounce_40ec7e, falling_415fa0 };
enum class BounceNotice {
    started, updating_to_integer, updated_by, new_position,
    checking_floor, hit_floor, hit_trampoline, hit_head, checking_wall, hit_wall
};
struct ActorBouncingCallbacks {
    virtual ~ActorBouncingCallbacks() = default;
    virtual void advance_model_2c(void* model, double elapsed) = 0;
    virtual std::int16_t actor_id_406a00(BouncingActor& actor) = 0;
    virtual void select_animation_42fd25(BouncingActor& actor, std::string_view name) = 0;
    virtual void notice_40f860(BounceNotice kind, double a = 0, double b = 0,
                              std::int32_t id = 0) = 0;
    virtual std::int32_t nearby_count_404d30() = 0;
    virtual std::int32_t nearby_id_404d10(std::int32_t index) = 0;
    virtual BounceNearbyActor* find_actor_444ba1(std::int32_t id) = 0;
    // Allocation plus the named constructor. Null is passed through to attach.
    virtual void* create_state(BounceStateKind kind) = 0;
    virtual void attach_state_42e696(BouncingActor& actor, void* state) = 0;
    virtual void select_landed_42f8b9(BouncingActor& actor) = 0;
    virtual std::uint8_t wall_contact_42eb48(BouncingActor& actor) = 0;
    virtual std::uint8_t has_state_42ecc9(BouncingActor& actor, std::int32_t type) = 0;
    virtual void remove_state_42e7f3(BouncingActor& actor, std::int32_t type) = 0;
};

// 0x40F700 fills the 50 roots. The caller supplies the initialized globals;
// the original uses apex_time when ordering each pair of quadratic roots.
void initialize_bounce_times(BounceTimeTable& table);
// 0x40F55D and 0x40F5F8. The latter preserves next_height if no interval matches.
long double actor_bounce_time_to_x(const ActorBouncingState& state, const BouncingActor& actor);
long double actor_bounce_time_to_y(const ActorBouncingState& state, const BounceTimeTable& table,
                                  std::int32_t& next_height);
// 0x40ECCA, 0x40EE9A, 0x40F1E9, and 0x40EEC4 respectively.
void enter_actor_bouncing(ActorBouncingState& state, BouncingActor& actor,
                          const CollisionGridHead& grid, ActorBouncingCallbacks& callbacks);
void leave_actor_bouncing(BouncingActor& actor, ActorBouncingCallbacks& callbacks);
std::int32_t resolve_actor_bounce_collision(const ActorBouncingState& state, BouncingActor& actor,
    std::uint8_t horizontal_boundary, CollisionGridHead& grid, const BounceTimeTable& table,
    ActorBouncingCallbacks& callbacks);
// Valid handles, positive grid height, and an initialized table are required.
// Like the original, the update requires a valid time-table interval whenever
// it takes a vertical substep, and inputs for which its loop terminates.
std::int32_t update_actor_bouncing(ActorBouncingState& state, BouncingActor& actor,
    double& remaining, CollisionGridHead& grid, const BounceTimeTable& table,
    ActorBouncingCallbacks& callbacks);

} // namespace lemmings
