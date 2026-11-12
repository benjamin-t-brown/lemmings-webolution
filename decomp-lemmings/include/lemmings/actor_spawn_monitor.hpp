#pragma once

#include "lemmings/actor_geometry.hpp"
#include "lemmings/actor_update.hpp"

namespace lemmings {

struct ActorSpawnMonitorState {
    double elapsed_08 = 0;
    std::array<std::uint64_t, 2> unknown_pair_18{}; // Empty constructor; preserved.
};
struct SpawnMonitorActor {
    ActorGeometryState geometry;
    std::uint8_t flag_5f = 0;
    std::array<double, 2> last_pair_68{};
};
struct SpawnMonitorRequest {
    std::array<double, 2> pair_10{};
};
enum class SpawnMonitorTransition { state_406c20, state_408700 };
struct ActorSpawnMonitorCallbacks {
    virtual ~ActorSpawnMonitorCallbacks() = default;
    virtual std::uint8_t can_continue_42f6ec(SpawnMonitorActor&) = 0;
    virtual std::int32_t region_count_4157f0() = 0;
    virtual std::uint8_t region_contains_420a52(std::int32_t index, std::array<double, 3> point) = 0;
    virtual std::uint8_t has_state_42ecc9(SpawnMonitorActor&, std::int32_t type) = 0;
    virtual void clear_primary_42e725(SpawnMonitorActor&) = 0;
    virtual ActorBehavior* create_state(SpawnMonitorTransition) = 0;
    virtual void attach_state_42e696(SpawnMonitorActor&, ActorBehavior*) = 0;
    // Allocation + 0x415710, with original global 0x4CBF88 +0x190 passed
    // explicitly. The original dereferences the returned request immediately.
    virtual SpawnMonitorRequest* create_request_415710(void* context) = 0;
    virtual void enqueue_request_43f0c0(SpawnMonitorRequest&) = 0;
};

// 0x4151F0: elapsed=0, phase=1, extra=1, type=0x2F; pair +18 untouched.
void initialize_actor_spawn_monitor(ActorSpawnMonitorState&, ActorBehavior&);
void enter_actor_spawn_monitor(); // 0x409310, empty
void leave_actor_spawn_monitor(); // 0x401320, empty
// 0x42F183: truncate actor X/Y through signed64's low32 before adding pair360.
std::array<double, 3> actor_monitor_sample_position(const ActorGeometryState&);
// 0x415800: ordered unequal component => true; unordered compares as equal.
bool monitor_pairs_differ(const std::array<double, 2>&, const std::array<double, 2>&);
// 0x41527B. Geometry is sampled before can_continue; remaining becomes +0
// before that callback. Terrain bit0x40 takes precedence over bit0x100.
// Requires a positive-height grid with matching storage and valid callbacks.
std::uint32_t update_actor_spawn_monitor(SpawnMonitorActor&, double& remaining,
    CollisionGridHead&, void* request_context, ActorSpawnMonitorCallbacks&);

struct SpawnMonitorClearHooks {
    void* context = nullptr;
    void (*notify_43b0a1)(void*, std::int32_t x1, std::int32_t y1,
                        std::int32_t x2, std::int32_t y2) = nullptr;
};
// 0x41565C deferred update: set the sampled cell to zero, notify its bounds,
// then return zero. Grid wrapping/clamping is inherited from collision_cell.
std::uint32_t clear_spawn_monitor_request(const SpawnMonitorRequest&,
    CollisionGridHead&, const SpawnMonitorClearHooks&);

} // namespace lemmings
