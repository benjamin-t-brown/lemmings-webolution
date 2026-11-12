#include "lemmings/actor_spawn_monitor.hpp"
#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

namespace {
using namespace lemmings;
struct Callbacks : ActorSpawnMonitorCallbacks {
    double* remaining = nullptr;
    bool continuation = true, present = false;
    int regions = 3, matching_region = -1;
    std::vector<int> trace;
    ActorBehavior behavior;
    SpawnMonitorRequest request;
    std::uint8_t can_continue_42f6ec(SpawnMonitorActor&) override {
        assert(*remaining == 0 && !std::signbit(*remaining)); trace.push_back(1); return continuation;
    }
    std::int32_t region_count_4157f0() override { trace.push_back(2); return regions; }
    std::uint8_t region_contains_420a52(std::int32_t index, std::array<double, 3> point) override {
        assert((point == std::array<double, 3>{2,4,0})); trace.push_back(10 + index); return index == matching_region;
    }
    std::uint8_t has_state_42ecc9(SpawnMonitorActor&, std::int32_t type) override { trace.push_back(type); return present; }
    void clear_primary_42e725(SpawnMonitorActor&) override { trace.push_back(3); }
    ActorBehavior* create_state(SpawnMonitorTransition kind) override {
        trace.push_back(kind == SpawnMonitorTransition::state_406c20 ? 4 : 5); return &behavior;
    }
    void attach_state_42e696(SpawnMonitorActor&, ActorBehavior* value) override { assert(value == &behavior); trace.push_back(6); }
    SpawnMonitorRequest* create_request_415710(void* context) override { assert(context == this); trace.push_back(7); return &request; }
    void enqueue_request_43f0c0(SpawnMonitorRequest& value) override {
        assert((value.pair_10 == std::array<double, 2>{2,4})); trace.push_back(8);
        value.pair_10 = {99,99}; // Actor cache is copied from the local pair, not this request.
    }
};
}
int main() {
    using namespace lemmings;
    ActorSpawnMonitorState state; state.unknown_pair_18 = {0x1234, 0x5678};
    ActorBehavior behavior; initialize_actor_spawn_monitor(state, behavior);
    assert(state.elapsed_08 == 0 && behavior.phase_10 == 1 && behavior.extra_11 == 1 && behavior.type_14 == 0x2f);
    assert((state.unknown_pair_18 == std::array<std::uint64_t,2>{0x1234,0x5678}));
    SpawnMonitorActor actor; actor.geometry.position = {2.9, 5.9, 0.25};
    actor.geometry.pair_360 = {0,0}; actor.geometry.direction_44 = 1;
    assert((actor_monitor_sample_position(actor.geometry) == std::array<double,3>{2,5,0.25}));
    const double nan = std::numeric_limits<double>::quiet_NaN();
    assert(!monitor_pairs_differ({nan,4}, {2,4}));
    assert(!monitor_pairs_differ({2,nan}, {2,4}));
    assert(monitor_pairs_differ({nan,5}, {2,4}));
    CollisionGridHead grid{}; grid.height = 8;
    std::vector<std::int32_t> cells(256 * grid.height); grid.cells.data = cells.data();
    auto& cell = cells[4*256+2];
    double remaining = 1; Callbacks callbacks; callbacks.remaining = &remaining;
    cell = 0x140; callbacks.matching_region = 1;
    assert(update_actor_spawn_monitor(actor,remaining,grid,&callbacks,callbacks) == 0);
    assert((callbacks.trace == std::vector<int>{1,2,10,2,11,0x2e,4,6}));
    assert(actor.flag_5f == 0); // 0x40 takes precedence over 0x100.
    callbacks.trace.clear(); callbacks.matching_region = -1; remaining = 2;
    update_actor_spawn_monitor(actor,remaining,grid,&callbacks,callbacks);
    assert((callbacks.trace == std::vector<int>{1,2,10,2,11,2,12,2,0x22,3,5,6}));
    callbacks.trace.clear(); callbacks.present = true;
    update_actor_spawn_monitor(actor,remaining,grid,&callbacks,callbacks);
    assert(callbacks.trace.back() == 0x22);
    cell = 0x100; callbacks.trace.clear();
    update_actor_spawn_monitor(actor,remaining,grid,&callbacks,callbacks);
    assert((callbacks.trace == std::vector<int>{1,7,8}));
    assert((actor.last_pair_68 == std::array<double,2>{2,4}) && actor.flag_5f == 1);
    callbacks.trace.clear(); update_actor_spawn_monitor(actor,remaining,grid,&callbacks,callbacks);
    assert((callbacks.trace == std::vector<int>{1}));
    callbacks.continuation = false; callbacks.trace.clear();
    assert(update_actor_spawn_monitor(actor,remaining,grid,&callbacks,callbacks) == 2);
    assert((callbacks.trace == std::vector<int>{1}));
    SpawnMonitorRequest request{{2.9,4.9}}; bool notified = false;
    const SpawnMonitorClearHooks hooks{&notified, [](void* context, int x1, int y1, int x2, int y2) {
        assert(x1 == 2 && y1 == 4 && x2 == 2 && y2 == 4); *static_cast<bool*>(context) = true;
    }};
    assert(clear_spawn_monitor_request(request,grid,hooks) == 0 && notified && cell == 0);
    enter_actor_spawn_monitor(); leave_actor_spawn_monitor();
}
