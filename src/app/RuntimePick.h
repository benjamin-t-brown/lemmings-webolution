#pragma once

#include "app/Pick.h"
#include "sim/World.h"

#include <optional>

namespace webo::app {

// Only state identities whose constructors have been traced. Unknown is not
// priority zero; the runtime's zero fallback for other states is SOURCE policy.
std::optional<int> knownPickPriority(sim::State state);

// Necessary Climber exclusions recovered from 45A0E0 -> 40C061 -> 4183D3.
// NOT a complete can-assign predicate: general native terrain support, pending
// states, lifecycle byte +370 and facing filters remain outside. TubeScene now
// supplies submission visibility separately; bounded support is provided below.
// Other skills retain the old SOURCE active-actor eligibility policy.
bool passesKnownPickExclusions(const sim::Lemming& lemming, sim::Skill skill);

// DERIVED for the authored MM_CLIMB_002 word alphabet and the existing SOURCE
// integer coordinate adapter. Call only for that level's terrain; this is NOT
// a general native raw-word predicate. Other skills/states/gravity are unknown.
std::optional<bool> firstLevelClimberSupport(const sim::Lemming& lemming,
    sim::Skill skill, const sim::Terrain& terrain);

PickCandidate runtimePickCandidate(void* id, const PickBox& box,
    const sim::Lemming& lemming, sim::Skill skill,
    const std::array<double, 3>& terrainPosition,
    const std::array<double, 3>& reference,
    std::optional<bool> terrainSupport = std::nullopt);

}  // namespace webo::app
