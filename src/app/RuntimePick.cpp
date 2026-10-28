#include "app/RuntimePick.h"

#include <algorithm>

namespace webo::app {

std::optional<int> knownPickPriority(sim::State state) {
  // State +14 categories, not sim enum ordinals or vtable slot indices.
  // Table 4C05D0: category18 -> 0, category19 -> 2, category0 -> 2.
  switch (state) {
    case sim::State::Walker: return 0;   // 41BD7B: category 0x12
    case sim::State::Faller: return 2;   // 415FCC: category 0x13
    case sim::State::Climber:            // 41376B: category 0
    case sim::State::Hoister: return 2;  // 413C8B: category 0
    default: return std::nullopt;
  }
}

bool passesKnownPickExclusions(const sim::Lemming& lemming, sim::Skill skill) {
  if (!lemming.active()) return false;
  if (skill != sim::Skill::Climber) return true;
  // canClimb is the source representation of the retained Climber capability;
  // native 42ECC9 checks category0 in both the current and parallel state lists.
  if (lemming.canClimb) return false;
  switch (lemming.state) {
    case sim::State::Climber:
    case sim::State::Hoister: // 40C061 excludes category0 even if ability flag differs
    case sim::State::Faller: // 4183D3 excludes category0x13
    case sim::State::Bounce: // 4183D3 excludes category0x21
      return false;
    default: return true; // other native eligibility predicates still unresolved
  }
}

std::optional<bool> firstLevelClimberSupport(const sim::Lemming& lemming,
    sim::Skill skill, const sim::Terrain& terrain) {
  if (skill != sim::Skill::Climber || lemming.state != sim::State::Walker ||
      lemming.verticalDir != 1) return std::nullopt;
  if (terrain.empty()) return false;
  // 42F66B produces left bias -1; 42E913 adds1 for Walker, cancelling it.
  // Native Y-up minus1 becomes SOURCE y+1. Clamp to the real row; Terrain's
  // implicit below-bottom Steel sentinel is not the native support sample.
  const int y = static_cast<int>(std::clamp(static_cast<long long>(lemming.y) + 1,
                                         0LL, static_cast<long long>(terrain.height() - 1)));
  return terrain.solid(lemming.x, y) && !terrain.trampoline(lemming.x, y);
}

PickCandidate runtimePickCandidate(void* id, const PickBox& box,
    const sim::Lemming& lemming, sim::Skill skill,
    const std::array<double, 3>& terrainPosition,
    const std::array<double, 3>& reference, std::optional<bool> terrainSupport) {
  return {id, box, passesKnownPickExclusions(lemming, skill) && terrainSupport.value_or(true),
          knownPickPriority(lemming.state).value_or(0),
          pickDistanceSquared(terrainPosition, reference)};
}

}  // namespace webo::app
