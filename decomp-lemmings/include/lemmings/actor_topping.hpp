#pragma once
#include "lemmings/actor_geometry.hpp"
#include <string_view>

namespace lemmings {
// Type 0x1E, entered when the climbing collision routine tops out.
// This routine family does not read or write any behavior fields.
struct ToppingActor {
    ActorGeometryState geometry;
    void* model_78 = nullptr; // Logical handle for original inline model storage.
};
struct ActorToppingCallbacks {
    virtual ~ActorToppingCallbacks() = default;
    virtual void select_animation_42fd25(ToppingActor&, std::string_view) = 0;
    virtual void advance_model_2c(ToppingActor&, double elapsed) = 0;
    virtual std::uint8_t finished_model_44(ToppingActor&) = 0;
    virtual long double model_time_48(ToppingActor&) = 0;
    virtual void select_landed_42f8b9(ToppingActor&) = 0;
};
void enter_actor_topping(ToppingActor&, ActorToppingCallbacks&); // 0x417CCC
// 0x417CE6. On completion, snap/move X by direction_40 and Y by three times
// direction_44, replace remaining with model slot +48, then select landed.
// Positive grid height and a terminating X wrap input are required.
std::uint32_t update_actor_topping(ToppingActor&, double& remaining,
                                   const CollisionGridHead&, ActorToppingCallbacks&);
// 0x417DD7: subtract one vertical and horizontal direction, without wrap/clamp.
void leave_actor_topping(ToppingActor&);
} // namespace lemmings
