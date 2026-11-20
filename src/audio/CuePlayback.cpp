#include "audio/CuePlayback.h"

#include "assets/Box.h"
#include "sim/World.h"

#include <array>
#include <chrono>
#include <cstdio>
#include <format>
#include <iterator>
#include <random>

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
    {sim::SoundCue::Explode, "Explode",
     "docs/scratch/work-sounds-001.md: 0x0045E570..0x0045E582 registers Explode "
     "as 0x004E32C0; helper 0x004081FC loads it at 0x00408205 and plays at "
     "0x0040821C; provisional clone trigger is the existing completed blast cue"},
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
     "opening/release; docs/scratch/letsgo-voice-with-text-001.md: player-requested "
     "clone point is the first drawn Let's Go text frame (playtest 11 A12)"},
    {sim::SoundCue::Work, "Mine",
     "docs/scratch/gameplay-sounds-001.md: destructible-cell arm 0x0040C0AF "
     "loads Mine handle 0x004E32E8 at 0x0040C317 and plays at 0x0040C32F; "
     "docs/scratch/work-sounds-001.md: miner success plays at 0x00418D39, "
     "digger success at 0x00415D81; provisional once per successful hybrid stroke"},
    {sim::SoundCue::BuilderWarning, "Chink",
     "docs/scratch/gameplay-sounds-001.md: 0x0040FF54 tests bricks < 4, "
     "0x0040FF5C loads Chink handle 0x004E32BC, 0x0040FF73 plays it before "
     "the brick counter decrement at 0x0040FF93"},
};

constexpr std::array<sim::SoundCue, 0> kUnrouted{};

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

CuePlayback::CuePlayback() { mixer_.setMusicVolume(0.3f); }

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
  library_ = &library;
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
  play(cue, {});
}

void CuePlayback::play(sim::SoundCue cue, StereoGains gains) {
  ++drained_;
  if (disabled_)
    return;
  const auto name = routedCueName(cue);
  if (!name)
    return;
  // Provisional overlap: exit cheers average; assignment clips finish before
  // another can start. Each successful work stroke and warning starts afresh,
  // replacing the previous tail rather than dropping fast-forward/worker hits.
  // Explode also restarts: at most one explosion voice, with a fresh attack
  // for each blast and no summed explosion tails in a nuke.
  // None queues requests during fast-forward/group arrivals.
  const auto overlap = cue == sim::SoundCue::Exit ? Mixer::Overlap::Average :
      cue == sim::SoundCue::Assign ? Mixer::Overlap::SkipPlaying :
      (cue == sim::SoundCue::Work || cue == sim::SoundCue::BuilderWarning ||
       cue == sim::SoundCue::Explode) ?
          Mixer::Overlap::RestartPlaying : Mixer::Overlap::Add;
  playName(*name, overlap, gains);
}

void CuePlayback::playUiClick() {
  ++drained_;
  if (!disabled_) playName("Changeop", Mixer::Overlap::SkipPlaying);
}

void CuePlayback::playGadgetSample(std::string_view name, StereoGains gains) {
  if (name.empty() || disabled_) return;
  ++drained_;
  playName(name, Mixer::Overlap::SkipPlaying, gains);
}

void CuePlayback::playFrontendMusic() {
  playMusic("lemming01", false);
}

void CuePlayback::playLevelMusic() {
  if (disabled_ || !mixer_.ready()) return;
  // Native 0x498E13..29 uses trunc(rand()/32767 * 12 + 1) as a
  // zero-based table index. Use an independent, bounded uniform draw here;
  // do not reproduce its rare out-of-table endpoint or consume world RNG.
  static std::mt19937 random(static_cast<std::mt19937::result_type>(
      std::chrono::steady_clock::now().time_since_epoch().count()));
  const int track = std::uniform_int_distribution<int>(2, 13)(random);
  playMusic(std::format("lemming{:02}", track), true);
}

void CuePlayback::playMusic(std::string_view name, bool restart) {
  if (disabled_ || !mixer_.ready() || !library_) return;
  if (!restart && currentMusic_ == name) return;
  // Loading/resampling is outside the callback lock, between scenes, never
  // in a sim update. Cache successes and failures across every app leg.
  if (attemptedMusic_.insert(std::string(name)).second) {
    if (auto loaded = mixer_.loadMusicFrom(*library_, name); !loaded)
      report(std::format("audio: music {} unavailable ({}); it will be silent", name,
                         loaded.error()));
  }
  mixer_.playMusic(name); // replaces only music, preserving every SFX tail
  currentMusic_ = name;
}

void CuePlayback::playName(std::string_view name, Mixer::Overlap overlap, StereoGains gains) {
  ++routed_;
  if (observer_) observer_(name);
  if (mixer_.ready()) {
    // Object samples load once on first use. Remember failures too.
    if (!mixer_.has(name) && library_ && attemptedSamples_.insert(std::string(name)).second) {
      if (auto loaded = mixer_.loadFrom(*library_, name); !loaded)
        report(std::format("audio: cue {} unavailable ({}); it will be silent", name, loaded.error()));
    }
    if (mixer_.has(name)) mixer_.play(name, 1.0f, overlap, gains);
  }
}

void CuePlayback::drain(sim::World& world) {
  drain(world, {});
}

void CuePlayback::drain(sim::World& world, const PositionResolver& position) {
  for (const sim::SoundEvent& event : world.takeSoundEvents()) {
    const auto gains = event.position && position ? position(*event.position) : StereoGains{};
    if (event.cue) {
      play(*event.cue, (event.cue == sim::SoundCue::LevelStart || event.cue == sim::SoundCue::Assign)
                         ? StereoGains{} : gains);
    } else {
      ++drained_;
      if (!disabled_) {
        const auto overlap = event.sample == "AntiGrav" ?
            Mixer::Overlap::AddWithoutEviction : Mixer::Overlap::Add;
        playName(event.sample, overlap, gains);
      }
    }
  }
}

}  // namespace webo::audio
