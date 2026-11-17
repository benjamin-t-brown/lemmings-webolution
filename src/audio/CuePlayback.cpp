#include "audio/CuePlayback.h"

#include "assets/Box.h"
#include "sim/World.h"

#include <cstdio>
#include <format>
#include <iterator>

namespace webo::audio {
namespace {

// The one cue table. Every routed entry carries the retained static read that
// justifies it (exe SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f).
// Cue strings are packed at 0x004C0C7C (docs/lessons/hud-and-panel.md).
constexpr CueRoute kRoutes[] = {
    {sim::SoundCue::Assign, "Action",
     "docs/scratch/ui-sounds-001.md: assignment 0x00455657 tests the candidate "
     "and acceptance, loads Action handle 0x004E32FC at 0x004556AA and plays "
     "at 0x004556B5; refused assignments bypass this call"},
    {sim::SoundCue::OhNo, "Ohno",
     "old/docs/scratch/phase3-round7.md 'When Ohno plays': helper step 0x00414370 "
     "tests [0x004CBB40] at 0x00414420 and plays Ohno at 0x00414443 if zero; "
     "docs/scratch/letsgo-text-nuke-ohno-001.md identifies that byte as "
     "level+0x5138, the nuke request; ordinary bombs play at fuse end"},
    // The sim names this moment Splat, but the retail `Splat` cue (DAT_004E32D4)
    // has zero play references. The fall death plays FallSplat.
    {sim::SoundCue::Splat, "FallSplat",
     "old/docs/scratch/phase3-round28-die.md: FallSplat (DAT_004E32E0) is played "
     "once, at 0x0040B4B6 inside splat enter FUN_0040b3ec (type 0x1a) after a "
     "long fall; Splat (DAT_004E32D4) has 0 play references"},
    {sim::SoundCue::Exit, "Yippee",
     "docs/scratch/exit-yippee-sound-001.md: 0x0045E540..0x0045E552 loads Yippee "
     "into level+0x1C8B0 (=0x004E32B8); save step 0x0041818A increments saved "
     "at 0x004182A1, then plays that handle at 0x004182E5 via 0x00445274"},
    {sim::SoundCue::LevelStart, "Letsgo",
     "docs/scratch/gameplay-sounds-001.md: hatch step 0x00425A95 tests its "
     "+0xAE latch, plays Letsgo handle 0x004E32B4 at 0x00425ADB before hatch "
     "opening/release; player-requested clone point: first hatch spawn"},
    {sim::SoundCue::Bash, "Mine",
     "docs/scratch/gameplay-sounds-001.md: destructible-cell arm 0x0040C0AF "
     "loads Mine handle 0x004E32E8 at 0x0040C317 and plays at 0x0040C32F; "
     "provisional clone cadence: once per successful hybrid bash bite"},
    {sim::SoundCue::BuilderWarning, "Chink",
     "docs/scratch/gameplay-sounds-001.md: 0x0040FF54 tests bricks < 4, "
     "0x0040FF5C loads Chink handle 0x004E32BC, 0x0040FF73 plays it before "
     "the brick counter decrement at 0x0040FF93"},
};

// Chink belongs to the last three builder bricks; assignment uses Action.
// Explode remains outside the routed scope.
constexpr sim::SoundCue kUnrouted[] = {
    sim::SoundCue::Explode,  // Explode -- no retained play site
};

static_assert(std::size(kRoutes) + std::size(kUnrouted) ==
                  static_cast<std::size_t>(sim::SoundCue::BuilderWarning) + 1,
              "every SoundCue is either routed with evidence or listed as unrouted");

}  // namespace

std::span<const CueRoute> cueRoutes() { return kRoutes; }

std::optional<std::string_view> routedCueName(sim::SoundCue cue) {
  for (const CueRoute& route : kRoutes)
    if (route.cue == cue)
      return route.name;
  return std::nullopt;
}

std::span<const sim::SoundCue> unroutedCues() { return kUnrouted; }

void CuePlayback::report(const std::string& message) const {
  if (diagnostic_)
    diagnostic_(message);
  else
    std::fprintf(stderr, "%s\n", message.c_str());
}

void CuePlayback::start(const AssetLibrary& library, bool openDevice) {
  if (disabled_ || started_)
    return;
  started_ = true;
  if (!openDevice)
    return;

  if (auto opened = mixer_.open(); !opened) {
    report(std::format("audio: {}; continuing silently", opened.error()));
    return;
  }
  for (const CueRoute& route : kRoutes) {
    if (auto loaded = mixer_.loadFrom(library, route.name); !loaded)
      report(std::format("audio: cue {} unavailable ({}); it will be silent", route.name,
                         loaded.error()));
  }
  if (auto loaded = mixer_.loadFrom(library, "Changeop"); !loaded)
    report(std::format("audio: cue Changeop unavailable ({}); it will be silent", loaded.error()));
}

void CuePlayback::play(sim::SoundCue cue) {
  ++drained_;
  if (disabled_)
    return;
  const auto name = routedCueName(cue);
  if (!name)
    return;
  // Provisional overlap: exit cheers average; successful bash/assignment
  // clips finish before another of the same sample can start; each warning
  // starts afresh. None queues requests during fast-forward/group arrivals.
  const auto overlap = cue == sim::SoundCue::Exit ? Mixer::Overlap::Average :
      (cue == sim::SoundCue::Bash || cue == sim::SoundCue::Assign) ? Mixer::Overlap::SkipPlaying :
      cue == sim::SoundCue::BuilderWarning ? Mixer::Overlap::RestartPlaying : Mixer::Overlap::Add;
  playName(*name, overlap);
}

void CuePlayback::playUiClick() {
  ++drained_;
  if (!disabled_) playName("Changeop", Mixer::Overlap::SkipPlaying);
}

void CuePlayback::playName(std::string_view name, Mixer::Overlap overlap) {
  ++routed_;
  if (observer_) observer_(name);
  if (mixer_.ready() && mixer_.has(name)) mixer_.play(name, 1.0f, overlap);
}

void CuePlayback::drain(sim::World& world) {
  for (const sim::SoundCue cue : world.takeCues())
    play(cue);
}

}  // namespace webo::audio
