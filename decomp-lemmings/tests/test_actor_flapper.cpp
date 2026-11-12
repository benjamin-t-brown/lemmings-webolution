#include "actor_flapper_fixture.hpp"
#include <cassert>
using namespace flapper_test;
int main() {
    Fixture f; ActorBehavior behavior;
    ActorBehaviorOperations operations{};
    behavior.operations = &operations; behavior.context = &f;
    behavior.phase_10 = behavior.extra_11 = 0xff; f.state.elapsed_08 = 123;
    initialize_actor_flapper(f.state, behavior);
    assert(f.state.elapsed_08 == 0 && behavior.phase_10 == 0 && behavior.extra_11 == 0 && behavior.type_14 == 0x20);
    assert(behavior.operations == &operations && behavior.context == &f);
    enter_actor_flapper(f.actor, f.global, f);
    assert(f.trace.size() == 2 && f.trace[0].kind == 1 && f.trace[1].kind == 2);
    f.trace.clear(); f.script = 1; enter_actor_flapper(f.actor, f.global, f);
    assert(f.trace.size() == 1); f.script = 0;
    assert(update_actor_flapper(f.state, f.actor, f.remaining, f.grid, f.global, f) == 0);
    assert(f.actor.geometry.position[1] == 6 && f.remaining == 0 && f.state.elapsed_08 == 0.125);
    f.remaining = 0.125; f.cells.fill(0x80); f.finished = 0x80; f.fail = true;
    f.script = 64;
    assert(update_actor_flapper(f.state, f.actor, f.remaining, f.grid, f.global, f) == 1);
    assert(f.actor.geometry.position[1] == 6 && f.remaining == 0.125 && f.valid);
    const auto trace = f.trace; leave_actor_flapper(); assert(trace == f.trace);
}
