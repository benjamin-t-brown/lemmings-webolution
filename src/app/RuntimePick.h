#pragma once

#include "app/Pick.h"
#include "sim/World.h"

#include <optional>

namespace webo::app {

// Returns priority for mapped native states. The caller supplies the clone's zero fallback.
std::optional<int> knownPickPriority(sim::State state);

// Climber exclusions at 45A0E0 -> 40C061 -> 4183D3. Callers handle
// visibility and terrain support separately. Native lifecycle byte +370,
// facing and pending-state checks are outside this helper. Other skills
// use the clone's active-actor eligibility.
bool passesKnownPickExclusions(const sim::Lemming& lemming, sim::Skill skill);

// Support check for MM_CLIMB_002 collision words and integer coordinates.
std::optional<bool> firstLevelClimberSupport(const sim::Lemming& lemming,
    sim::Skill skill, const sim::Terrain& terrain);

PickCandidate runtimePickCandidate(void* id, const PickBox& box,
    const sim::Lemming& lemming, sim::Skill skill,
    const std::array<double, 3>& terrainPosition,
    const std::array<double, 3>& reference,
    std::optional<bool> terrainSupport = std::nullopt);

struct ScreenPickCandidate {
  void* id = nullptr;
  PickBox box;
  bool eligible = true;
  std::array<double, 2> bodyCentre{};
};

// Provisional clone ranking uses squared distance from displayed sprite-box
// centre to cursor-dot centre. Exact ties keep the first candidate in World order.
const ScreenPickCandidate* pickNearestScreenLemming(
    std::span<const ScreenPickCandidate> candidates, int cursorX, int cursorY);

}  // namespace webo::app
