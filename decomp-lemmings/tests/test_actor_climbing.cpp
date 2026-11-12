#include "lemmings/actor_climbing.hpp"
#include <cassert>
#include <bit>
#include <cmath>
#include <limits>
#include <vector>
using namespace lemmings;
int main() {
    std::array<std::int32_t,8*256> cells{};
    CollisionGridHead grid{};grid.height=8;grid.cells.data=cells.data();
    ClimbingActor actor{};actor.geometry.position={3,2.5,0};
    ActorClimbingState state{};
    std::vector<int> trace;
    ActorBehavior created{};
    struct Context { std::vector<int>& trace; ActorBehavior& created; ActorClimbingState& state; } context{trace,created,state};
    ActorClimbingHooks hooks{};hooks.context=&context;
    hooks.set_animation=[](void* p,ClimbingActor&,std::string_view name){assert(name=="climb");static_cast<Context*>(p)->trace.push_back(1);};
    hooks.advance_animation=[](void* p,ClimbingActor& actor,double step){assert(std::bit_cast<std::uint64_t>(step)==0x3FB999999999999AULL);assert(actor.geometry.position[1]==3);static_cast<Context*>(p)->trace.push_back(2);};
    hooks.notice=[](void* p,ClimbingActor&,ClimbingNotice notice){static_cast<Context*>(p)->trace.push_back(10+static_cast<int>(notice));};
    hooks.create_state=[](void* p,ClimbingTransition type){auto& c=*static_cast<Context*>(p);c.trace.push_back(20+static_cast<int>(type));return &c.created;};
    hooks.attach_state=[](void* p,ClimbingActor&,ActorBehavior*){auto& c=*static_cast<Context*>(p);c.trace.push_back(30);c.state.reached_boundary_18=99;};
    enter_actor_climbing(actor,hooks);
    double time=0.1;
    assert(update_actor_climbing(state,actor,time,grid,hooks)==1);
    assert(time==0 && state.reached_boundary_18==1);
    assert((trace==std::vector<int>{1,2,10,12,13,21,30,14}));
    cells.fill(1);trace.clear();actor.geometry.direction_40=1;
    assert(resolve_actor_climbing_collision(actor,grid,hooks)==1);
    assert(actor.geometry.position[0]==2 && actor.geometry.direction_40==-1);
    assert(actor.geometry.pair_350[0]==-1 && actor.geometry.pair_360[0]==-1);
    assert((trace==std::vector<int>{11,20,30}));
    cells.fill(0);cells[2*256+3]=1;
    actor.geometry.position={3,3,0};actor.geometry.direction_40=1;
    actor.geometry.pair_350={0,0};trace.clear();
    assert(resolve_actor_climbing_collision(actor,grid,hooks)==0 && trace.empty());
    actor.geometry.direction_44=-1;state.reached_boundary_18=0;
    assert(climbing_boundary_time(state,actor)==0);
    state.reached_boundary_18=1;assert(std::bit_cast<std::uint64_t>(climbing_boundary_time(state,actor))==0x3FC999999999999AULL);
    actor.geometry.position[1]=std::numeric_limits<double>::quiet_NaN();
    assert(std::bit_cast<std::uint64_t>(climbing_boundary_time(state,actor))==0x3FC999999999999AULL);
    time=std::numeric_limits<double>::quiet_NaN();
    assert(update_actor_climbing(state,actor,time,grid,hooks)==0 && time==0 && !std::signbit(time));
}
