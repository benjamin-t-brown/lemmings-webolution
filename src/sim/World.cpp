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
    case State::Basher: return "basher";
    case State::Miner: return "miner";
    case State::Digger: return "digger";
    case State::Exploder: return "exploder";
    case State::Splatter: return "splatter";
    case State::Exited: return "exited";
    case State::Dead: return "dead";
  }
  return "?";
}

World::World(LevelSetup setup) : World(std::move(setup), Rules{}) {}

World::World(LevelSetup setup, Rules rules) : setup_(std::move(setup)), rules_(rules) {
  lemmings_.reserve(static_cast<std::size_t>(std::max(0, setup_.totalLemmings)));
  // Release the first lemming on the first tick rather than making the player
  // wait a full interval for anything to happen.
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

void World::spawn() {
  if (spawned_ >= setup_.totalLemmings || setup_.entrances.empty())
    return;
  const Entrance& e = setup_.entrances[nextEntrance_ % setup_.entrances.size()];
  nextEntrance_ = (nextEntrance_ + 1) % setup_.entrances.size();

  Lemming l;
  l.x = setup_.terrain.wrapX(e.x);
  l.y = e.y;
  l.state = State::Faller;
  l.facing = Facing::Right;
  lemmings_.push_back(l);
  ++spawned_;
}

bool World::blockedBy(const Lemming& l, std::size_t self) const {
  for (std::size_t i = 0; i < lemmings_.size(); ++i) {
    if (i == self)
      continue;
    const Lemming& b = lemmings_[i];
    if (b.state != State::Blocker)
      continue;
    // A blocker stops anyone who walks into the few pixels either side of it.
    const int dx = setup_.terrain.wrapX(l.x - b.x + setup_.terrain.width() / 2) -
                   setup_.terrain.width() / 2;
    if (std::abs(dx) <= 3 && std::abs(l.y - b.y) < rules_.lemmingHeight)
      return true;
  }
  return false;
}

bool World::atExit(const Lemming& l) const {
  for (const Exit& e : setup_.exits) {
    const int dx = setup_.terrain.wrapX(l.x - e.x + setup_.terrain.width() / 2) -
                   setup_.terrain.width() / 2;
    if (dx >= 0 && dx < e.width && l.y >= e.y && l.y < e.y + e.height)
      return true;
  }
  return false;
}

void World::explode(Lemming& l) {
  setup_.terrain.clearCircle(l.x, l.y - rules_.lemmingHeight / 2, rules_.explosionRadius);
  l.state = State::Dead;
  ++dead_;
  cue(SoundCue::Explode);
}

void World::stepFaller(Lemming& l) {
  const bool floating = l.canFloat && l.fallDistance >= rules_.floatAfter;
  if (floating)
    l.state = State::Floater;

  const int speed = floating ? rules_.floatSpeed : rules_.fallSpeed;
  for (int i = 0; i < speed; ++i) {
    if (solidAt(l.x, l.y + 1)) {
      // Landed.
      if (!floating && l.fallDistance > rules_.splatDistance) {
        l.state = State::Splatter;
        ++dead_;
        cue(SoundCue::Splat);
        return;
      }
      l.state = State::Walker;
      l.fallDistance = 0;
      l.frame = 0;
      return;
    }
    ++l.y;
    ++l.fallDistance;
  }
}

void World::stepWalker(Lemming& l, std::size_t index) {
  if (blockedBy(l, index)) {
    turn(l);
    return;
  }

  const int dir = static_cast<int>(l.facing);
  for (int s = 0; s < rules_.walkSpeed; ++s) {
    const int nx = l.x + dir;

    // Walk up a step if there is one and it is low enough.
    int rise = 0;
    while (rise <= rules_.maxStepUp && solidAt(nx, l.y - rise))
      ++rise;

    if (rise > rules_.maxStepUp) {
      // A wall. Climbers go up it, everyone else turns around.
      if (l.canClimb) {
        l.state = State::Climber;
        l.frame = 0;
        return;
      }
      turn(l);
      return;
    }

    l.x = setup_.terrain.wrapX(nx);
    l.y -= rise;

    // Walk off an edge?
    if (!solidAt(l.x, l.y + 1)) {
      l.state = State::Faller;
      l.fallDistance = 0;
      return;
    }
  }
}

void World::stepClimber(Lemming& l) {
  const int dir = static_cast<int>(l.facing);
  // Reached the top of the wall? Pull up.
  if (!solidAt(l.x + dir, l.y - rules_.lemmingHeight)) {
    l.state = State::Hoister;
    l.frame = 0;
    return;
  }
  // Ceiling: climbers that hit one drop back off the wall.
  if (solidAt(l.x, l.y - rules_.lemmingHeight)) {
    turn(l);
    l.state = State::Faller;
    l.fallDistance = 0;
    return;
  }
  l.y -= rules_.climbSpeed;
}

void World::stepHoister(Lemming& l) {
  // Two ticks of pulling over the lip, then walk on.
  if (l.frame >= 2) {
    const int dir = static_cast<int>(l.facing);
    l.x = setup_.terrain.wrapX(l.x + dir);
    l.y -= rules_.lemmingHeight;
    l.state = State::Walker;
    l.frame = 0;
  }
}

void World::stepBuilder(Lemming& l) {
  if (l.frame % rules_.builderInterval != 0)
    return;
  if (l.bricksLeft <= 0) {
    l.state = State::Walker;
    return;
  }

  const int dir = static_cast<int>(l.facing);
  // A brick is laid at the feet, then the lemming steps up onto it.
  const int bx = dir > 0 ? l.x : l.x - rules_.brickLength + 1;
  setup_.terrain.fillBox(bx, l.y, rules_.brickLength, 1, Material::Built);
  --l.bricksLeft;

  l.x = setup_.terrain.wrapX(l.x + dir * rules_.brickLength / 2);
  l.y -= 1;

  // Head against something: stop building and turn.
  if (solidAt(l.x, l.y - rules_.lemmingHeight)) {
    turn(l);
    l.state = State::Walker;
  }
}

void World::stepBasher(Lemming& l) {
  if (l.frame % rules_.basherInterval != 0)
    return;
  const int dir = static_cast<int>(l.facing);
  const int x0 = dir > 0 ? l.x + 1 : l.x - rules_.basherReach;

  // Nothing left to bash, or steel in the way: back to walking.
  bool anySolid = false, anySteel = false;
  for (int dy = 1; dy <= rules_.lemmingHeight; ++dy)
    for (int dx = 0; dx < rules_.basherReach; ++dx) {
      const Material m = setup_.terrain.at(x0 + dx, l.y - dy);
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

  setup_.terrain.clearBox(x0, l.y - rules_.lemmingHeight, rules_.basherReach,
                          rules_.lemmingHeight);
  l.x = setup_.terrain.wrapX(l.x + dir);
}

void World::stepMiner(Lemming& l) {
  if (l.frame % rules_.minerInterval != 0)
    return;
  const int dir = static_cast<int>(l.facing);
  const int x0 = dir > 0 ? l.x : l.x - rules_.basherReach + 1;

  for (int dx = 0; dx < rules_.basherReach; ++dx)
    if (setup_.terrain.at(x0 + dx, l.y) == Material::Steel) {
      turn(l);
      l.state = State::Walker;
      return;
    }

  setup_.terrain.clearBox(x0, l.y - rules_.lemmingHeight / 2, rules_.basherReach,
                          rules_.lemmingHeight / 2 + 1);
  l.x = setup_.terrain.wrapX(l.x + dir);
  l.y += 1;

  if (!solidAt(l.x, l.y + 1)) {
    l.state = State::Faller;
    l.fallDistance = 0;
  }
}

void World::stepDigger(Lemming& l) {
  if (l.frame % rules_.diggerInterval != 0)
    return;
  for (int dx = -2; dx <= 2; ++dx)
    if (setup_.terrain.at(l.x + dx, l.y + 1) == Material::Steel) {
      l.state = State::Walker;
      return;
    }

  const std::size_t removed = setup_.terrain.clearBox(l.x - 2, l.y + 1, 5, rules_.diggerDepth);
  l.y += rules_.diggerDepth;

  // Dug through into open air.
  if (removed == 0 || !solidAt(l.x, l.y + 1)) {
    l.state = State::Faller;
    l.fallDistance = 0;
  }
}

void World::step(Lemming& l, std::size_t index) {
  if (!l.alive())
    return;

  // The fuse runs regardless of what the lemming is doing.
  if (l.fuse > 0) {
    --l.fuse;
    // Classic plays "oh no" when the countdown reaches the panic beat, not
    // when the skill is assigned. One second of fuse left is the audible
    // moment here; the exact tick is provisional with the rest of the rules
    // (bomberFuse is written as 5 * 20 for the same reason).
    if (l.fuse == 20)
      cue(SoundCue::OhNo);
    if (l.fuse == 0) {
      explode(l);
      return;
    }
  }

  if (l.state == State::Splatter) {
    if (++l.frame > 8)
      l.state = State::Dead;
    return;
  }

  // Reaching the exit ends the lemming's run, whatever it was doing, except
  // that a blocker stays put.
  if (l.state != State::Blocker && atExit(l)) {
    l.state = State::Exited;
    ++saved_;
    cue(SoundCue::Exit);
    return;
  }

  switch (l.state) {
    case State::Faller:
    case State::Floater: stepFaller(l); break;
    case State::Walker: stepWalker(l, index); break;
    case State::Climber: stepClimber(l); break;
    case State::Hoister: stepHoister(l); break;
    case State::Builder: stepBuilder(l); break;
    case State::Basher: stepBasher(l); break;
    case State::Miner: stepMiner(l); break;
    case State::Digger: stepDigger(l); break;
    case State::Blocker:
      // A blocker that loses the ground under it falls and reverts.
      if (!solidAt(l.x, l.y + 1)) {
        l.state = State::Faller;
        l.fallDistance = 0;
      }
      break;
    default: break;
  }

  ++l.frame;
}

void World::tick() {
  // Once the clock runs out the entrance stops. The lemmings already out keep
  // going until they exit or die; deciding that the *level* is over belongs to
  // the game shell, not here, so the simulation only closes the entrance.
  if (!timedOut() && ++sinceSpawn_ >= setup_.releaseIntervalTicks) {
    sinceSpawn_ = 0;
    spawn();
  }

  for (std::size_t i = 0; i < lemmings_.size(); ++i)
    step(lemmings_[i], i);

  ++tick_;
}

bool World::assign(std::size_t index, Skill skill) {
  if (index >= lemmings_.size())
    return false;
  Lemming& l = lemmings_[index];
  if (!l.active())
    return false;

  switch (skill) {
    case Skill::Climber:
      if (l.canClimb)
        return false;
      l.canClimb = true;
      cue(SoundCue::Assign);
      return true;
    case Skill::Floater:
      if (l.canFloat)
        return false;
      l.canFloat = true;
      cue(SoundCue::Assign);
      return true;
    case Skill::Bomber:
      if (l.fuse >= 0)
        return false;
      l.fuse = rules_.bomberFuse;
      cue(SoundCue::Assign);
      return true;
    default:
      break;
  }

  // The rest replace what the lemming is doing, and only from states that can
  // be interrupted.
  if (l.state != State::Walker && l.state != State::Builder && l.state != State::Basher &&
      l.state != State::Miner && l.state != State::Digger && l.state != State::Blocker)
    return false;

  switch (skill) {
    case Skill::Blocker:
      if (!solidAt(l.x, l.y + 1))
        return false;
      l.state = State::Blocker;
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
  cue(SoundCue::Assign);
  return true;
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
    if (l.active() && l.fuse < 0)
      l.fuse = rules_.bomberFuse;
  // OhNo fires from step() when each fuse reaches the panic beat, so a nuke
  // does not need a separate cue here.
}

void World::cue(SoundCue c) {
  // A nuke lights every fuse on the same tick, and they all panic / explode
  // together. One sample of each is what you hear in the original; stacking
  // N identical voices would be louder, not more informative.
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

}  // namespace webo::sim
