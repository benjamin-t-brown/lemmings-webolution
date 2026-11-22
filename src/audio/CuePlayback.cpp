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

// Retail cue table for exe SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f.
// Cue strings are packed at 0x004C0C7C (docs/lessons/hud-and-panel.md).
constexpr CueRoute kRoutes[] = {
    {sim::SoundCue::Assign, "Action",
     "assignment 0x00455657 tests the candidate "
     "and acceptance, loads Action handle 0x004E32FC at 0x004556AA and plays "
     "at 0x004556B5; refused assignments bypass this call"},
    {sim::SoundCue::OhNo, "Ohno",
     "helper step 0x00414370 "
     "tests [0x004CBB40] at 0x00414420 and plays Ohno at 0x00414443 if zero; "
     "the tested byte is "
     "level+0x5138, the nuke request; ordinary bombs play at fuse end"},
    {sim::SoundCue::Explode, "Explode",
     "0x0045E570..0x0045E582 registers Explode "
     "as 0x004E32C0; helper 0x004081FC loads it at 0x00408205 and plays at "
     "0x0040821C; provisional clone trigger is the existing completed blast cue"},
    // Fall deaths play FallSplat. The retail Splat cue (DAT_004E32D4) has no play references.
    {sim::SoundCue::Splat, "FallSplat",
     "FallSplat (DAT_004E32E0) is played "
     "once, at 0x0040B4B6 inside splat enter FUN_0040b3ec (type 0x1a) after a "
     "long fall; Splat (DAT_004E32D4) has 0 play references"},
    {sim::SoundCue::Exit, "Yippee",
     "0x0045E540..0x0045E552 loads Yippee "
     "into level+0x1C8B0 (=0x004E32B8); save step 0x0041818A increments saved "
     "at 0x004182A1, then plays that handle at 0x004182E5 via 0x00445274"},
    {sim::SoundCue::LevelStart, "Letsgo",
     "hatch step 0x00425A95 tests its "
     "+0xAE latch, plays Letsgo handle 0x004E32B4 at 0x00425ADB before hatch "
     "opening/release; player-requested "
     "clone point is the first drawn Let's Go text frame"},
    {sim::SoundCue::Work, "Mine",
     "destructible-cell arm 0x0040C0AF "
     "loads Mine handle 0x004E32E8 at 0x0040C317 and plays at 0x0040C32F; "
     "miner success plays at 0x00418D39, "
     "digger success at 0x00415D81; provisional once per successful hybrid stroke"},
    {sim::SoundCue::BuilderWarning, "Chink",
     "0x0040FF54 tests bricks < 4, "
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
  // Provisional overlap: average exit cheers and suppress overlapping assignment clips.
  // Restart work, warning and Explode clips on each event. Requests are never queued.
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
  pendingLevelMusic_.clear();
  playMusic("lemming01", false);
}

void CuePlayback::prepareLevelMusic() {
  if (disabled_ || !mixer_.ready() || !library_) return;
  // 0x498E2F stops and releases the previous music buffer before loading the next one.
  mixer_.stopMusic();
  currentMusic_.clear();
  // Native 0x498E13..29 indexes the table with trunc(rand()/32767 * 12 + 1).
  // The clone uses a bounded uniform draw independent of the world RNG.
  static std::mt19937 random(static_cast<std::mt19937::result_type>(
      std::chrono::steady_clock::now().time_since_epoch().count()));
  const int track = std::uniform_int_distribution<int>(2, 13)(random);
  pendingLevelMusic_ = std::format("lemming{:02}", track);
  loadMusic(pendingLevelMusic_);
}

void CuePlayback::playLevelMusic() {
  if (disabled_ || pendingLevelMusic_.empty()) return;
  mixer_.playMusic(pendingLevelMusic_);
  currentMusic_ = std::exchange(pendingLevelMusic_, {});
}

void CuePlayback::loadMusic(std::string_view name) {
  // Cache load and resample results, including failures, between scenes outside the callback lock.
  if (attemptedMusic_.insert(std::string(name)).second) {
    if (auto loaded = mixer_.loadMusicFrom(*library_, name); !loaded)
      report(std::format("audio: music {} unavailable ({}); it will be silent", name,
                         loaded.error()));
  }
}

void CuePlayback::playMusic(std::string_view name, bool restart) {
  if (disabled_ || !mixer_.ready() || !library_) return;
  if (!restart && currentMusic_ == name) return;
  loadMusic(name);
  mixer_.playMusic(name); // Preserve active SFX tails.
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
