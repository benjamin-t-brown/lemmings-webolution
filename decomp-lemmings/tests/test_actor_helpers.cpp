#include "actor_clear_fixture.hpp"
#include "lemmings/actor_motion.hpp"

#include <bit>
#include <limits>
#include <source_location>
#include <stdexcept>

namespace {
void check(bool condition, std::source_location where = std::source_location::current()) {
    if (!condition) throw std::runtime_error("actor helpers mismatch at line " + std::to_string(where.line()));
}
}
int main() {
    lemmings::ActorGeometryState actor;
    lemmings::CollisionGridHead grid{};
    grid.height = 96;
    actor.position = {3, 4, 5};
    actor.pair_360 = {1, 3};
    lemmings::move_actor_sample_x(actor, 10);
    check(actor.position == std::array<double, 3>{9, 4, 5});
    lemmings::move_actor_sample_x(actor, -1);
    check(actor.position[0] == 254);
    lemmings::move_actor_sample_x(actor, 257);
    check(actor.position[0] == 0);
    actor.position[0] = 3;
    lemmings::move_actor_sample_y(actor, 10, grid);
    check(actor.position == std::array<double, 3>{3, 7, 5});
    lemmings::move_actor_sample_y(actor, -1, grid);
    check(std::bit_cast<std::uint64_t>(actor.position[1]) == 0);
    lemmings::move_actor_sample_y(actor, 1000, grid);
    check(actor.position[1] == 95);
    actor.pair_360[1] = std::numeric_limits<double>::quiet_NaN();
    lemmings::move_actor_sample_y(actor, 10, grid);
    check(std::bit_cast<std::uint64_t>(actor.position[1]) == 0);
    for (std::size_t count = 0; count <= 16; ++count) {
        for (unsigned mode = 0; mode <= 4; ++mode) {
            actor_clear_test::Fixture f;
            f.initialize(count, mode);
            f.run();
            check(f.reset_correctly());
            for (std::size_t i = 0; i < count; ++i) {
                check(f.trace[i][0] == 1 && f.trace[i][1] == i);
            }
            for (std::size_t i = count; i < f.trace.size(); ++i) {
                check(f.trace[i][0] != 1);
                if (f.trace[i][0] == 2) check(f.trace[i][2] == 1);
            }
            if (mode == 0) check(f.trace.size() == 3 * count);
            if (mode == 1) check(f.trace.size() == 2 * count); // Leave cleared payloads.
            if (mode == 2 && count != 0) check(f.trace[count][1] == count); // New head destroyed first.
            if (mode == 3 && count >= 3) check(f.trace.size() == 3 * count - 2);
            if (mode == 4) check(f.trace.size() == 3 * count); // Cached next survives node release.
        }
    }
    for (unsigned mode=0;mode<8;++mode) {
        actor_clear_test::Fixture f;f.initialize_removal(3,mode);
        lemmings::clear_actor_primary_behavior(f.actor);
        check(f.actor.primary==nullptr && f.trace[0][0]==1 && f.trace[0][1]==17);
        check(f.trace.size()==(mode==6?1:2));
        if(mode!=6)check(f.trace[1][0]==2 && f.trace[1][1]==(mode==1?16:17) && f.trace[1][2]==1);
    }
    for (std::size_t count=0;count<=16;++count) for (unsigned mode=0;mode<8;++mode) for (int type=-1;type<=4;++type) {
        actor_clear_test::Fixture f;f.initialize_removal(count,mode);
        auto* found=lemmings::find_actor_behavior(f.actor,type);
        if (type==3) check(found==f.actor.primary);
        if (type>=0 && type<3 && static_cast<std::size_t>(type)<count) check(found==&f.states[type]);
        check(f.remove(type)==(found!=nullptr));
        if (found==nullptr) check(f.trace.empty());
        else {
            check(f.trace.size()>=2 && f.trace[0][0]==1 && f.trace[1][0]==2 && f.trace[1][2]==1);
            if (type==3 && mode!=1 && mode!=6) check(f.actor.primary==nullptr);
            if (f.trace.size()==3) check(f.trace[2][0]==3);
        }
    }
}
