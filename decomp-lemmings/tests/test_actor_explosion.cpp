#include "actor_explosion_fixture.hpp"
#include <cassert>
int main(){
    lemmings::ActorTerminalState s{99};lemmings::ActorBehavior b;lemmings::initialize_actor_explosion(s,b);
    assert(s.elapsed_08==0&&b.phase_10==0&&b.extra_11==0&&b.type_14==2);
    for(unsigned stage=0;stage<=18;++stage){
        lemmings::ExplosionActor a;a.position_08={2.5,3.5,.5};a.pair_360={.25,.75};double dt=.5;
        explosion_test::Trace t;t.position=&a.position_08;t.category=&a.category_4c;t.remaining=&dt;t.mutation=stage;
        lemmings::enter_actor_explosion(a,71,t);assert(lemmings::update_actor_explosion(a,dt,t)==2);
        assert(std::bit_cast<std::uint64_t>(dt)==0);assert(t.events.back()==18);lemmings::leave_actor_explosion();
    }
    // Clearing remaining happens after insertion, including when the reference
    // aliases actor position, sample offset, or the behavior's elapsed.
    for(unsigned storage=0;storage!=6;++storage)for(unsigned stage=0;stage<=18;++stage){
        lemmings::ExplosionActor a;a.position_08={2.5,3.5,.5};a.pair_360={.25,.75};
        lemmings::ActorTerminalState state{.5};
        double& remaining=storage<3?a.position_08[storage]:storage<5?a.pair_360[storage-3]:state.elapsed_08;
        explosion_test::Trace t;t.position=&a.position_08;t.category=&a.category_4c;t.remaining=&remaining;t.mutation=stage;
        t.child_total=storage==0?-1:6;t.null_entity=storage==1;
        assert(lemmings::update_actor_explosion(a,remaining,t)==2);
        assert(std::bit_cast<std::uint64_t>(remaining)==0&&t.events.back()==18);
    }
}
