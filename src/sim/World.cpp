#include "World.h"

#include <algorithm>
#include <cmath>

namespace webo::sim {

namespace {
bool insideTeleport(const Terrain& terrain, const Teleport& t, const Lemming& l) {
  const int dx = terrain.wrapX(l.x - t.x + terrain.width() / 2) - terrain.width() / 2;
  const int dy = l.y - t.y;
  return dx >= 0 && dx <= t.width && dy >= 0 && dy <= t.height;
}

void releaseTeleportContacts(const LevelSetup& setup, Lemming& l) {
  // 0x435467..0x435478: remove an excluded actor only outside the pad.
  // This also runs for non-walkers, so falling/skill motion can rearm a visit.
  std::erase_if(l.teleportPadsLatched, [&](std::size_t i) {
    return i >= setup.teleports.size() || !insideTeleport(setup.terrain, setup.teleports[i], l);
  });
}
}  // namespace

std::string_view skillName(Skill s) {
  switch (s) {
    case Skill::Climber: return "climber";
    case Skill::Floater: return "floater";
    case Skill::Bomber: return "bomber";
    case Skill::Blocker: return "blocker";
    case Skill::Builder: return "builder";
    case Skill::Basher: return "basher";
    case Skill::Miner: return "miner";
    case Skill::Digger: return "digger";
  }
  return "?";
}

std::string_view stateName(State s) {
  switch (s) {
    case State::Faller: return "faller";
    case State::Walker: return "walker";
    case State::Climber: return "climber";
    case State::Hoister: return "hoister";
    case State::Floater: return "floater";
    case State::Blocker: return "blocker";
    case State::Builder: return "builder";
    case State::Shrugger: return "shrugger";
    case State::Basher: return "basher";
    case State::Miner: return "miner";
    case State::Digger: return "digger";
    case State::Exploder: return "exploder";
    case State::Splatter: return "splatter";
    case State::Exited: return "exited";
    case State::Teleporting: return "teleporting";
    case State::Dead: return "dead";
    case State::Bounce: return "bounce";
    case State::Drown: return "drown";
  }
  return "?";
}

World::World(LevelSetup setup) : World(std::move(setup), Rules{}) {}

World::World(LevelSetup setup, Rules rules) : setup_(std::move(setup)), rules_(rules) {
  lemmings_.reserve(static_cast<std::size_t>(std::max(0, setup_.totalLemmings)));


  sinceSpawn_ = rules_.walkSpeed > 0 ? setup_.releaseIntervalTicks : 0;
}

int World::activeCount() const {
  int n = 0;
  for (const auto& l : lemmings_)
    if (l.active())
      ++n;
  return n;
}

bool World::finished() const {
  if (remainingToRelease() > 0)
    return false;
  for (const auto& l : lemmings_)
    if (l.alive())
      return false;
  return true;
}

void World::place(Lemming lemming) {
  lemming.x = setup_.terrain.wrapX(lemming.x);
  lemmings_.push_back(lemming);
}

void World::spawn(double budgetSeconds) {
  if (spawned_ >= setup_.totalLemmings || setup_.entrances.empty())
    return;
  const std::size_t entranceIndex = nextEntrance_ % setup_.entrances.size();
  Entrance& e = setup_.entrances[entranceIndex];
  nextEntrance_ = (nextEntrance_ + 1) % setup_.entrances.size();

  Lemming l;
  l.x = setup_.terrain.wrapX(e.x);
  l.y = e.y;
  l.state = State::Faller;
  // Round 29: hatch+0xB0 -> [lemming+0x4c] at 0x0042E05F.
  l.sheetType = e.sheetType;
  // 0x42DFDD initializes +0x40=1; 0x425C6E..0x425C7D reverses it only
  // for hatch +0xB4 == -1. Toggle this hatch AFTER choosing the birth face,
  // before the actor's first update (0x425C82..0x425CCC).
  auto& direction = e.direction;
  l.facing = direction == -1 ? Facing::Left : Facing::Right;
  if (e.alternateDirection)
    direction = direction == 1 ? -1 : 1;
  // D1n: under the opt-in continuous stepper clock, a lemming released
  // partway through the tick that spawned it (continuous release model
  // only -- the caller passes a reduced budgetSeconds in that case) should
  // not get a full tick's worth of fall time before the next World::tick()
  // call. budgetSeconds < 0.0 (every integer-release-model call, and every
  // continuous-release call that happened to land exactly on a tick
  // boundary) means "the full tick," matching the old unconditional
  // behaviour.
  if (continuousMotion()) {
    const double dt = motionDtSeconds();
    const double budget = budgetSeconds >= 0.0 ? budgetSeconds : dt;
    l.stepBudgetSeconds = budget - dt;
  }
  lemmings_.push_back(l);
  ++spawned_;
}
void World::stampBlockerField(const Lemming& l) {
  // 40E2D7 multiplies the row offset by actor+44; 40E19A does the
  // same on exit. Convert Y-up to our feet-based Y-down coordinates.
  // The exact original +368 anchor conversion remains provisional, as for
  // inverted drawing; reflect the accepted upright footprint about the feet.
  const int v = l.verticalDir >= 0 ? 1 : -1;
  for (int k = 0; k < rules_.blockerFieldHeight; ++k)
    setup_.terrain.setBlockerField(l.x, l.y - k * v, true);
}
void World::clearBlockerField(const Lemming& l) {
  const int v = l.verticalDir >= 0 ? 1 : -1;
  for (int k = 0; k < rules_.blockerFieldHeight; ++k)
    setup_.terrain.setBlockerField(l.x, l.y - k * v, false);
}

void World::leaveBlocker(Lemming& l) {
  if (l.state == State::Blocker)
    clearBlockerField(l);
}

bool World::atExit(const Lemming& l) const {
  for (const Exit& e : setup_.exits) {
    const int dx = setup_.terrain.wrapX(l.x - e.x + setup_.terrain.width() / 2) -
                   setup_.terrain.width() / 2;
    const int dy = l.y - e.y;
    if (dx >= 0 && dx <= e.width && dy >= 0 && dy <= e.height)
      return true;
  }
  return false;
}

const Teleport* World::teleportAt(const Lemming& l) const {
  for (std::size_t i = 0; i < setup_.teleports.size(); ++i) {
    const Teleport& t = setup_.teleports[i];
    // 0x435501/0x43550D: an id already in this pad's +0x248 list is skipped.
    if (insideTeleport(setup_.terrain, t, l) &&
        std::find(l.teleportPadsLatched.begin(), l.teleportPadsLatched.end(), i) ==
            l.teleportPadsLatched.end())
      return &t;
  }
  return nullptr;
}

const SpeedPad* World::speedPadAt(const Lemming& l) const {
  for (const SpeedPad& p : setup_.speedPads) {
    const int dx = setup_.terrain.wrapX(l.x - p.x + setup_.terrain.width() / 2) -
                   setup_.terrain.width() / 2;
    const int dy = l.y - p.y;
    if (dx < 0 || dx > p.width || dy < 0 || dy > p.height)
      continue;
    if (p.faceFilter != 0 &&
        static_cast<int>(l.facing) != p.faceFilter)
      continue;
    if (p.requireUpright && l.verticalDir != 1)
      continue;
    if (!p.requireUpright && l.verticalDir != -1)
      continue;
    return &p;
  }
  return nullptr;
}

bool World::leverAccepts(const Lever& lev, const Lemming& l) const {
  // 0x00436D30: AABB then facing vs +0xb8. state==1 needs face −1; else +1.
  const int face = static_cast<int>(l.facing);
  const int dx =
      setup_.terrain.wrapX(l.x - lev.x + setup_.terrain.width() / 2) -
      setup_.terrain.width() / 2;
  const int dy = l.y - lev.y;
  if (dx < 0 || dx > lev.width || dy < 0 || dy > lev.height)
    return false;
  return lev.state == 1 ? face == -1 : face == 1;
}

Lever* World::leverAt(const Lemming& l) {
  for (Lever& lev : setup_.levers)
    if (leverAccepts(lev, l))
      return &lev;
  return nullptr;
}

double FluidGate::rateFor(int kind) {
  // Startup writer 0x00428848: kind 0 -> 2.0 at 0x004BF6F8, kind 1 -> 0.5 at 0x004BF740
  // (row stride 0x48). Preserve the accepted kind-0/1 height machine. fluid-kinds-001
  // traces kind 2's rate too, but adds its rendering without expanding height/zone rules.
  switch (kind) {
    case 0: return 2.0;
    case 1: return 0.5;
    default: return 0.0;
  }
}

void FluidGate::initialise(Terrain& terrain) {
  // Reader 0x0042ACAC (0x42af32..0x42afc4): request 1 -> height := low, CLEAR the zone bit over
  // the stream AABB; otherwise height := high and SET it. [+0x68] is not recomputed here.
  const bool drained = request == 1;
  height = drained ? low : high;
  if (paintsZonePlane())
    terrain.paintZoneWaterRect(minX, minY, maxX, maxY, !drained);
}

void FluidGate::step(double dt, Terrain& terrain) {
  stepHeight(dt, terrain);
  advanceWave(dt);
}

void FluidGate::advanceWave(double dt) {
  wavePhase += dt;
  const double period = wavePeriodFor(kind);
  while (period < wavePhase)
    wavePhase -= period;
  animated = true;
}

double FluidGate::wavePeriodFor(int kind) {
  // Startup 0x428848: 0x4BF6E8/0x4BF730/0x4BF778, rows stride 0x48.
  return kind == 1 ? 8.0 : kind == 2 ? 0.8 : kWavePeriod;
}

void FluidGate::stepHeight(double dt, Terrain& terrain) {
  // 0x0042B24D. While request == done the height machine exits to the wave/model tail
  // (advanceWave; the model update itself is rendered from this state by the app).
  if (request == done)
    return;
  const double rate = rateFor(kind);
  if (request == 0) {
    // Raise (message type 4). The comparison is made BEFORE the add and nothing clamps the
    // add, so the update that crosses `high` overshoots by less than dt * rate and the next
    // update snaps to `high`; rows the overshoot set are never cleared. The settle update
    // still repaints (it is the unconditional tail of this branch).
    if (height < high) {
      height = dt * rate + height;
    } else {
      height = high;
      if (armed) {
        armed = false;
        done = 0;
      }
    }
    maxY = minY + height;
    if (paintsZonePlane())
      terrain.paintZoneWaterRect(minX, minY, maxX, maxY, true);
  } else {
    // Drain (message type 5). Settling dispatches the +0x12C list in the original; the
    // serialized objects carry none, so only the bookkeeping is reproduced. The settle path
    // neither paints nor recomputes [+0x68].
    if (height <= low) {
      height = low;
      if (!armed) {
        armed = true;
        done = 1;
      }
    } else {
      if (paintsZonePlane())
        terrain.paintZoneWaterRect(minX, minY, maxX, maxY, false);  // CLEAR the current AABB
      height = height - dt * rate;
      maxY = minY + height;
      if (low < height && paintsZonePlane())
        terrain.paintZoneWaterRect(minX, minY, maxX, maxY, true);  // SET the shortened AABB
    }
  }
}

void dispatchLeverMessages(LevelSetup& setup, const std::vector<LeverMessage>& list) {
  for (const LeverMessage& m : list) {
    // 0x00433B2E: only `elem+4 >= 0` is a list lookup; an empty slot is skipped.
    if (m.targetId < 0) {
      ++setup.leverStats.unsupported;
      continue;
    }
    const auto it = setup.objectIds.find(m.targetId);
    if (it == setup.objectIds.end()) {
      ++setup.leverStats.unresolved;
      continue;
    }
    const ObjectRef& ref = it->second;
    const bool gateTarget = ref.cls == ObjectClass::Gate && ref.index >= 0 &&
                            static_cast<std::size_t>(ref.index) < setup.gates.size();
    if (!gateTarget || (m.type != 4 && m.type != 5)) {
      // Type 0 (vt+0xC) and the other classes' vt+4/vt+8 are not traced: bounded, not applied.
      ++setup.leverStats.unsupported;
      continue;
    }
    // vt+4 (0x0042C8C0) writes request 0, vt+8 (0x0042C8E0) writes request 1; nothing else.
    setup.gates[static_cast<std::size_t>(ref.index)].request = m.type == 4 ? 0 : 1;
    ++setup.leverStats.delivered;
  }
}

void activateLeverFace(LevelSetup& setup, Lever& lever, int face) {
  if (face != 1 && face != -1)
    return;  // 0x00436E6A ignores every other face value
  lever.state = face;
  // Dispatch edits gates and counters only, never the lever lists being iterated.
  dispatchLeverMessages(setup, face == 1 ? lever.facePlus : lever.faceMinus);
}

void fanOutLeverLoadFaces(LevelSetup& setup) {
  // 0x00444417 -> vt+0x28 0x0043703F -> 0x00436E6A([+0xDC]): the stored face is the state AND the
  // list that is sent, in one step, so the two cannot disagree. activateLeverFace does exactly the
  // runtime step (state := face, then the matching list) and touches neither the cooldown (the load
  // path calls 0x00436E6A directly, not the cooldown-reloading 0x00436E14) nor the cue. A face that
  // is not +-1 changes nothing and the lever keeps its constructed state, as 0x00436E6A ignores it.
  // An unlinked (legacy / synthetic) lever has no serialized face and is left untouched.
  for (Lever& lever : setup.levers) {
    if (!lever.linked)
      continue;
    activateLeverFace(setup, lever, lever.initialFace);
  }
}

void World::stepTeleporting(Lemming& l) {
  // Countdown mirrors state+0x18 from push 4 (0x0041A317). On zero the
  // original attaches type 0x2c then writes partner pose (0x0043503C).
  ++l.frame;
  if (l.frame < rules_.teleportDelayTicks)
    return;
  // destX/destY were stashed in bricksLeft / fallDistance at attach — those
  // fields are idle for a walker-sourced teleport.
  l.x = setup_.terrain.wrapX(l.bricksLeft);
  l.y = l.fallDistance;
  // 0x4350E1..0x4350F9 registers the actor with the partner before relocation.
  // RetailLevel projects each partner pose to its pad's bottom-center cell;
  // retain that identity without changing serialized pairing or the snap rule.
  // Provisional for custom overlapping pads: exclude every pad containing the
  // arrival cell. Installed links all have exactly one containing destination.
  for (std::size_t i = 0; i < setup_.teleports.size(); ++i) {
    if (insideTeleport(setup_.terrain, setup_.teleports[i], l) &&
        std::find(l.teleportPadsLatched.begin(), l.teleportPadsLatched.end(), i) ==
            l.teleportPadsLatched.end())
      l.teleportPadsLatched.push_back(i);
  }
  l.state = State::Walker;
  l.frame = 0;
  l.bricksLeft = 0;
  l.fallDistance = 0;
}
void World::explode(Lemming& l) {
  std::array<std::array<int, 2>, 49> occupied{};
  std::size_t count = 0;
  if (deathObserver_) {
    for (int y = l.y - 4; y < l.y + 3; ++y)
      for (int x = l.x - 4; x < l.x + 3; ++x)
        if (y >= 0 && y < setup_.terrain.height() &&
            setup_.terrain.at(x, y) != Material::Empty)
          occupied[count++] = {setup_.terrain.wrapX(x), y};
  }
  setup_.terrain.clearExplosion(l.x, l.y);
  l.state = State::Dead;
  ++dead_;
  cue(SoundCue::Explode);
  if (deathObserver_) {
    const auto end = std::remove_if(occupied.begin(), occupied.begin() + count,
        [&](const auto& cell) { return setup_.terrain.at(cell[0], cell[1]) != Material::Empty; });
    deathObserver_(l, true, std::span(occupied.begin(), end));
  }
}
void World::stepExploder(Lemming& l) {






  for (int i = 0; i < rules_.fallSpeed && !solidAt(l.x, l.y + 1); ++i)
    ++l.y;





  if ((l.frame + 1) * msPerTick() <= rules_.exploderCycleMs)
    return;

  ++l.y;
  explode(l);
}
int World::floatOpenCells() const {
  const int ms = msPerTick();
  if (ms <= 0)
    return 0;
  return (rules_.floatOpenMs * rules_.fallSpeed + ms / 2) / ms;
}

void World::stepFaller(Lemming& l) {
  const bool floating = l.canFloat && l.fallDistance >= rules_.floatAfter;
  if (floating)
    l.state = State::Floater;

  // D1n: opt-in continuous sub-tick clock (LevelSetup::continuousStepperClock,
  // docs/scratch/phase-d1-stepper-clock.md). Off (the default, `continuous`
  // false below): `units` is exactly `speed` and `dt`/`unitSeconds`/`budget`
  // are never read, so the loop runs byte-for-byte as it did before this
  // round. On: `units` is how many whole fall cells this tick's own
  // real-time budget (this tick's dt, plus whatever a prior call could not
  // spend -- Lemming::stepBudgetSeconds) actually buys, and a landing
  // mid-loop hands the unconsumed remainder of that budget to stepWalker()
  // within the SAME tick instead of discarding it.

  const bool halved =
      floating && l.fallDistance >= rules_.floatAfter + floatOpenCells();
  const int speed = halved ? rules_.floatSpeed : rules_.fallSpeed;

  const bool continuous = continuousMotion() && speed > 0;
  // D1n: whether this call already carries a REAL, measured sub-tick phase
  // (traced back to a reduced spawn budget from the continuous release
  // model, or a same-tick handoff from an earlier stepper this very tick --
  // see Lemming::stepBudgetSeconds). A fall that has run only on whole,
  // tick-aligned dt budgets so far (every ordinary mid-walk ledge fall,
  // where nothing upstream ever set a fractional carry) reaches its OWN
  // landing tick with `stepBudgetSeconds == 0.0` even though the loop below
  // may stop partway through this tick's `speed` sub-units -- that
  // leftover is an artifact of how this implementation happens to batch a
  // whole tick's fall into `speed` equal-sized cell crossings, not a
  // measured real-time quantity (nothing in D1m's reading establishes that
  // the original's own per-frame dt is quantized to 1/fallSpeed of a
  // 200ms tick), so it must NOT be handed to the walker -- only a call that
  // is already carrying real phase may extend that same real phase forward.
  const bool hasTrackedPhase = continuous && l.stepBudgetSeconds != 0.0;
  // shared-timing-impl-001: `dt` is this update's own elapsed time (motionDtSeconds(): the
  // explicit segment dt, or tickDtSeconds() under legacy tick()); the lattice unit stays
  // the measured tickDtSeconds() / speed, so a legacy 0.2 s call computes exactly as before.
  const double dt = continuous ? motionDtSeconds() : 0.0;
  const double unitSeconds = continuous ? tickDtSeconds() / speed : 0.0;
  const double budget = continuous ? dt + l.stepBudgetSeconds : 0.0;
  const int units = continuous
      ? std::clamp(static_cast<int>(std::floor(budget / unitSeconds + 1e-9)), 0, speed)
      : speed;
  if (continuous)
    l.stepBudgetSeconds = 0.0;  // consumed; set again below if a transition needs to carry it

  const int v = l.verticalDir >= 0 ? 1 : -1;
  for (int i = 0; i < units; ++i) {
    // d2h repair 1 (round 2): native (0x004160bd) writes this sub-unit's
    // move back to lemming+0x10 (0x0041627f) strictly BEFORE calling the
    // land/floater/splat check (0x0041632b -> 0x00416433), which re-reads
    // the lemming's position fresh at its own entry (0x42f22e) -- so the
    // check below tests the cell neighbouring the Y this sub-unit's OWN
    // move already reached, not the Y before it. Moving first here (unlike
    // the prior check-then-move shape) matches that: it lets the final
    // sub-unit of a tick whose fall exactly reaches the ground see the
    // ground as `l.y + v` using the just-updated `l.y`, instead of
    // deferring that same-tick landing to the next call's first sub-unit.
    // build/tmp/d2h-skill-assign-floater-repair1/disasm_confirmation.md has
    // the full byte trace and the gap-to-ground invariant (established from
    // stepWalker's own Faller-entry condition) that makes an unconditional
    // move safe here -- a fall's gap-to-ground is never 1 at the start of
    // any stepFaller call, so this can never walk a lemming INTO solid
    // ground before its own post-move check gets a chance to catch it.
    // PROVISIONAL feet-grid adapter for inverted entries already adjacent to
    // a ceiling: do not move through that support. Upright's accepted move/check
    // order is unchanged; ordinary inverted falls also move first.
    if (v > 0 || !groundAt(l.x, l.y - 1, v)) {
      l.y += v;
      ++l.fallDistance;
    }

    if (v < 0 && (l.y < 0 || l.y >= setup_.terrain.height())) {
      // READ 4164CE..41653A: vertical bounds precede landing and attach the
      // same type-1A loss as an excessive fall; distance/gravity do not exempt
      // it. PROVISIONAL feet/raw-origin adapter: lose at the first cell outside
      // this Y-down grid. Native raw Y/bias/clamp differs (40DF50, 42F22E).
      // Keep the existing clone splat lifetime/counting, without fall retiming.
      l.y = std::clamp(l.y, 0, std::max(0, setup_.terrain.height() - 1));
      l.state = State::Splatter;
      l.frame = 0;
      l.stepBudgetSeconds = 0.0;
      ++dead_;
      cue(SoundCue::Splat);
      if (deathObserver_) deathObserver_(l, false, {});
      return;
    }

    if (attachFluidHazard(l))
      return;

    // Round 14: 0x2f probes the ground cell every sub-step. Live paint of
    // 0x40 on MM_CLIMB_002 attached drown mid-fall when under=(col,floor)
    // before the walk land (`measure_water.py`). Ahead of trampoline/splat.
    if (setup_.terrain.waterHazard(l.x, l.y + v)) {
      l.state = State::Drown;
      l.frame = 0;
      return;
    }
    if (groundAt(l.x, l.y + v, v)) {

      if (setup_.terrain.trampoline(l.x, l.y + v)) {
        l.state = State::Bounce;
        l.fallDistance = 0;
        l.frame = 0;
        return;
      }

      // Round 29: after floater, FUN_00416433 skips splat when the foot
      // cell's 0x200/0x4000 matches sheetType 1/2 (measure_splat_exempt.py).
      const bool zoneExempt =
          setup_.terrain.splatExempt(l.x, l.y + v, l.sheetType);
      if (!floating && !zoneExempt &&
          l.fallDistance > rules_.splatDistance) {
        l.state = State::Splatter;
        ++dead_;
        cue(SoundCue::Splat);
        if (deathObserver_) deathObserver_(l, false, {});
        return;
      }
      l.state = State::Walker;
      l.fallDistance = 0;
      l.frame = 0;
      if (hasTrackedPhase) {
        // D1n: the landing instant is `(i + 1) * unitSeconds` into this
        // call's own budget -- updated from `i * unitSeconds` as a direct,
        // named consequence of the repair-1 reorder above: this sub-unit's
        // OWN move (now counted before this check runs) is part of the
        // consumed budget, so the (i+1)-th sub-unit, not the i-th, is what
        // actually landed. Everything after that was never fall time to
        // begin with (the lemming was already standing on the ground) --
        // hand it to the walk stepper in the SAME tick instead of dropping
        // it, which is the mechanism D1m's replay showed collapses the
        // landing-tick drift (docs/scratch/phase-d1-stepper-clock.md).
        // Gated on hasTrackedPhase (see above): only when this fall's own
        // budget already carries a real measured sub-tick offset.
        const double leftover = budget - (i + 1) * unitSeconds;
        l.stepBudgetSeconds = leftover - dt;
        stepWalker(l);
      }
      return;
    }
  }
  if (continuous)
    l.stepBudgetSeconds = budget - units * unitSeconds;  // cross-tick carry
}

void World::stepBounce(Lemming& l) {





  const int v = l.verticalDir >= 0 ? 1 : -1;
  const int dir = static_cast<int>(l.facing);
  for (int i = 0; i < rules_.bounceSpeed; ++i) {
    if (l.fallDistance >= rules_.bounceRiseCells) {
      l.state = State::Faller;
      l.fallDistance = 0;
      l.frame = 0;
      return;
    }

    if (solidAt(l.x, l.y - v * rules_.lemmingHeight)) {
      l.state = State::Faller;
      l.fallDistance = 0;
      l.frame = 0;
      return;
    }
    l.y -= v;
    l.x = setup_.terrain.wrapX(l.x + dir);
    ++l.fallDistance;
  }
}

void World::stepDrown(Lemming& l) {
  // Round 14. Drown step 0x00408766: add dt to state+8; when > 2.0 return 2
  // (lemming leaves the actor list). Live: acc reached 2.000 then removal
  // (~2.0 s). frame accumulates ms via msPerTick.
  l.frame += msPerTick();
  // Original: return 2 when accumulator > 2.0. Live removals sat at
  // acc=2.000 then left within one sample; at 200 ms ticks, >= 2000 ms is
  // the same discrete length (10 ticks).
  if (l.frame >= rules_.drownMs) {
    l.state = State::Dead;
    ++dead_;
  }
}

bool World::groundAt(int x, int y, int verticalDir) const {
  if (solidAt(x, y))
    return true;
  if (verticalDir < 0 || setup_.terrain.width() <= 0)
    return false;
  const int width = setup_.terrain.width();
  const int row = setup_.terrain.height() - 1 - y;
  for (const FluidGate& g : setup_.gates) {
    if (g.kind < 0 || g.kind > 2 || !(g.height > 0.0))
      continue;
    double span = g.maxX - g.minX;
    if (span < 0.0)
      span += width;
    double dx = std::fmod(x - g.minX, double(width));
    if (dx < 0.0)
      dx += width;
    // Provisional integer support: feet use floor(mean live surface), so
    // the ground probe is one row below it. Visual waves are not collision.
    // Original 0x41C321..0x41C41D separates receiver/species contact from
    // terrain; human MM_W_012 observations establish dry fall / wet travel.
    if (dx < span && row == int(std::floor(g.minY + g.height)) - 1)
      return true;
  }
  return false;
}

bool World::attachFluidHazard(Lemming& l) {
  if (!l.active() || l.state == State::Drown || l.state == State::Teleporting)
    return false;
  // 0x41C321..0x41C36F: water (kind 0) spares sheet 1, acid (2)
  // spares sheet 2; lava (1) always kills. 0x404261 enters "drown".
  // PROVISIONAL playable-v1 contact: feet enter the mean fluid volume,
  // inclusive vertically and half-open in X. The original writer
  // of lem+0x54 / exact immersion threshold is not yet traced. Waves are
  // visual, not a new hazard clock. Read live height, never the stored mask
  // or maxY (which can remain stale after drain-to-low settling).
  // Reuse State::Drown's existing animation/loss bookkeeping and duration;
  // the original fluid-mismatch state (0x23) instead sinks to the bottom.
  // Its sinking/FX lifetime is not an established clone timing contract.
  // A ground-cell probe would kill an actor standing above the surface:
  // MM_BUILD_BLOCK_A has surface Y=28.5 and dry feet at Y=29.
  const double row = setup_.terrain.height() - 1 - l.y;
  const int width = setup_.terrain.width();
  if (width <= 0)
    return false;
  for (const FluidGate& g : setup_.gates) {
    if (g.kind < 0 || g.kind > 2 || !(g.height > 0.0))
      continue;
    double span = g.maxX - g.minX;
    if (span < 0.0) span += width;
    double dx = std::fmod(l.x - g.minX, double(width));
    if (dx < 0.0) dx += width;
    if (dx >= span || row < g.minY || row > g.minY + g.height)
      continue;
    if ((g.kind == 0 && l.sheetType == 1) || (g.kind == 2 && l.sheetType == 2)) {
      // Provisional cell quantization of surface riding. Re-evaluate the
      // live height each update, including while a linked receiver rises.
      // Do not move an actor standing on actual terrain above the fluid.
      if (l.verticalDir > 0 && (l.state == State::Walker ||
          l.state == State::Faller || l.state == State::Floater))
        l.y = setup_.terrain.height() - 1 - int(std::floor(g.minY + g.height));
      continue;
    }
    leaveBlocker(l);
    l.state = State::Drown;
    l.frame = 0;
    l.stepBudgetSeconds = 0.0;
    return true;
  }
  return false;
}

void World::stepWalker(Lemming& l) {
  const int dir = static_cast<int>(l.facing);
  // D1f (round 6), MEASURED via old/docs/scratch/phase3-round10-steppers.md
  // Part 6 (0x0041C67E): captured before either `turn(l)` below can run, so
  // it reflects the facing this call actually walked in on, not any facing
  // a recursive continuation already flipped.
  const bool wasLeft = l.facing == Facing::Left;

  // D1n: opt-in continuous sub-tick clock -- see the matching comment in
  // stepFaller. Off (the default, `continuous` false below): `units` is
  // exactly `rules_.walkSpeed` and `dt`/`unitSeconds`/`budget` are never
  // read, so the loop runs byte-for-byte as it did before this round.
  const bool continuous = continuousMotion() && rules_.walkSpeed > 0;
  // shared-timing-impl-001: same dt / lattice-unit split as stepFaller.
  const double dt = continuous ? motionDtSeconds() : 0.0;
  const double unitSeconds = continuous ? tickDtSeconds() / rules_.walkSpeed : 0.0;
  const double budget = continuous ? dt + l.stepBudgetSeconds : 0.0;
  const int units = continuous
      ? std::clamp(static_cast<int>(std::floor(budget / unitSeconds + 1e-9)), 0,
                   rules_.walkSpeed)
      : rules_.walkSpeed;
  if (continuous)
    l.stepBudgetSeconds = 0.0;  // consumed; set again below if a transition needs to carry it
  const int v = l.verticalDir >= 0 ? 1 : -1;
  const auto flipIfAntiGrav = [&] {
    if (!setup_.terrain.antiGrav(l.x, l.y + v))
      return false;
    l.verticalDir = -v;
    l.state = State::Faller;
    l.fallDistance = 0;
    l.frame = 0;
    return true;
  };
  if (flipIfAntiGrav())
    return;
  if (attachFluidHazard(l))
    return;
  for (int s = 0; s < units; ++s) {
    const int nx = l.x + dir;
    const auto settleOnProbeColumn = [&] {
      if (l.facing == Facing::Right)
        l.x = setup_.terrain.wrapX(nx);
    };

    // Step-up probes along -verticalDir (against gravity). MEASURED round 30:
    // upright add [+0x44] at 0x0041C48E / 0x0041C528; inverted walkers on
    // m06 adopted dy -1 / -2 in Y-up (= +rise in this Y-down terrain).
    int rise = 0;
    while (rise <= rules_.maxStepUp) {
      if (setup_.terrain.blockerField(nx, l.y - rise * v)) {
        settleOnProbeColumn();
        turn(l);
        // MEASURED, round 10 Part 6 (0x0041C67E, after 0x0041C679 calls
        // 0x0042F66B): the original's turn arm returns 0, so its slice loop
        // (0x0041BDD4) keeps going in the SAME tick and a walker that was
        // facing Left steps once in its new (now Right) direction
        // immediately -- live went 24, 25, 26, 27 against the column-23
        // wall where ours went 24, 24, 25, 26. A walker that was facing
        // Right already consumed the tick moving onto the wall column via
        // settleOnProbeColumn() above, so it must NOT get this extra step
        // ("right-facing turns already match" -- do not add it there).
        if (wasLeft) {
          // D1n: only when the recursive call actually happens -- the
          // right-facing case above must stay byte-for-byte untouched, so
          // the leftover-budget handoff lives inside this branch alone.
          if (continuous) {
            const double leftover = budget - s * unitSeconds;
            l.stepBudgetSeconds = leftover - dt;
          }
          stepWalker(l);
        }
        return;
      }
      if (!solidAt(nx, l.y - rise * v))
        break;
      ++rise;
    }

    if (rise > rules_.maxStepUp) {


      settleOnProbeColumn();
      if (l.canClimb) {
        l.state = State::Climber;
        l.frame = 0;
        return;
      }
      turn(l);
      // Same mechanism as the blocker-field turn above: only a walker that
      // was facing Left gets the same-tick continuation. D1f, round 6.
      if (wasLeft) {
        // D1n: same leftover-budget handoff as the blocker-field turn
        // above, same restriction to the wasLeft branch only.
        if (continuous) {
          const double leftover = budget - s * unitSeconds;
          l.stepBudgetSeconds = leftover - dt;
        }
        stepWalker(l);
      }
      return;
    }

    if (rise > 0) {




      l.x = setup_.terrain.wrapX(nx);
      l.y -= rise * v;
      if (attachFluidHazard(l))
        return;
      if (flipIfAntiGrav())
        return;
      continue;
    }
    l.x = setup_.terrain.wrapX(nx);
    if (attachFluidHazard(l))
      return;
    if (flipIfAntiGrav())
      return;



    if (groundAt(l.x, l.y + v, v))
      continue;
    if (groundAt(l.x, l.y + 2 * v, v)) {
      l.y += v;
      if (attachFluidHazard(l))
        return;
      continue;
    }
    l.state = State::Faller;
    l.fallDistance = 0;
    return;
  }
  if (continuous)
    l.stepBudgetSeconds = budget - units * unitSeconds;  // cross-tick carry
}

void World::stepClimber(Lemming& l) {
  const int dir = static_cast<int>(l.facing);
  // Climb advances Y by vdir * 5.0 * dt (0x00413869). Hoist net is
  // +2*vdir (0x00417D6F / exit). MEASURED round 30 on m06: inverted climb
  // then hoist walked at Y 12 after enter at ~14.5 (Y-up, against gravity).
  const int v = l.verticalDir >= 0 ? 1 : -1;

  const int wall = l.facing == Facing::Right
                       ? l.x
                       : setup_.terrain.wrapX(l.x - 1);

  // READ round 2 of d2f-skill-assign-capture (repair 1), disassembly of
  // 0x004137CA (the outer per-tick stepper) and 0x004139BF (the function it
  // calls at 0x004138E8). The Y write-back (0x00413875-0x0041388B) happens
  // strictly BEFORE 0x004139BF's own biased-position read (0x0042F35F,
  // called from 0x004139DD), which both the ceiling probe (0x00413a53) and
  // the topout probe (0x00413b50) are built from. So both checks below test
  // the lemming's Y AFTER this tick's climb move, not before it -- moving
  // first and checking the new Y, below, matches the native ordering.
  // build/tmp/d2f-skill-assign-capture-repair1/disasm_notes.md has the full
  // byte trace. This corrects old/docs/scratch/phase3-round10-steppers.md's
  // "tests the ceiling first (the original's order)" claim, which asserted
  // check-then-move without having the 0x4137CA/0x4139BF call split in view;
  // round 1's live capture (45/50 ticks, one residual: our Hoister label one
  // tick later than the original at the same Y) is what exposed it.
  l.y -= v * rules_.climbSpeed;

  if (solidAt(setup_.terrain.wrapX(wall - dir),
              l.y - v * rules_.lemmingHeight)) {
    l.x = setup_.terrain.wrapX(l.x - dir);
    turn(l);
    l.state = State::Faller;
    l.fallDistance = 0;
    return;
  }

  if (!solidAt(wall, l.y - v * rules_.climberTopOut)) {
    l.state = State::Hoister;
    l.frame = 0;
  }
}

void World::stepHoister(Lemming& l) {
  if ((l.frame + 1) * msPerTick() <= rules_.hoistCycleMs)
    return;

  const int v = l.verticalDir >= 0 ? 1 : -1;
  l.y -= v * rules_.hoistLift;
  l.state = State::Walker;
  l.frame = 0;
}
static bool bitesThisTick(int frame, int msPerTick, int cycleMs, int percent) {
  if (cycleMs <= 0 || msPerTick <= 0)
    return frame == 0;
  const long long a = static_cast<long long>(frame) * msPerTick;
  const long long phase = static_cast<long long>(cycleMs) * percent / 100;

  const long long k = a <= phase ? 0 : (a - phase + cycleMs - 1) / cycleMs;
  const long long t = k * cycleMs + phase;
  return t >= a && t < a + msPerTick;
}

int World::msPerTick() const {
  return rules_.ticksPerSecond > 0 ? 1000 / rules_.ticksPerSecond : 200;
}
double World::tickDtSeconds() const {
  return rules_.ticksPerSecond > 0 ? 1.0 / rules_.ticksPerSecond : 0.2;
}
void World::stepBuilder(Lemming& l) {
  if (!bitesThisTick(l.frame, msPerTick(), rules_.builderCycleMs,
                     rules_.builderBrickPercent - rules_.builderEnterPercent))
    return;
  const int dir = static_cast<int>(l.facing);




  for (int k = 1; k <= rules_.brickLength; ++k) {
    const int bx = setup_.terrain.wrapX(l.x + k * dir);
    if (!setup_.terrain.solid(bx, l.y))
      setup_.terrain.set(bx, l.y, Material::Built);
  }





  l.y -= 1;
  if (solidAt(setup_.terrain.wrapX(l.x + dir), l.y)) {
    turn(l);
    l.state = State::Walker;
    l.frame = 0;
    return;
  }

  l.x = setup_.terrain.wrapX(l.x + dir);
  --l.bricksLeft;
  if (l.bricksLeft <= 0) {
    l.state = State::Shrugger;
    l.frame = 0;
    return;
  }
  if (solidAt(l.x, l.y - rules_.lemmingHeight)) {
    turn(l);
    l.state = State::Walker;
    l.frame = 0;
  }
}
void World::stepShrugger(Lemming& l) {
  if (l.frame * msPerTick() >= rules_.shrugCycleMs) {
    l.state = State::Walker;
    l.frame = 0;
  }
}

void World::stepBasher(Lemming& l) {
  if (!bitesThisTick(l.frame, msPerTick(), rules_.basherCycleMs, rules_.basherBitePercent))
    return;
  const int dir = static_cast<int>(l.facing);


  const int x0 = l.x + dir;


  const int top = l.y - (rules_.basherHeight - 1);


  bool anySolid = false, anySteel = false;
  for (int dy = 0; dy < rules_.basherHeight; ++dy)
    for (int dx = 0; dx < rules_.basherReach; ++dx) {
      const Material m = setup_.terrain.at(x0 + dx * dir, top + dy);
      if (m == Material::Steel)
        anySteel = true;
      else if (isSolid(m))
        anySolid = true;
    }

  if (anySteel) {
    turn(l);
    l.state = State::Walker;
    return;
  }
  // basher-fix-001: an empty leading column ends the bash only once the grace cycles are spent
  // (0x0040C6E8 / 0x0040C357 `+0x44 == 2 && +0x41 == 0`, see Rules::basherGraceCycles). Until then
  // the swing finds nothing and the lemming still advances a column per cycle. Before this the
  // first empty bite sent the lemming back to Walker in the tick it was assigned (playtest P1).
  if (!anySolid && l.bashCycles >= rules_.basherGraceCycles) {
    l.state = State::Walker;
    return;
  }

  if (anySolid)
    setup_.terrain.clearBox(dir > 0 ? x0 : x0 - rules_.basherReach + 1, top,
                            rules_.basherReach, rules_.basherHeight);
  l.x = setup_.terrain.wrapX(l.x + dir);
  if (l.bashCycles < rules_.basherGraceCycles)
    ++l.bashCycles;

  // 0x0040C6E8: off the ground after the advance -> the fall state. INFERENCE: FUN_0042E913 is the
  // walker's ground probe (the RuntimePick notes cite it as such) and false selects FUN_00415FA0,
  // the fall state; this also lets a lemming that swings off a ledge fall instead of hanging.
  if (!solidAt(l.x, l.y + 1)) {
    l.state = State::Faller;
    l.fallDistance = 0;
  }
}

void World::stepMiner(Lemming& l) {
  if (!bitesThisTick(l.frame, msPerTick(), rules_.minerCycleMs, rules_.minerBitePercent))
    return;
  const int dir = static_cast<int>(l.facing);





  for (int dx = 1; dx <= rules_.minerReach; ++dx)
    for (int arg = -1; arg <= 3; ++arg) {
      if (arg == 2 && dx == 1)
        continue;
      const int cx = l.x + dx * dir;
      const int cy = l.y - arg;
      if (setup_.terrain.at(cx, cy) == Material::Steel) {
        turn(l);
        l.state = State::Walker;
        return;
      }
    }

  for (int dx = 1; dx <= rules_.minerReach; ++dx)
    for (int arg = -1; arg <= 3; ++arg) {
      if (arg == 2 && dx == 1)
        continue;
      setup_.terrain.clearBox(l.x + dx * dir, l.y - arg, 1, 1);
    }



  l.x = setup_.terrain.wrapX(l.x + dir);
  l.y += 1;

  if (!solidAt(l.x, l.y + 1)) {
    l.state = State::Faller;
    l.fallDistance = 0;
  }
}

void World::stepDigger(Lemming& l) {
  if (!bitesThisTick(l.frame, msPerTick(), rules_.diggerCycleMs, rules_.diggerBitePercent))
    return;

  // Dig toward gravity: row = posY - vdir (0x00415AA0), Y step -= vdir
  // (0x00415DD2). Refusal mask 0x10 upright / 0x20 inverted (0x00415A86 /
  // 0x00415A96). MEASURED round 30: inverted cleared painted 0x11 with
  // Y += 1 per cycle in Y-up (= y -= 1 here) and refused painted 0x21.
  const int v = l.verticalDir >= 0 ? 1 : -1;
  const int digY = l.y + v;
  const int half = rules_.diggerWidth / 2;
  for (int dx = -half; dx <= half; ++dx) {
    const int cx = setup_.terrain.wrapX(l.x + dx);
    if (setup_.terrain.at(cx, digY) == Material::Steel ||
        setup_.terrain.digRefused(cx, digY, v)) {
      l.state = State::Walker;
      return;
    }
  }

  const std::size_t removed = setup_.terrain.clearBox(l.x - half, digY,
                                                      rules_.diggerWidth,
                                                      rules_.diggerDepth);
  l.y += v * rules_.diggerDepth;

  if (removed == 0 || !solidAt(l.x, l.y + v)) {
    l.state = State::Faller;
    l.fallDistance = 0;
  }
}

void World::step(Lemming& l) {
  if (!l.alive())
    return;

  if (l.nukeExploding) {
    // Nuke is a primary-state replacement (0x42E696), not a Bomber skill
    // assignment. Do not let exit/fluid contact replace it before removal.
    // frame -1 gives the same entry-only tick as an ordinary fuse expiring.
    if (l.frame >= 0) stepExploder(l);
    ++l.frame;
    return;
  }

  releaseTeleportContacts(setup_, l);







  if (l.fasterMs > 0) {
    l.fasterMs -= msPerTick();
    if (l.fasterMs < 0)
      l.fasterMs = 0;
  }

  if (l.fuseMs > 0) {
    l.fuseMs -= msPerTick();
    if (l.fuseMs <= 0) {






      l.fuseMs = 0;
      cue(SoundCue::OhNo);




      leaveBlocker(l);
      l.state = State::Exploder;
      l.frame = 0;
      return;
    }
  }

  if (l.state == State::Splatter) {
    if (++l.frame > 8)
      l.state = State::Dead;
    return;
  }

  if (l.state == State::Drown) {
    stepDrown(l);
    return;
  }

  if (attachFluidHazard(l))
    return;

  // Round 14/15: ground cell bit 0x40 attaches drown (side-effect 0x2f /
  // 0x00415386). Acid-as-hazard uses this same attach (round 15); type 0x2e
  // Fluid-match sibling never fired live. Checked before exit so a water
  // cell under the feet kills rather than saves when both overlap —
  // Exit_water / Exit_acid are separate AABBs (round 12); this is the
  // hazard bit alone.
  {
    const int v = l.verticalDir >= 0 ? 1 : -1;
    if (setup_.terrain.waterHazard(l.x, l.y + v)) {
      l.state = State::Drown;
      l.frame = 0;
      return;
    }
  }

  if (l.state != State::Blocker && atExit(l)) {
    l.state = State::Exited;
    ++saved_;
    cue(SoundCue::Exit);
    return;
  }

  // Round 16: teleport pad AABB (object 0x004B1250 / 0x00430BA1). Ahead of
  // the walk step so a lemming standing in the box enters Teleporting rather
  // than walking out. Mutate-alone: delete this arm and testTeleportPad fails.
  if (l.state == State::Walker) {
    if (const Teleport* pad = teleportAt(l)) {
      l.teleportPadsLatched.push_back(static_cast<std::size_t>(pad - setup_.teleports.data()));
      l.state = State::Teleporting;
      l.frame = 0;
      l.bricksLeft = pad->destX;
      l.fallDistance = pad->destY;
      return;
    }
  }

  // Round 17: SpeedPad AABB (0x004B1178 / FUN_00432742). Rising-edge latch
  // mirrors object+0xb8 — one FASTER attach per visit. Mutate-alone: delete
  // this arm and testSpeedPadFaster fails.
  if (l.state == State::Walker) {
    if (speedPadAt(l)) {
      if (!l.speedPadLatched) {
        l.fasterMs = rules_.fasterMs;
        l.speedPadLatched = true;
      }
    } else {
      l.speedPadLatched = false;
    }
  }

  // Round 21: Lever AABB (0x004B1288 / 0x00436D30). Facing-gated toggle of
  // Lever::state (object+0xb8); no lemming attach. Mutate-alone: delete this
  // arm and testLeverToggle fails.
  // shared-timing-impl-001: only the legacy tick() path evaluates contact here. The explicit
  // path evaluates it once per update AFTER all lemmings (stepLeverContacts), so the two
  // semantics never both run in one route.
  if (l.state == State::Walker && !explicitUpdate_) {
    if (Lever* lev = leverAt(l)) {
      // 0x00436E6A: +0xb8 := pull facing (±1).
      lev->state = static_cast<int>(l.facing);
    }
  }

  switch (l.state) {
    case State::Faller:
    case State::Floater: stepFaller(l); break;
    case State::Bounce: stepBounce(l); break;
    case State::Teleporting: stepTeleporting(l); break;
    case State::Walker:
      // 0x0042E2BF: while +0x50==2, run the main state twice per frame.
      stepWalker(l);
      if (l.fasterMs > 0 && l.state == State::Walker)
        stepWalker(l);
      break;
    case State::Climber: stepClimber(l); break;
    case State::Hoister: stepHoister(l); break;
    case State::Builder: stepBuilder(l); break;
    case State::Shrugger: stepShrugger(l); break;
    case State::Exploder: stepExploder(l); break;
    case State::Basher: stepBasher(l); break;
    case State::Miner: stepMiner(l); break;
    case State::Digger: stepDigger(l); break;
    case State::Blocker:




      // Shared native support 42E913 subtracts actor+44 in Y-up.
      if (!groundAt(l.x, l.y + l.verticalDir, l.verticalDir)) {
        leaveBlocker(l);
        l.state = State::Faller;
        l.fallDistance = 0;
      }
      break;
    default: break;
  }

  ++l.frame;
}

void World::stepRelease(double dt, bool tickBoundary, bool continueHatchesAfterTimeout) {
  if (nukeRequested_) return;

  // Round D1e: an additive, opt-in real-seconds release model
  // (docs/scratch/phase-d1-release-trigger.md and the two D1c/D1d follow-ups),
  // alongside the untouched integer-tick model it extends. A LevelSetup that
  // leaves releaseIntervalSeconds at its default 0.0 runs the exact old
  // branch, byte-for-byte.
  // D1n: the stepper clock's own dt is the same number the release model
  // already computes below -- shared via tickDtSeconds() so both read the
  // exact same value, per the brief's "Already settled" note.
  // shared-timing-impl-001: `dt` is now a parameter -- tickDtSeconds() from the legacy tick(),
  // the update's own segment dt from the explicit path -- so every elapsed-time consumer
  // below (arm clock, countdown, spawn phase) sees the same number as the steppers.

  if (!timedOut() || continueHatchesAfterTimeout) {
    if (setup_.releaseIntervalSeconds > 0.0) {
      releaseElapsedSeconds_ += dt;
      if (!releaseArmed_ && releaseElapsedSeconds_ >= setup_.releaseArmSeconds) {
        // Arm once, this tick: seed the countdown but do not also decrement
        // it the same tick it was seeded (matches the original's continuous
        // arm-then-seed instant; the decrement is "every step while armed",
        // starting from the next one).
        releaseArmed_ = true;
        releaseCountdown_ = setup_.releaseFirstCountdownSeconds;
      } else if (releaseArmed_) {
        // D1n: capture the countdown BEFORE the decrement -- if it crosses
        // zero this tick, it crossed `countdownBefore` seconds into this
        // tick's own dt window, not at the window's start, so only
        // `dt - countdownBefore` of real time remains in this tick for the
        // new lemming's first fall call (see World::spawn()).
        const double countdownBefore = releaseCountdown_;
        releaseCountdown_ -= dt;
        if (releaseCountdown_ <= 0.0) {
          const double spawnBudget = continuousMotion()
                                          ? std::max(0.0, dt - countdownBefore)
                                          : -1.0;
          spawn(spawnBudget);
          // Rollover, not a reset: D1b measured the recurring interval added
          // back into whatever the countdown currently holds.
          releaseCountdown_ += setup_.releaseIntervalSeconds;
        }
      }
    } else if (tickBoundary && ++sinceSpawn_ >= setup_.releaseIntervalTicks) {
      // The integer-tick release model counts whole level-clock ticks, never raw updates.
      sinceSpawn_ = 0;
      spawn();
    }
  }
}

void World::tick(bool continueHatchesAfterTimeout) {
  explicitUpdate_ = false;
  stepNuke(msPerTick(), true);
  stepRelease(tickDtSeconds(), true, continueHatchesAfterTimeout);

  for (std::size_t i = 0; i < lemmings_.size(); ++i)
    step(lemmings_[i]);

  ++tick_;
  levelClockMs_ += static_cast<std::uint64_t>(msPerTick());
  levelSecondEvents_ = levelClockMs_ / kLevelEventPeriodMs;
}

void World::stepElapsed(Lemming& l, bool tickBoundary) {
  if (tickBoundary) {
    step(l);
    return;
  }
  if (!l.alive())
    return;
  if (l.nukeExploding) return; // Bomber animation remains on the existing 5 Hz clock.
  releaseTeleportContacts(setup_, l);
  if (attachFluidHazard(l))
    return;
  // Between tick boundaries only the continuous movers advance (their budget carries the
  // sub-tick time); every other state, bookkeeping and counter keeps its measured 5 Hz step.
  switch (l.state) {
    case State::Faller:
    case State::Floater: stepFaller(l); break;
    case State::Walker:
      stepWalker(l);
      if (l.fasterMs > 0 && l.state == State::Walker)
        stepWalker(l);
      break;
    default: break;
  }
}

void World::stepLeverContacts(double dt) {
  // 0x4423D2 / lever step: `cooldown -= dt` while positive, then the contact test, then
  // activation only if the cooldown is <= 0 after that decrement (installed cooldown 0.0).
  // Contact takes the first accepted lemming in list order, so one lever fires at most once
  // per update no matter how many actors stand in its box (no duplicate dispatch).
  for (Lever& lev : setup_.levers) {
    if (lev.cooldownRemaining > 0.0)
      lev.cooldownRemaining -= dt;
    const Lemming* contact = nullptr;
    for (const Lemming& l : lemmings_) {
      // Same eligibility as the legacy arm: a live walker, then the AABB + facing filter.
      if (l.alive() && l.state == State::Walker && leverAccepts(lev, l)) {
        contact = &l;
        break;
      }
    }
    if (contact && lev.cooldownRemaining <= 0.0) {
      // 0x00436E6A: +0xb8 := pull facing, then the matching serialized list is sent.
      activateLeverFace(setup_, lev, static_cast<int>(contact->facing));
      lev.cooldownRemaining = lev.cooldownSeconds;
    }
  }
}

void World::updateSegment(int segmentMs, bool continueHatchesAfterTimeout, bool releaseEnabled) {
  const std::uint64_t period = static_cast<std::uint64_t>(std::max(1, msPerTick()));
  // The level clock is advanced before the update, as in the frame loop (0x45CFF9).
  levelClockMs_ += static_cast<std::uint64_t>(segmentMs);
  const bool tickBoundary = levelClockMs_ / period > tick_;
  const double dt = static_cast<double>(segmentMs) / 1000.0;
  explicitUpdate_ = true;
  updateDtSeconds_ = dt;

  stepNuke(segmentMs, tickBoundary);
  if (releaseEnabled) stepRelease(dt, tickBoundary, continueHatchesAfterTimeout);
  for (std::size_t i = 0; i < lemmings_.size(); ++i)
    stepElapsed(lemmings_[i], tickBoundary);
  stepLeverContacts(dt);
  // 0x4423D2 order: lemmings -> levers -> receivers (+0x1F70). A request written by a contact
  // above reaches its gate in this same update and its first raise step uses this segment's dt,
  // including a zero-length one. There is no separate gate clock.
  for (FluidGate& gate : setup_.gates)
    gate.step(dt, setup_.terrain);

  if (tickBoundary)
    tick_ = levelClockMs_ / period;
  ++explicitUpdates_;
  explicitUpdate_ = false;
  updateDtSeconds_ = 0.0;
}

void World::advanceElapsedMs(int dtMs, bool continueHatchesAfterTimeout,
                             const std::function<bool(double)>& afterUpdate) {
  advanceElapsedMsWithReleaseGate(dtMs, continueHatchesAfterTimeout, true, afterUpdate);
}

void World::advanceElapsedMsWithReleaseGate(int dtMs, bool continueHatchesAfterTimeout,
                             bool releaseEnabled, const std::function<bool(double)>& afterUpdate) {
  const std::uint64_t period = static_cast<std::uint64_t>(std::max(1, msPerTick()));
  int remaining = std::max(0, dtMs);
  bool open = true;
  // One update; reports false once the caller abandoned the pass.
  const auto run = [&](int ms) {
    if (!open)
      return;
    // A request that spans several tick boundaries first splits at them, so each update
    // reaches at most one boundary (the connected app never exceeds its 100 ms cap).
    while (((levelClockMs_ % period) + static_cast<std::uint64_t>(ms)) / period >= 2) {
      const int toBoundary = static_cast<int>(period - levelClockMs_ % period);
      updateSegment(toBoundary, continueHatchesAfterTimeout, releaseEnabled);
      ms -= toBoundary;
      if (afterUpdate && !afterUpdate(toBoundary / 1000.0)) {
        open = false;
        return;
      }
    }
    updateSegment(ms, continueHatchesAfterTimeout, releaseEnabled);
    if (afterUpdate && !afterUpdate(ms / 1000.0))
      open = false;
  };
  // Frame loop 0x45CFF9: advance to each due level-clock event, update, fire it, then the
  // trailing update with whatever is left (possibly zero).
  int toEvent = kLevelEventPeriodMs - static_cast<int>(levelClockMs_ % kLevelEventPeriodMs);
  while (open && toEvent <= remaining) {
    run(toEvent);
    remaining -= toEvent;
    ++levelSecondEvents_;
    toEvent = kLevelEventPeriodMs;
  }
  run(remaining);
}

bool World::assign(std::size_t index, Skill skill) {
  if (index >= lemmings_.size())
    return false;
  Lemming& l = lemmings_[index];
  if (!l.active())
    return false;

  int& stock = setup_.skills[static_cast<std::size_t>(skill)];
  if (stock == 0)
    return false;

  auto take = [&] {
    if (stock > 0)
      --stock;
    cue(SoundCue::Assign);
    return true;
  };

  switch (skill) {
    case Skill::Climber:
      if (l.canClimb)
        return false;
      l.canClimb = true;
      return take();
    case Skill::Floater:
      if (l.canFloat)
        return false;
      l.canFloat = true;
      return take();
    case Skill::Bomber:





      if (l.fuseMs >= 0)
        return false;
      l.fuseMs = rules_.bomberFuseMs;
      return take();
    default:
      break;
  }



  if (l.state != State::Walker && l.state != State::Builder && l.state != State::Basher &&
      l.state != State::Miner && l.state != State::Digger && l.state != State::Blocker)
    return false;




  leaveBlocker(l);

  switch (skill) {
    case Skill::Blocker:
      if (!groundAt(l.x, l.y + l.verticalDir, l.verticalDir)) {

        if (l.state == State::Blocker)
          stampBlockerField(l);
        return false;
      }
      l.state = State::Blocker;
      stampBlockerField(l);
      break;
    case Skill::Builder:
      l.state = State::Builder;
      l.bricksLeft = rules_.builderBricks;
      break;
    case Skill::Basher:
      l.state = State::Basher;
      l.bashCycles = 0;
      break;
    case Skill::Miner: l.state = State::Miner; break;
    case Skill::Digger: l.state = State::Digger; break;
    default: return false;
  }
  l.frame = 0;
  return take();
}

std::optional<std::size_t> World::pick(int x, int y, int radius) const {
  std::optional<std::size_t> best;
  int bestDist = radius * radius + 1;
  for (std::size_t i = 0; i < lemmings_.size(); ++i) {
    const Lemming& l = lemmings_[i];
    if (!l.active())
      continue;
    const int half = setup_.terrain.width() / 2;
    const int dx = setup_.terrain.wrapX(x - l.x + half) - half;
    const int dy = y - (l.y - rules_.lemmingHeight / 2);
    const int d = dx * dx + dy * dy;
    if (d < bestDist) {
      bestDist = d;
      best = i;
    }
  }
  return best;
}

void World::nuke() {
  if (nukeRequested_ || setup_.nukeStock <= 0) return;
  --setup_.nukeStock;
  nukeRequested_ = true;
  nukeCountdownMs_ = 5000; // Static 0x4423D2 -> 0x413F15: global five-second overlay.
  nukeCursor_ = 0;
  // PROVISIONAL playable-v1: stop releases immediately, including an empty
  // hatch. Original stops in the consumer after a non-type-3 actor exists.
}

void World::stepNuke(int elapsedMs, bool tickBoundary) {
  if (!nukeRequested_) return;
  if (nukeCountdownMs_ > 0) {
    nukeCountdownMs_ = std::max(0, nukeCountdownMs_ - elapsedMs);
    if (nukeCountdownMs_ > 0) return;
  }
  if (!tickBoundary) return;
  // Static 0x4423D2: oldest insertion first, primary replacement, no skill
  // eligibility veto. Stable vector indices skip actors already removed by
  // gameplay, avoiding the unresolved original cursor lifetime hazard.
  // PROVISIONAL stagger: one per existing 5 Hz tick (normally 200 ms).
  // Original adds 0.05 s credit, at most one attachment per update, with
  // credit accumulating during countdown. A native activation recording
  // with frame/update timestamps would settle its player-visible cadence.
  while (nukeCursor_ < lemmings_.size()) {
    auto& l = lemmings_[nukeCursor_++];
    if (!l.active()) continue; // Splatter is already counted lost at entry.
    if (l.state == State::Exploder) continue; // preserve an already running Bomber.
    leaveBlocker(l);
    l.teleportPadsLatched.clear();
    l.fuseMs = 0;
    l.nukeExploding = true;
    l.state = State::Exploder;
    l.frame = -1;
    cue(SoundCue::OhNo);
    break;
  }
}

void World::cue(SoundCue c) {



  if (c == SoundCue::OhNo || c == SoundCue::Explode) {
    if (std::find(cues_.begin(), cues_.end(), c) != cues_.end())
      return;
  }
  cues_.push_back(c);
}

std::vector<SoundCue> World::takeCues() {
  std::vector<SoundCue> out;
  out.swap(cues_);
  return out;
}

}
