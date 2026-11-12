// Maps the simulation's SoundCue values onto the retail cue names and plays
// them through one Mixer.
//
// The simulation only says what happened (sim::SoundCue); this adapter owns the
// single table that decides which retail cue, if any, each of those moments is
// allowed to play. An entry is routed ONLY when a retained static play-site
// read exists for the retail cue name. Everything else stays unrouted and
// silent -- it is not guessed from classic Lemmings. See
// docs/scratch/native-022-sound-cue-routing.md for the evidence table.
//
// This is cue routing only. It makes no claim about native sample timing,
// volume, mixing or resampling fidelity.
//
// Failure is silent by design: no audio backend, no device, a missing WAV or
// --no-audio produce a diagnostic (or nothing) and the game continues.

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

namespace webo::sim {
enum class SoundCue : std::uint8_t;
class World;
}  // namespace webo::sim

namespace webo::audio {

struct CueRoute {
  sim::SoundCue cue;
  std::string_view name;      // retail cue name, resolved with soundAssetName()
  std::string_view evidence;  // retained static play-site read
};

// The routed entries only, in table order.
std::span<const CueRoute> cueRoutes();

// Retail cue name for a sim cue, or nullopt when the cue is unrouted.
std::optional<std::string_view> routedCueName(sim::SoundCue cue);

// Sim cues that have no retained play-site evidence and therefore stay silent.
std::span<const sim::SoundCue> unroutedCues();

// Called with the literal retail cue name each time a routed cue is requested,
// whether or not a device exists. Tests use it as a recording sink.
using CueObserver = std::function<void(std::string_view)>;
using DiagnosticSink = std::function<void(const std::string&)>;

class CuePlayback {
public:
  CuePlayback() = default;
  CuePlayback(const CuePlayback&) = delete;
  CuePlayback& operator=(const CuePlayback&) = delete;

  // --no-audio: no device, no routing, observer silent. Cues are still drained
  // from the World so its buffer does not grow.
  void disable() { disabled_ = true; }
  bool disabled() const { return disabled_; }

  void setObserver(CueObserver observer) { observer_ = std::move(observer); }
  void setDiagnosticSink(DiagnosticSink sink) { diagnostic_ = std::move(sink); }

  // Opens the device (when asked) and loads the routed cue WAVs from the
  // library overlay. Idempotent: the first call wins, so the frontend and every
  // gameplay leg can call it without a second device lifecycle. Failures are
  // reported through the diagnostic sink and leave the adapter silent.
  void start(const AssetLibrary& library, bool openDevice);
  bool started() const { return started_; }
  bool deviceReady() const { return mixer_.ready(); }

  // Routes one sim cue. Unrouted cues are counted and ignored.
  void play(sim::SoundCue cue);

  // Takes every pending cue from the World and plays each in order.
  void drain(sim::World& world);

  // Whether a routed cue would currently reach an observer or a device.
  bool active() const { return !disabled_ && (observer_ || mixer_.ready()); }

  std::size_t drained() const { return drained_; }  // every sim cue taken
  std::size_t routed() const { return routed_; }    // routed cues requested

private:
  void report(const std::string& message) const;

  Mixer mixer_;
  CueObserver observer_;
  DiagnosticSink diagnostic_;
  bool disabled_ = false;
  bool started_ = false;
  std::size_t drained_ = 0;
  std::size_t routed_ = 0;
};

}  // namespace webo::audio
