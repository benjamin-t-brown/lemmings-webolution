#include "app/RuntimePick.h"

#include <cstdio>
#include <cstdlib>

namespace {
void check(bool ok, const char* message) {
  if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
}

int main() {
  using namespace webo;
  using namespace webo::app;
  check(knownPickPriority(sim::State::Walker) == 0, "Walker category18 priority0");
  check(knownPickPriority(sim::State::Faller) == 2, "Faller category19 priority2");
  check(knownPickPriority(sim::State::Climber) == 2, "Climber category0 priority2");
  check(knownPickPriority(sim::State::Hoister) == 2, "Hoister category0 priority2");
  check(!knownPickPriority(sim::State::Builder), "unmapped category stays explicitly unknown");
  sim::Lemming walker;
  walker.state = sim::State::Walker;
  auto falling = walker; falling.state = sim::State::Faller;
  const PickBox box{100, 100, 110, 120}, cursor{90, 90, 122, 122};
  const std::array<double, 3> reference{32, 48, 0.5};
  int first = 0, second = 1;
  PickCandidate distance[] = {
      runtimePickCandidate(&first, box, walker, sim::Skill::Climber, {60,32,0.75}, reference),
      runtimePickCandidate(&second, box, walker, sim::Skill::Climber, {40,32,0.75}, reference)};
  check(pickHoveredLemming(distance, cursor)->id == &second,
        "production candidate keys select closer later Walker");
  const std::array<double, 3> movedReference{64, 48, 0.5};
  PickCandidate moved[] = {
      runtimePickCandidate(&first, box, walker, sim::Skill::Climber, {60,32,0.75}, movedReference),
      runtimePickCandidate(&second, box, walker, sim::Skill::Climber, {40,32,0.75}, movedReference)};
  check(pickHoveredLemming(moved, cursor)->id == &first,
        "same actor positions and rectangles use supplied moved target, not fixed start");
  // Floater applicability is intentionally not claimed: this checks the native
  // priority input independently of the first-level Climber exclusion path.
  PickCandidate priority[] = {
      runtimePickCandidate(&first, box, walker, sim::Skill::Floater, reference, reference),
      runtimePickCandidate(&second, box, falling, sim::Skill::Floater, {200,0,0.75}, reference)};
  check(pickHoveredLemming(priority, cursor)->id == &second,
        "higher Faller priority beats much closer Walker");
  walker.canClimb = true;
  distance[1] = runtimePickCandidate(&second, box, walker, sim::Skill::Climber,
                                     {40,32,0.75}, reference);
  check(pickHoveredLemming(distance, cursor)->id == &first,
        "already-Climber candidate is excluded before choosing overlap winner");
  walker.canClimb = false;
  for (auto state : {sim::State::Faller, sim::State::Climber, sim::State::Hoister,
                     sim::State::Bounce, sim::State::Splatter, sim::State::Dead,
                     sim::State::Exited}) {
    walker.state = state;
    check(!passesKnownPickExclusions(walker, sim::Skill::Climber),
          "known unavailable Climber candidate excluded");
  }
  {
    sim::Terrain terrain(256, 96);
    sim::Lemming supported;
    supported.state = sim::State::Walker;
    supported.x = 60; supported.y = 63;
    terrain.set(60, 64, sim::Material::Earth);
    const auto support = [&](const sim::Lemming& l, sim::Skill skill = sim::Skill::Climber) {
      return firstLevelClimberSupport(l, skill, terrain);
    };
    check(support(supported) == true, "upright Walker samples row below feet");
    supported.facing = sim::Facing::Left;
    check(support(supported) == true, "left bias cancels; no wrong next-column support probe");
    terrain.set(60, 64, sim::Material::Empty);
    terrain.set(60, 63, sim::Material::Earth);
    check(support(supported) == false, "solid feet cell does not substitute for support cell");
    terrain.set(60, 64, sim::Material::Steel);
    check(support(supported) == true, "steel supports assignment");
    terrain.setTrampoline(60, 64, true);
    check(support(supported) == false, "trampoline marker rejects otherwise solid support");
    terrain.setTrampoline(60, 64, false);
    sim::Lemming unsupported = supported; unsupported.x = 40;
    PickCandidate supportedOverlap[] = {
        runtimePickCandidate(&first, box, unsupported, sim::Skill::Climber,
            {40,32,.75}, reference, support(unsupported)),
        runtimePickCandidate(&second, box, supported, sim::Skill::Climber,
            {60,32,.75}, reference, support(supported))};
    check(pickHoveredLemming(supportedOverlap, cursor)->id == &second,
          "terrain support filters closer unsupported actor before overlap ranking");
    unsupported.y = 95;
    check(support(unsupported) == false, "last empty row clamps instead of using synthetic Steel sentinel");
    terrain.set(40, 95, sim::Material::Earth);
    check(support(unsupported) == true, "last solid row remains supported after clamping");
    unsupported.x = 256; unsupported.y = 63;
    terrain.set(0, 64, sim::Material::Earth);
    check(support(unsupported) == true, "support wraps source X seam");
    unsupported.verticalDir = -1;
    check(!support(unsupported).has_value(), "inverted anchor correspondence explicitly unknown");
    unsupported.verticalDir = 1;
    check(!support(unsupported, sim::Skill::Floater).has_value(), "other skill support explicitly unknown");
    unsupported.state = sim::State::Builder;
    check(!support(unsupported).has_value(), "non-Walker support explicitly unknown");
  }
  std::puts("RuntimePick: all checks passed (partial native mapping)");
}
