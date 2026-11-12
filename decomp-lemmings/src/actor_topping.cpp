#include "lemmings/actor_topping.hpp"
#include "lemmings/cylinder_coordinate.hpp"
#include <bit>

namespace lemmings {
namespace {
std::int32_t integer_low(long double x) {
    constexpr long double limit=9223372036854775808.0L;
    if (!(x>=-limit && x<limit)) return 0;
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::int64_t>(x)));
}
}
void enter_actor_topping(ToppingActor& actor, ActorToppingCallbacks& cb) {
    cb.select_animation_42fd25(actor,"flip");
}
std::uint32_t update_actor_topping(ToppingActor& actor, double& remaining,
                                   const CollisionGridHead& grid, ActorToppingCallbacks& cb) {
    cb.advance_model_2c(actor,remaining);
    if (cb.finished_model_44(actor)==0) { remaining=0.0;return 0; }
    auto& g=actor.geometry;
    const auto x=std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(integer_low(g.position[0]))+
                                          static_cast<std::uint32_t>(g.direction_40));
    g.position[0]=integer_low(wrap_cylinder_x(static_cast<double>(x)));
    const auto y=std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(integer_low(g.position[1]))+
                                          static_cast<std::uint32_t>(g.direction_44)*3u);
    g.position[1]=integer_low(clamp_cylinder_y(grid,static_cast<double>(y)));
    remaining=static_cast<double>(cb.model_time_48(actor));
    cb.select_landed_42f8b9(actor);
    return 1;
}
void leave_actor_topping(ToppingActor& actor) {
    auto& g=actor.geometry;
    g.position[1]=static_cast<double>(static_cast<long double>(g.position[1])-g.direction_44);
    g.position[0]=static_cast<double>(static_cast<long double>(g.position[0])-g.direction_40);
}
} // namespace lemmings
