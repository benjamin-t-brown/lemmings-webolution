#include "lemmings/actor_construction.hpp"

namespace lemmings {
ActorConstructionState& construct_actor(ActorConstructionState& actor, double x, double y,
    std::int32_t category, ActorConstructionGlobals globals, ActorConstructionCallbacks& callbacks) {
    const std::array<double, 3> position{x, y, 0.5};
    callbacks.construct_base_4230a2(actor, position);
    // Pair +68, +350, +360, +390 and object +348 constructors are empty.
    callbacks.construct_model_43ce00(actor);
    callbacks.construct_extra_list_430090(actor);
    actor.original_vtable = 0x4b10e8;
    auto* storage = callbacks.allocate_49c1a8(0x3d0);
    actor.shadow_344 = storage ? callbacks.construct_shadow_457f40(storage, actor) : nullptr;
    actor.flag_38c = 0;
    callbacks.bind_resource_46836d(actor, globals.resource_ids_4e335c[category]);
    actor.flag_60 = 0;
    actor.geometry.flag_5c = 0;
    actor.flag_5f = 0;
    actor.flag_4a = 0;
    actor.flag_5e = 1;
    actor.forced_5d = 0;
    actor.pair_68 = {0.0, 0.0};
    actor.geometry.direction_40 = 1;
    actor.geometry.direction_44 = 1;
    auto& count = globals.counts_4cba78[category];
    count = static_cast<std::uint16_t>(count + 1u);
    callbacks.refresh_444abc();
    storage = callbacks.allocate_49c1a8(0x28);
    auto* state = storage ? callbacks.construct_monitor_4151f0(storage) : nullptr;
    callbacks.attach_42e696(actor, state);
    actor.geometry.category_4c = category;
    actor.gadget_54 = nullptr;
    actor.flag_370 = 1;
    actor.primary_340 = nullptr;
    storage = callbacks.allocate_49c1a8(0x28);
    state = storage ? callbacks.construct_falling_415fa0(storage) : nullptr;
    callbacks.attach_42e696(actor, state);
    actor.climb_48 = 0;
    actor.umbrella_49 = 0;
    actor.geometry.pair_350 = {0.0, 3.0};
    actor.geometry.pair_360 = {0.0, 0.0};
    callbacks.sync_model_position_08(actor, actor.geometry.position);
    callbacks.register_actor_43b517(actor);
    actor.mode_50 = 1;
    return actor;
}
} // namespace lemmings
