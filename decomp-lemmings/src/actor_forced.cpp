#include "lemmings/actor_forced.hpp"
#include <bit>
#include <cmath>

namespace lemmings {
namespace {
double rounded(long double v) { volatile double out=static_cast<double>(v);return out; }
std::uint32_t ftol_low(double v) {
    if (!std::isfinite(v) || static_cast<long double>(v)<-9223372036854775808.0L ||
        static_cast<long double>(v)>=9223372036854775808.0L) return 0;
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(v));
}
}
void initialize_actor_forced(ActorTerminalState& s, ActorBehavior& b) {
    s.elapsed_08=0; b.phase_10=0; b.extra_11=0; b.type_14=0x25;
}
void enter_actor_forced() {}
void leave_actor_forced() {}
std::uint32_t update_actor_forced(TerminalActor& actor, double& remaining, ActorForcedCallbacks& c) {
    auto offset=actor.position_08;
    offset[2]=rounded(c.random_404080()+2.0L+static_cast<long double>(offset[2]));
    offset[1]=rounded(c.random_404080()+1.5L+static_cast<long double>(offset[1]));
    auto* effect=c.create_effect_416ed0();
    c.set_category_d8(effect,actor.category_4c);
    c.set_count_417150(effect,1);
    const auto original=c.world_position_43d56c(actor.position_08);
    const auto shifted=c.world_position_43d56c(offset);
    std::array<double,3> direction;
    for(unsigned i=0;i<3;++i) direction[i]=rounded(static_cast<long double>(shifted[i])-original[i]);
    c.set_direction_54(effect,direction);
    c.set_position_08(effect,actor.position_08);
    c.set_pair_4171f0(effect,4.0,0.5);
    c.set_mode_417230(effect,8);
    c.advance_effect_2c(effect,remaining);
    const auto y=std::bit_cast<std::int32_t>(ftol_low(actor.position_08[1]));
    const auto x=ftol_low(actor.position_08[0]);
    const auto triple=c.sample_triple_403f40(x,y);
    auto* child=c.child_4018b0(effect,0);
    c.set_child_triple_403820(child,triple);
    auto* entity=c.create_entity_432080(offset,effect,1);
    c.insert_entity_401890(entity);
    return 2;
}
} // namespace lemmings
