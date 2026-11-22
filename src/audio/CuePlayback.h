// Routes simulation SoundCue values and serialized hazard samples through one Mixer.
// Callers resolve optional positions through the current scene and camera.
// Provisional: spatial gains, supplemental fluid and pad triggers, and UI click reuse.
// Missing devices or WAVs leave gameplay running. --no-audio disables routing.

#pragma once

#include "audio/Mixer.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <unordered_set>

namespace webo::sim {
enum class SoundCue : std::uint8_t;
class World;
}  // namespace webo::sim

namespace webo::audio {

struct CueRoute {
  sim::SoundCue cue;
  std::string_view name;      // retail cue name, resolved with soundAssetName()
  std::string_view evidence;  // original play site evidence
};

// The routed entries only, in table order.
std::span<const CueRoute> cueRoutes();

// Retail cue name for a sim cue, or nullopt when the cue is unrouted.
std::optional<std::string_view> routedCueName(sim::SoundCue cue);

// Sim cues with no established original play site stay silent.
std::span<const sim::SoundCue> unroutedCues();

// Receives every routed retail cue name even without an audio device.
using CueObserver = std::function<void(std::string_view)>;
using DiagnosticSink = std::function<void(const std::string&)>;

class CuePlayback {
public:
  CuePlayback();
  CuePlayback(const CuePlayback&) = delete;
  CuePlayback& operator=(const CuePlayback&) = delete;

  // --no-audio disables playback and the observer. World cues are still drained.
  void disable() { disabled_ = true; }
  bool disabled() const { return disabled_; }

  void setObserver(CueObserver observer) { observer_ = std::move(observer); }
  void setDiagnosticSink(DiagnosticSink sink) { diagnostic_ = std::move(sink); }

  // The first call opens the optional device and loads cue WAVs through the library overlay.
  // The diagnostic sink receives failures, which leave the adapter silent.
  void start(const AssetLibrary& library, bool openDevice);
  bool started() const { return started_; }
  bool deviceReady() const { return mixer_.ready(); }
  void setVolumes(float music, float sfx) {
    mixer_.setMusicVolume(music);
    mixer_.setSfxVolume(sfx);
  }

  // Routes one sim cue. Unrouted cues are counted and ignored.
  void play(sim::SoundCue cue);

  // Changeop selection feedback (0x45AD11). Also used provisionally for
  // pause, speed and hatch-rate controls; no looping or queued repeats.
  void playUiClick();

  // Serialized gadget sample, empty names intentionally silent (0x4347EC).
  void playGadgetSample(std::string_view name, StereoGains gains = {});

  // Frontend and map share lemming01. Each level load, including Retry, selects lemming02..13 (0x498DE8).
  // The independent RNG and default 30% music gain are provisional. Pause and focus preserve playback speed.
  // Music bypasses the SFX observer and counters.
  void playFrontendMusic();
  void prepareLevelMusic();  // Stop the previous bed and cache the selection before the entry clock starts.
  void playLevelMusic();

  // Takes every pending cue from the World and plays each in order.
  void drain(sim::World& world);
  using PositionResolver = std::function<StereoGains(const std::array<double, 3>&)>;
  void drain(sim::World& world, const PositionResolver& position);

  // Whether a routed cue would currently reach an observer or a device.
  bool active() const { return !disabled_ && (observer_ || mixer_.ready()); }

  std::size_t drained() const { return drained_; }  // every sim cue taken
  std::size_t routed() const { return routed_; }    // routed cues requested

private:
  friend struct WorkSoundTestAccess;
  friend struct PositionalSoundTestAccess;
  friend struct MusicTestAccess;
  void play(sim::SoundCue cue, StereoGains gains);
  void playName(std::string_view name, Mixer::Overlap overlap, StereoGains gains = {});
  void report(const std::string& message) const;
  void loadMusic(std::string_view name);
  void playMusic(std::string_view name, bool restart);

  Mixer mixer_;
  const AssetLibrary* library_ = nullptr; // caller owns it through playback lifetime
  std::unordered_set<std::string> attemptedSamples_;
  std::unordered_set<std::string> attemptedMusic_;
  std::string currentMusic_;
  std::string pendingLevelMusic_;
  CueObserver observer_;
  DiagnosticSink diagnostic_;
  bool disabled_ = false;
  bool started_ = false;
  std::size_t drained_ = 0;
  std::size_t routed_ = 0;
};

}  // namespace webo::audio
