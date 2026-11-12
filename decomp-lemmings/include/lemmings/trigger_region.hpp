#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "lemmings/gadget_subboxes.hpp"

namespace lemmings {

// Known prefix of the object used by 0x0041DE6B. Its remaining fields and
// precise gameplay type are not yet recovered.
struct TriggerRegionHead {
    std::array<std::byte, 8> unknown_prefix;
    double center_x;
    double center_y;
    double center_z;
    std::array<std::byte, 0x18> unknown_to_enabled;
    std::uint8_t enabled;
    std::array<std::byte, 0x5F> unknown_to_timer;
    std::uint8_t timer_active;
    std::array<std::byte, 7> unknown_before_timer;
    double timer_remaining;
};
static_assert(offsetof(TriggerRegionHead, center_x) == 8);
static_assert(offsetof(TriggerRegionHead, center_y) == 0x10);
static_assert(offsetof(TriggerRegionHead, center_z) == 0x18);
static_assert(offsetof(TriggerRegionHead, enabled) == 0x38);
static_assert(offsetof(TriggerRegionHead, timer_active) == 0x98);
static_assert(offsetof(TriggerRegionHead, timer_remaining) == 0xA0);

// A point is tested against a 12x12 XY square and Z interval [0, 1].
// The original computes half-width as 12.0 / 2.0 on the x87 stack.
bool trigger_region_contains_point(const TriggerRegionHead& region,
                                   const double (&point)[3]);

// Recovered from 0x0041DD4B: arm once, leaving an already active timer alone.
void arm_trigger_timer_once(TriggerRegionHead& region, double duration);

// Recovered from 0x0041DD86. The original reads an actor-pointer array at
// 0x004C89A8; the source receives that list explicitly.
void arm_nearby_trigger_timers(const double (&origin)[3], double radius,
                               std::span<TriggerRegionHead* const> actors);

using TerrainEffect = void (*)(const double (&position)[3], void* context);

// Recovered state transition at 0x0041DF31. The original terrain effect at
// 0x0043F510 is supplied as a callback until that routine is recovered.
// `boxes` and `region` are two views of the same original object.
bool update_trigger_region(TriggerRegionHead& region,
                           const GadgetSubboxHead& boxes,
                           const ActorListHead& retained,
                           std::span<TriggerRegionHead* const> all_actors,
                           double elapsed,
                           TerrainEffect terrain_effect,
                           void* effect_context);

} // namespace lemmings
