#include "World.h"

#include <algorithm>
#include <cmath>

namespace webo::sim {

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
  if (spawned_ < setup_.totalLemmings)
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
  const Entrance& e = setup_.entrances[nextEntrance_ % setup_.entrances.size()];
  nextEntrance_ = (nextEntrance_ + 1) % setup_.entrances.size();

  Lemming l;
  l.x = setup_.terrain.wrapX(e.x);
  l.y = e.y;
  l.state = State::Faller;
  // Round 29: hatch+0xB0 -> [lemming+0x4c] at 0x0042E05F.
  l.sheetType = e.sheetType;
  l.facing = Facing::Right;
  // D1n: under the opt-in continuous stepper clock, a lemming released
  // partway through the tick that spawned it (continuous release model
  // only -- the caller passes a reduced budgetSeconds in that case) should
  // not get a full tick's worth of fall time before the next World::tick()
  // call. budgetSeconds < 0.0 (every integer-release-model call, and every
  // continuous-release call that happened to land exactly on a tick
  // boundary) means "the full tick," matching the old unconditional
  // behaviour.
  if (setup_.continuousStepperClock) {
    const double dt = tickDtSeconds();
    const double budget = budgetSeconds >= 0.0 ? budgetSeconds : dt;
    l.stepBudgetSeconds = budget - dt;
  }
  lemmings_.push_back(l);
  ++spawned_;
}
void World::stampBlockerField(const Lemming& l) {
  for (int k = 0; k < rules_.blockerFieldHeight; ++k)
    setup_.terrain.setBlockerField(l.x, l.y - k, true);
}
void World::clearBlockerField(const Lemming& l) {
  for (int k = 0; k < rules_.blockerFieldHeight; ++k)
    setup_.terrain.setBlockerField(l.x, l.y - k, false);
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
  for (const Teleport& t : setup_.teleports) {
    const int dx = setup_.terrain.wrapX(l.x - t.x + setup_.terrain.width() / 2) -
                   setup_.terrain.width() / 2;
    const int dy = l.y - t.y;
    if (dx >= 0 && dx <= t.width && dy >= 0 && dy <= t.height)
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

Lever* World::leverAt(const Lemming& l) {
  // 0x00436D30: AABB then facing vs +0xb8. state==1 needs face −1; else +1.
  const int face = static_cast<int>(l.facing);
  for (Lever& lev : setup_.levers) {
    const int dx =
        setup_.terrain.wrapX(l.x - lev.x + setup_.terrain.width() / 2) -
        setup_.terrain.width() / 2;
    const int dy = l.y - lev.y;
    if (dx < 0 || dx > lev.width || dy < 0 || dy > lev.height)
      continue;
    if (lev.state == 1) {
      if (face != -1)
        continue;
    } else if (face != 1) {
      continue;
    }
    return &lev;
  }
  return nullptr;
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
  l.state = State::Walker;
  l.frame = 0;
  l.bricksLeft = 0;
  l.fallDistance = 0;
}
void World::explode(Lemming& l) {
  setup_.terrain.clearExplosion(l.x, l.y);
  l.state = State::Dead;
  ++dead_;
  cue(SoundCue::Explode);
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

  const bool continuous = setup_.continuousStepperClock && speed > 0;
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
  const double dt = continuous ? tickDtSeconds() : 0.0;
  const double unitSeconds = continuous ? dt / speed : 0.0;
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
    l.y += v;
    ++l.fallDistance;

    // Round 14: 0x2f probes the ground cell every sub-step. Live paint of
    // 0x40 on MM_CLIMB_002 attached drown mid-fall when under=(col,floor)
    // before the walk land (`measure_water.py`). Ahead of trampoline/splat.
    if (setup_.terrain.waterHazard(l.x, l.y + v)) {
      l.state = State::Drown;
      l.frame = 0;
      return;
    }
    if (solidAt(l.x, l.y + v)) {

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
  const bool continuous = setup_.continuousStepperClock && rules_.walkSpeed > 0;
  const double dt = continuous ? tickDtSeconds() : 0.0;
  const double unitSeconds = continuous ? dt / rules_.walkSpeed : 0.0;
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
      if (flipIfAntiGrav())
        return;
      continue;
    }
    l.x = setup_.terrain.wrapX(nx);
    if (flipIfAntiGrav())
      return;



    if (solidAt(l.x, l.y + v))
      continue;
    if (solidAt(l.x, l.y + 2 * v)) {
      l.y += v;
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
  if (!anySolid) {
    l.state = State::Walker;
    return;
  }

  setup_.terrain.clearBox(dir > 0 ? x0 : x0 - rules_.basherReach + 1, top,
                          rules_.basherReach, rules_.basherHeight);
  l.x = setup_.terrain.wrapX(l.x + dir);
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
  if (l.state == State::Walker) {
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




      if (!solidAt(l.x, l.y + 1)) {
        leaveBlocker(l);
        l.state = State::Faller;
        l.fallDistance = 0;
      }
      break;
    default: break;
  }

  ++l.frame;
}

void World::tick(bool continueHatchesAfterTimeout) {

  // Round D1e: an additive, opt-in real-seconds release model
  // (docs/scratch/phase-d1-release-trigger.md and the two D1c/D1d follow-ups),
  // alongside the untouched integer-tick model it extends. A LevelSetup that
  // leaves releaseIntervalSeconds at its default 0.0 runs the exact old
  // branch, byte-for-byte.
  // D1n: the stepper clock's own dt is the same number the release model
  // already computes below -- shared via tickDtSeconds() so both read the
  // exact same value, per the brief's "Already settled" note.
  const double dt = tickDtSeconds();

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
          const double spawnBudget = setup_.continuousStepperClock
                                          ? std::max(0.0, dt - countdownBefore)
                                          : -1.0;
          spawn(spawnBudget);
          // Rollover, not a reset: D1b measured the recurring interval added
          // back into whatever the countdown currently holds.
          releaseCountdown_ += setup_.releaseIntervalSeconds;
        }
      }
    } else if (++sinceSpawn_ >= setup_.releaseIntervalTicks) {
      sinceSpawn_ = 0;
      spawn();
    }
  }

  for (std::size_t i = 0; i < lemmings_.size(); ++i)
    step(lemmings_[i]);

  ++tick_;
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
      if (!solidAt(l.x, l.y + 1)) {

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
    case Skill::Basher: l.state = State::Basher; break;
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
  for (auto& l : lemmings_)
    if (l.active() && l.fuseMs < 0)
      l.fuseMs = rules_.bomberFuseMs;


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
