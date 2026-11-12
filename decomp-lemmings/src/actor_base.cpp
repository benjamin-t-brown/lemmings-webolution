#include "lemmings/actor_base.hpp"
#include <cstring>

namespace lemmings {
ActorBaseState& construct_actor_base(ActorBaseState& actor,
    const std::array<double, 3>& position, std::uint16_t& next_id,
    const ActorBaseRegistryHooks& hooks) {
    actor.position = {0.0, 0.0, 0.0}; // 0x4066A0 -> 0x4017F0.
    actor.original_vtable = 0x4b0e90;
    actor.flag_38 = 0;
    actor.flag_28 = 1;
    // Original REP MOVSD preserves every bit, including signaling NaNs.
    std::memmove(actor.position.data(), position.data(), sizeof(actor.position));
    actor.field_30 = 0.0;
    actor.field_20 = 0.0;
    actor.id_3a = next_id;
    next_id = static_cast<std::uint16_t>(next_id + 1u);
    auto* registry_argument = &actor;
    hooks.register_4233a0(hooks.context, registry_argument);
    return actor;
}
} // namespace lemmings
