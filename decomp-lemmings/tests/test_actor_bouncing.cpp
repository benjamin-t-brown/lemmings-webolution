#include "lemmings/actor_bouncing.hpp"
#include <bit>
#include <cassert>
#include <cmath>
#include <limits>
#include <vector>
using namespace lemmings;
namespace {
struct Effects : ActorBouncingCallbacks {
    std::vector<BounceNotice> notices;
    std::vector<BounceStateKind> created;
    std::array<BounceNearbyActor, 3> nearby{};
    std::vector<int> ids;
    std::uint8_t wall = 0, has = 0;
    unsigned attached = 0, landed = 0, removed = 0, advances = 0;
    void advance_model_2c(void*, double) override { ++advances; }
    std::int16_t actor_id_406a00(BouncingActor&) override { return -9; }
    void select_animation_42fd25(BouncingActor&, std::string_view name) override { assert(name == "fall"); }
    void notice_40f860(BounceNotice n, double = 0, double = 0, std::int32_t = 0) override { notices.push_back(n); }
    std::int32_t nearby_count_404d30() override { return static_cast<std::int32_t>(ids.size()); }
    std::int32_t nearby_id_404d10(std::int32_t i) override { return ids.at(i); }
    BounceNearbyActor* find_actor_444ba1(std::int32_t id) override { return id < 0 ? nullptr : &nearby.at(id); }
    void* create_state(BounceStateKind k) override { created.push_back(k); return nullptr; }
    void attach_state_42e696(BouncingActor&, void* p) override { assert(p == nullptr); ++attached; }
    void select_landed_42f8b9(BouncingActor&) override { ++landed; }
    std::uint8_t wall_contact_42eb48(BouncingActor&) override { return wall; }
    std::uint8_t has_state_42ecc9(BouncingActor&, std::int32_t type) override { assert(type == 0x2d); return has; }
    void remove_state_42e7f3(BouncingActor&, std::int32_t type) override { assert(type == 0x2d); ++removed; }
};
}
int main() {
    BounceTimeTable table; initialize_bounce_times(table);
    // Compare stored binary64 values explicitly: unoptimized i386 can retain
    // the decimal literal in x87 precision during an ordinary comparison.
    assert(table.times[0] == 0 && std::bit_cast<std::uint64_t>(table.times[49]) == 0x4006666666666666ULL);
    for (unsigned i = 1; i < 50; ++i) assert(table.times[i] > table.times[i-1]);
    ActorBouncingState state; BouncingActor actor; actor.geometry.position = {4,3,0.5};
    assert(std::bit_cast<std::uint64_t>(static_cast<double>(actor_bounce_time_to_x(state, actor))) == 0x3fc999999999999aULL);
    actor.geometry.direction_40 = -1;
    assert(actor_bounce_time_to_x(state, actor) == 0);
    state.reached_boundary_28 = 1;
    assert(std::bit_cast<std::uint64_t>(static_cast<double>(actor_bounce_time_to_x(state, actor))) == 0x3fc999999999999aULL);
    actor.geometry.position[0] = std::numeric_limits<double>::quiet_NaN();
    assert(std::bit_cast<std::uint64_t>(static_cast<double>(actor_bounce_time_to_x(state, actor))) == 0x3fc999999999999aULL);
    int next = 99; state.elapsed_08 = table.times[24];
    assert(actor_bounce_time_to_y(state, table, next) > 0 && next == 24);
    state.elapsed_08 = table.times[49]; next = 99;
    assert(actor_bounce_time_to_y(state, table, next) == 0 && next == 99);
    auto unordered_table = table;
    unordered_table.times[1] = std::numeric_limits<double>::quiet_NaN();
    state.elapsed_08 = 0; state.reached_boundary_28 = 0;
    assert(std::isnan(actor_bounce_time_to_y(state, unordered_table, next)) && next == 1);
    state.reached_boundary_28 = 1;
    assert(std::isnan(actor_bounce_time_to_y(state, unordered_table, next)) && next == 2);
    std::array<std::int32_t, 8*256> cells{}; CollisionGridHead grid{};
    grid.height = 8; grid.cells.data = cells.data();
    actor.geometry.position = {4,3,0.5}; actor.geometry.direction_44 = 1;
    Effects cb; cb.ids = {-1,0,1,2};
    cb.nearby[0] = {{5,1,0},0x0e,0}; cb.nearby[1] = {{3,1,0},0x0e,0};
    cb.nearby[2] = {{100,7,0},0x0e,0};
    enter_actor_bouncing(state, actor, grid, cb);
    assert(state.origin_y_18 == 2 && state.origin_x_20 == 4);
    assert(cb.nearby[0].primary_flag_18 == 1 && cb.nearby[1].primary_flag_18 == 0);
    cells[2*256+4] = 0x1000; state.elapsed_08 = 2;
    // The generic floor branch wins over trampoline when bit 0x80 is absent.
    assert(resolve_actor_bounce_collision(state, actor, 0, grid, table, cb) == 1 && cb.landed == 1 && cb.created.empty());
    cells[2*256+4] = 0x1080;
    assert(resolve_actor_bounce_collision(state, actor, 0, grid, table, cb) == 1 && cb.created.back() == BounceStateKind::bounce_40ec7e);
    cells.fill(0); cells[3*256+4] = 1;
    assert(resolve_actor_bounce_collision(state, actor, 0, grid, table, cb) == 1 && cb.created.back() == BounceStateKind::falling_415fa0);
    cells.fill(0); state.elapsed_08 = table.times[49];
    assert(resolve_actor_bounce_collision(state, actor, 0, grid, table, cb) == 1);
    cb.wall = 0x80;
    assert(resolve_actor_bounce_collision(state, actor, 9, grid, table, cb) == 0 && actor.geometry.direction_40 == 1);
    leave_actor_bouncing(actor, cb); assert(cb.removed == 0);
    cb.has = 0x80; leave_actor_bouncing(actor, cb); assert(cb.removed == 1);
    double time = std::numeric_limits<double>::quiet_NaN(); actor.effect_60 = 0;
    assert(update_actor_bouncing(state, actor, time, grid, table, cb) == 0 && time == 0 && state.reached_boundary_28 == 0);
    assert(cb.advances == 1 && cb.created.back() == BounceStateKind::effect_40e3c0);
}
