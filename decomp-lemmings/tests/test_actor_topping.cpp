#include "lemmings/actor_topping.hpp"
#include <cassert>
#include <bit>
#include <cmath>
#include <limits>
#include <vector>
using namespace lemmings;
namespace {
struct Effects : ActorToppingCallbacks {
    std::vector<int> events;
    std::uint8_t finished=0;
    double* remaining=nullptr;
    void select_animation_42fd25(ToppingActor&, std::string_view name) override { assert(name=="flip");events.push_back(1); }
    void advance_model_2c(ToppingActor&, double dt) override { assert(dt==0.5);events.push_back(2); }
    std::uint8_t finished_model_44(ToppingActor&) override { events.push_back(3);return finished; }
    long double model_time_48(ToppingActor& a) override { assert(a.geometry.position[0]==0 && a.geometry.position[1]==9);events.push_back(4);return 0.125L; }
    void select_landed_42f8b9(ToppingActor&) override { assert(*remaining==0.125);events.push_back(5); }
};
}
int main() {
    ToppingActor actor; actor.geometry.position={255.75,7.5,0.25};
    CollisionGridHead grid{};grid.height=10;
    Effects cb;enter_actor_topping(actor,cb);double time=0.5;cb.remaining=&time;
    assert(update_actor_topping(actor,time,grid,cb)==0 && time==0 && !std::signbit(time));
    assert((actor.geometry.position==std::array<double,3>{255.75,7.5,0.25}));
    time=0.5;cb.finished=0x80;
    assert(update_actor_topping(actor,time,grid,cb)==1 && time==0.125);
    assert((cb.events==std::vector{1,2,3,2,3,4,5}));
    leave_actor_topping(actor);
    assert((actor.geometry.position==std::array<double,3>{-1,8,0.25}));
    actor.geometry.position={0,0,0};actor.geometry.direction_40=-1;actor.geometry.direction_44=-1;
    leave_actor_topping(actor);assert(actor.geometry.position[0]==1 && actor.geometry.position[1]==1);
}
