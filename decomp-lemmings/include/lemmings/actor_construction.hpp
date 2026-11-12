#pragma once

#include "lemmings/actor_geometry.hpp"
#include "lemmings/actor_update.hpp"
#include <span>

namespace lemmings {

// A field projection, not the executable's object layout. Fields absent here
// belong to the base/model/list or other unresolved object components.
struct ActorConstructionState {
    ActorGeometryState geometry;
    std::uint32_t original_vtable = 0;
    std::uint8_t climb_48 = 0, umbrella_49 = 0, flag_4a = 0;
    std::int32_t mode_50 = 0;
    void* gadget_54 = nullptr;
    std::uint8_t forced_5d = 0, flag_5e = 0, flag_5f = 0, flag_60 = 0;
    std::array<double, 2> pair_68{};
    ActorBehavior* primary_340 = nullptr;
    void* shadow_344 = nullptr;
    std::array<std::uint64_t, 1> unknown_348{}; // Empty 0x430A90 constructor.
    std::uint8_t flag_370 = 0, flag_38c = 0;
    std::array<std::uint64_t, 2> unknown_pair_390{}; // Empty 0x4821D0 constructor.
};

struct ActorConstructionGlobals {
    std::span<const std::int32_t> resource_ids_4e335c;
    std::span<std::uint16_t> counts_4cba78;
};

struct ActorConstructionCallbacks {
    virtual ~ActorConstructionCallbacks() = default;
    virtual void construct_base_4230a2(ActorConstructionState&, const std::array<double, 3>& position) = 0;
    virtual void construct_model_43ce00(ActorConstructionState&) = 0;
    virtual void construct_extra_list_430090(ActorConstructionState&) = 0;
    virtual void* allocate_49c1a8(std::uint32_t bytes) = 0;
    virtual void* construct_shadow_457f40(void* storage, ActorConstructionState&) = 0;
    virtual void bind_resource_46836d(ActorConstructionState&, std::int32_t resource) = 0;
    virtual void refresh_444abc() = 0;
    virtual ActorBehavior* construct_monitor_4151f0(void* storage) = 0;
    virtual ActorBehavior* construct_falling_415fa0(void* storage) = 0;
    virtual void attach_42e696(ActorConstructionState&, ActorBehavior*) = 0;
    // Original model +0x78 virtual slot +8 receives a pointer to actor.position.
    virtual void sync_model_position_08(ActorConstructionState&, std::array<double, 3>& position) = 0;
    virtual void register_actor_43b517(ActorConstructionState&) = 0;
};

// 0x42DE51 normal-return orchestration. Category must index both globals.
// Callbacks implement the named boundaries and must return normally; original
// SEH unwind/destruction paths are outside this projection. Null allocations
// skip construction but still pass nullptr to attach, as the original does.
// No implied initialization of unspecified fields: caller defaults are not
// evidence that the executable initializes those bytes.
ActorConstructionState& construct_actor(ActorConstructionState&, double x, double y,
    std::int32_t category, ActorConstructionGlobals, ActorConstructionCallbacks&);

} // namespace lemmings
