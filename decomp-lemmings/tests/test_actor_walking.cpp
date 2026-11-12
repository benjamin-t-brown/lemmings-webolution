#include "lemmings/actor_walking.hpp"
#include <cassert>
#include <bit>
#include <cmath>
#include <limits>
#include <vector>
using namespace lemmings;
int main() {
    WalkingActor actor{};
    ActorWalkingState state{};
    std::vector<int> trace;
    ActorWalkingHooks hooks{};
    hooks.context = &trace;
    hooks.set_animation = [](void* p, WalkingActor&, std::string_view name) {
        assert(name == "walk"); static_cast<std::vector<int>*>(p)->push_back(1);
    };
    hooks.advance_animation = [](void* p, WalkingActor& a, double dt) {
        assert(dt == 0.2); assert(a.geometry.position[0] == 1);
        static_cast<std::vector<int>*>(p)->push_back(2);
    };
    hooks.collision = [](void* p, ActorWalkingState& s, WalkingActor&) {
        s.reached_boundary_18 = 99;
        static_cast<std::vector<int>*>(p)->push_back(3); return 7;
    };
    hooks.surface = [](void* p, ActorWalkingState& s, WalkingActor& a) {
        assert(s.reached_boundary_18 == 1);
        a.gadget_54 = reinterpret_cast<void*>(1);
        static_cast<std::vector<int>*>(p)->push_back(4);
    };
    hooks.gadget_contact = [](void* p, void* gadget, WalkingActor&) {
        assert(gadget == reinterpret_cast<void*>(1));
        static_cast<std::vector<int>*>(p)->push_back(5); return false;
    };
    hooks.finish = [](void* p, WalkingActor&) { static_cast<std::vector<int>*>(p)->push_back(6); };
    enter_actor_walking(actor, hooks);
    double time = 0.4;
    actor.mode_50 = 3;
    assert(update_actor_walking(state, actor, time, hooks) == 7);
    assert(time == 0 && actor.gadget_54 == nullptr && state.reached_boundary_18 == 1);
    assert((trace == std::vector<int>{1,2,3,4,5,6}));
    actor.geometry.direction_40 = -1;
    assert(walking_boundary_time(state, actor) == 0.2);
    state.reached_boundary_18 = 0;
    assert(walking_boundary_time(state, actor) == 0);
    actor.geometry.position[0] = std::numeric_limits<double>::quiet_NaN();
    assert(std::isnan(walking_boundary_time(state, actor)));
    state.reached_boundary_18 = 1;
    assert(walking_boundary_time(state, actor) == 0.2);
    for (double initial : {-1.0, -0.0, std::numeric_limits<double>::quiet_NaN()}) {
        trace.clear(); time = initial;
        assert(update_actor_walking(state, actor, time, hooks) == 0);
        assert(time == 0 && !std::signbit(time));
        assert((trace == std::vector<int>{6}));
    }
    actor.depth_adjust_5e = 1;
    actor.geometry.direction_40 = -1;
    actor.geometry.position[2] = -0.01;
    approach_actor_walking_depth(actor, 0, 2);
    assert(actor.geometry.position[2] == 0);
    actor.geometry.direction_40 = 2;
    actor.geometry.position[2] = 0.01;
    approach_actor_walking_depth(actor, 2, 0);
    assert(actor.geometry.position[2] == 0);
    actor.geometry.position[2] = -std::bit_cast<double>(std::uint64_t{0x3F947AE147AE147B});
    approach_actor_walking_depth(actor, 0, 2);
    assert(actor.geometry.position[2] == 0);
    actor.geometry.position[2] = 1;
    approach_actor_walking_depth(actor, 0, std::numeric_limits<double>::quiet_NaN());
    assert(actor.geometry.position[2] == 1.02);
    std::array<std::int32_t, 8*256> cells{};
    CollisionGridHead grid{}; grid.height = 8; grid.cells.data = cells.data();
    ActorWalkingSurfaceHooks surface{};
    struct SurfaceTest { unsigned calls=0; } context;
    surface.context = &context;
    surface.gadget_count = [](void*) { return 2; };
    surface.gadget_at = [](void*, int index) { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(index+1)); };
    surface.gadget_contains = [](void* p, void* gadget, const std::array<double,3>& point) -> std::uint8_t {
        ++static_cast<SurfaceTest*>(p)->calls;
        assert(point[0] == 3.5);
        return point[1] == 3.5 && gadget == reinterpret_cast<void*>(2) ? 0x80 : 0;
    };
    actor.geometry.position = {2.5,3.5,0};
    actor.geometry.direction_40 = actor.geometry.direction_44 = 1;
    actor.gadget_54 = nullptr;
    actor.gadget_58 = reinterpret_cast<void*>(99);
    actor.special_4a = 1;
    cells[3*256+3] = 0x2000;
    track_actor_walking_surface(actor, grid, surface);
    assert(context.calls == 4 && actor.special_4a == 0);
    assert(actor.gadget_54 == reinterpret_cast<void*>(2) && actor.gadget_58 == actor.gadget_54);
    context.calls = 0; cells.fill(0);
    track_actor_walking_surface(actor, grid, surface);
    assert(context.calls == 0 && actor.gadget_54 == nullptr && actor.gadget_58 == reinterpret_cast<void*>(2));
}
