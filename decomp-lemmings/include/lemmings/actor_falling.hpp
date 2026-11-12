#pragma once

#include "lemmings/actor_geometry.hpp"

#include <string_view>

namespace lemmings {

// Logical views; suffixes are original offsets, not host structure offsets.
struct ActorFallingState {
    double distance_18 = 0;
    std::uint8_t umbrella_open_20 = 0;
    std::uint8_t last_cell_21 = 0;
};

struct FallingActor {
    ActorGeometryState geometry;
    std::uint8_t umbrella_49 = 0;
    std::uint8_t floating_4b = 0;
    void* model_78 = nullptr; // Handle to the original inline model at +0x78.
};

struct ActorFallingCallbacks {
    virtual ~ActorFallingCallbacks() = default;
    virtual std::uint8_t has_state_42ecc9(FallingActor& actor, std::int32_t id) = 0;
    virtual void select_animation_42fd25(FallingActor& actor, std::string_view name) = 0;
    // x87 return values: the duration is stored to double before elapsed is
    // queried, but elapsed remains extended during the subsequent subtraction.
    virtual long double model_duration_4c(void* model) = 0;
    virtual long double model_elapsed_48(void* model) = 0;
    virtual void log_umbrella_time_40f860(double time) = 0;
    virtual long double height_43da38(std::array<double, 3>& position, double next_y) = 0;
    virtual void advance_model_2c(void* model, double elapsed) = 0;
    virtual std::int32_t landing_416433(ActorFallingState& state, FallingActor& actor) = 0;
};

// 0x00416004. Queries state 0x2B, conditionally selects "fall", then clears +4B.
void enter_actor_falling(FallingActor& actor, ActorFallingCallbacks& callbacks);

// 0x00416038: speed 20 normally or 10 when actor +4B is nonzero. The original
// result remains extended precision until its caller chooses to store it.
long double actor_fall_distance(const FallingActor& actor, double elapsed);

// 0x00416367, including low-32-bit projection of x87 FISTP signed64 conversion,
// and the zero-or-unordered boundary retry when state +21 is nonzero.
long double actor_fall_time_to_cell(const ActorFallingState& state,
                                    const FallingActor& actor);

// 0x004160BD. Substeps to cell boundaries and umbrella animation completion.
// Nonzero landing result preserves the remaining time; result zero clears it.
// Requires valid callback/model handles, a valid grid, masked FP exceptions,
// and inputs/callbacks for which the original substep loop terminates.
std::int32_t update_actor_falling(ActorFallingState& state, FallingActor& actor,
                                  double& remaining_time, const CollisionGridHead& grid,
                                  ActorFallingCallbacks& callbacks);

} // namespace lemmings
