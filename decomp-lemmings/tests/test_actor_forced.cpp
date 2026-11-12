#include "actor_forced_fixture.hpp"
#include <cassert>
int main(){
    lemmings::ActorTerminalState s{99};lemmings::ActorBehavior b;
    lemmings::initialize_actor_forced(s,b);
    assert(s.elapsed_08==0&&b.phase_10==0&&b.extra_11==0&&b.type_14==0x25);
    for(unsigned m=0;m<=15;++m){
        forced_test::Trace c;lemmings::TerminalActor a;a.position_08={2,3,4};a.category_4c=7;
        double dt=.5;c.position=&a.position_08;c.category=&a.category_4c;c.remaining=&dt;c.mutate=m;
        lemmings::enter_actor_forced();assert(lemmings::update_actor_forced(a,dt,c)==2);
        assert(dt==(m?-.125:.5));assert(c.random_index==2);assert(c.events.back()==15);
        // Offset captured before all callbacks: effect entity ends at X=2,Y=3.75,Z=6.25.
        auto e=c.events.size();assert(c.events[e-7]==std::bit_cast<std::uint64_t>(2.0));
        assert(c.events[e-6]==std::bit_cast<std::uint64_t>(3.75));
        assert(c.events[e-5]==std::bit_cast<std::uint64_t>(6.25));
        lemmings::leave_actor_forced();
    }
}
