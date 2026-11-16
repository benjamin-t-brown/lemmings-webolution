#pragma once

#include "app/RuntimeHud.h"
#include "app/RuntimeSkillPanel.h"
#include "sim/RetailLevel.h"

#include <chrono>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace webo::app {

// PROVISIONAL clone format, not an original-game replay format or fidelity claim. A step
// is one admitted advanceElapsed call (including its fast-forward passes and
// zero-ms updates), not World::tickCount(). That hybrid simulation depends on
// the exact sequence of dt values, not merely total elapsed time. Store the
// integer budget AFTER the wall-clock clamp/carry, and the presentation's hatch
// gate; replay never supplies wall time or re-runs picking.
inline const std::string& replayBuildIdentity() {
  // A TU timestamp alone misses incremental builds which only rebuild World.
  // Identify the actual running clone image once per process, outside play's
  // frame loop. FNV-1a is a build label, not a security/compatibility check.
  static const std::string identity = [] {
    std::filesystem::path executable;
#ifdef _WIN32
    std::array<wchar_t, 32768> path{};
    const auto length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (length && length < path.size()) executable = std::wstring(path.data(), length);
#elif defined(__linux__)
    executable = "/proc/self/exe";
#endif
    std::ifstream binary(executable, std::ios::binary);
    if (!binary) return std::string("image-unavailable; compiler ") + __VERSION__;
    std::uint64_t hash = 14695981039346656037ull;
    std::array<char, 65536> buffer{};
    while (binary) {
      binary.read(buffer.data(), buffer.size());
      for (std::streamsize i = 0; i < binary.gcount(); ++i) {
        hash ^= static_cast<unsigned char>(buffer[i]); hash *= 1099511628211ull;
      }
    }
    if (!binary.eof()) return std::string("image-read-failed; compiler ") + __VERSION__;
    std::ostringstream label;
    label << "exe-fnv1a64:" << std::hex << hash << " compiler " << __VERSION__;
    return label.str();
  }();
  return identity;
}

struct ReplayRecord {
  char kind = 0;
  std::uint64_t step = 0;
  std::array<int, 7> values{};
  std::uint64_t clockMs = 0;
  double seconds = 0;
};

class ReplayRecorder {
 public:
  static std::expected<ReplayRecorder, std::string> create(
      const std::filesystem::path& directory, std::string level, bool connected,
      int initialTicks, int initialRate) {
    std::error_code ec;
    std::filesystem::create_directories(directory, ec);
    if (ec) return std::unexpected("cannot create replay directory: " + ec.message());
    // Time plus a process-local serial also distinguishes retries in one clock tick.
    static unsigned int serial = 0;
    const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
    ReplayRecorder recorder;
    recorder.path_ = directory / (level + "-" + std::to_string(stamp) + "-" +
                                   std::to_string(++serial) + ".replay");
    recorder.partial_ = recorder.path_;
    recorder.partial_ += ".partial";
    recorder.output_.open(recorder.partial_, std::ios::binary | std::ios::trunc);
    if (!recorder.output_) return std::unexpected("cannot open " + recorder.partial_.string());
    recorder.output_ << "WEBOLUTION_REPLAY 1\n" << std::quoted(level) << ' '
        << std::quoted(replayBuildIdentity()) << ' ' << connected << ' ' << initialTicks << '\n';
    recorder.output_.flush();
    if (!recorder.output_) return std::unexpected("cannot write replay header");
    recorder.rate_ = initialRate;
    std::cerr << "record: " << recorder.path_.string() << '\n';
    return recorder;
  }

  // Called only when a pick resolved to an actor, even if assign() refused it.
  // The actor index is stable: World retains exited/dead actors in its vector.
  void assignment(std::size_t actor, sim::Skill skill, bool accepted, const sim::Lemming& before) {
    append('a', {static_cast<int>(actor), static_cast<int>(skill), accepted,
                 before.x, before.y, static_cast<int>(before.state)});
  }

  // Observe effective simulation commands after each input event and held-rate
  // repeat. No timestamp, SDL event, cursor pixel or camera is serialized.
  void commands(const sim::World& world, const RuntimeSimState& runtime,
                const RuntimeSkillSelection& selection, int rate) {
    if (selected_ != selection.selectedIndex()) {
      selected_ = selection.selectedIndex(); append('s', {selected_});
    }
    if (paused_ != runtime.paused()) { paused_ = runtime.paused(); append('p', {paused_}); }
    if (fast_ != runtime.fastForward()) { fast_ = runtime.fastForward(); append('f', {fast_}); }
    if (nuked_ != world.nukeRequested()) { nuked_ = world.nukeRequested(); append('n', {}); }
    if (rate_ != rate) {
      rate_ = rate;
      auto& row = append('r', {rate, world.setup().releaseIntervalTicks});
      row.seconds = world.setup().releaseIntervalSeconds;
    }
  }

  void advanced(const sim::World& world, const RuntimeSimState& runtime, bool releaseEnabled) {
    auto& row = append('t', {runtime.lastPassMs(), releaseEnabled, world.spawned(),
                            world.saved(), world.dead(), static_cast<int>(runtime.endTransition().phase())});
    row.clockMs = world.levelClockMs();
    ++step_;
  }

  std::expected<std::filesystem::path, std::string> finish(
      const sim::World& world, const RuntimeSimState& runtime, std::string_view reason) {
    // Buffer compact records in memory during play. Only attempt boundaries do
    // formatting or disk I/O; a crash leaves an explicitly incomplete .partial.
    output_ << std::setprecision(std::numeric_limits<double>::max_digits10);
    for (const auto& row : records_) {
      output_ << row.kind << ' ' << row.step;
      for (const int value : row.values) output_ << ' ' << value;
      output_ << ' ' << row.clockMs << ' ' << row.seconds << '\n';
    }
    const auto& snapshot = runtime.endTransition().snapshot();
    output_ << "result " << step_ << ' ' << world.saved() << ' ' << world.setup().saveTarget
        << ' ' << (snapshot ? snapshot->success : world.won()) << ' ' << bool(snapshot)
        << ' ' << std::quoted(std::string(reason)) << '\n';
    output_.flush();
    if (!output_) return std::unexpected("cannot write replay " + partial_.string());
    output_.close();
    if (output_.fail()) return std::unexpected("cannot close replay " + partial_.string());
    std::error_code ec;
    std::filesystem::rename(partial_, path_, ec);
    if (ec) return std::unexpected("cannot publish replay: " + ec.message());
    return path_;
  }

 private:
  ReplayRecord& append(char kind, std::array<int, 7> values) {
    records_.push_back({kind, step_, values});
    return records_.back();
  }
  std::ofstream output_;
  std::filesystem::path path_, partial_;
  // Small blocks avoid copying a whole attempt during a render frame as the
  // recording grows. There are no per-actor scans or per-frame disk writes.
  std::deque<ReplayRecord> records_;
  std::uint64_t step_ = 0;
  int selected_ = 0, rate_ = 0;
  bool paused_ = false, fast_ = false, nuked_ = false;
};

// No frontend, renderer, audio device or SaveStore is constructed on this path.
// A different build is expected when using recordings as regression checks;
// its identity is diagnostic, never an excuse to skip a replay.
inline int replayLevel(const AssetLibrary& library, const std::filesystem::path& file,
                       std::ostream& log = std::cerr) {
  std::ifstream input(file, std::ios::binary);
  std::string magic, level, build;
  int version = 0, connected = 0, initialTicks = 0;
  std::uint64_t step = 0;
  const auto fail = [&](std::string_view why) {
    log << "replay " << (level.empty() ? file.string() : level) << " step " << step
        << ": DIVERGED " << why << '\n';
    return 1;
  };
  if (!(input >> magic >> version) || magic != "WEBOLUTION_REPLAY" || version != 1 ||
      !(input >> std::quoted(level) >> std::quoted(build) >> connected >> initialTicks) ||
      (connected != 0 && connected != 1) || initialTicks < 0 || initialTicks > 1000000)
    return fail("invalid header/version");
  auto setup = sim::loadRetailLevel(library, level);
  if (!setup) return fail(setup.error());
  if (connected) setup->releaseArmSeconds = 0.0;
  sim::World world(std::move(*setup));
  for (int i = 0; i < initialTicks; ++i) { world.tick(); world.takeCues(); }
  RuntimeSimState runtime;
  int selected = 0;
  auto rate = RuntimeReleaseRateControl::create(library, level);
  auto rates = loadReleaseRateTable(library);
  if (!rate || !rates) return fail(!rate ? rate.error() : rates.error());
  if (build != replayBuildIdentity()) log << "replay " << level << ": recorded build " << build
                                          << "; running " << replayBuildIdentity() << '\n';
  const auto drain = [&] { world.takeCues(); };
  std::string kind;
  std::uint64_t records = 0;
  while (input >> kind) {
    if (++records > 10000000) return fail("record limit exceeded");
    std::uint64_t recordedStep = 0;
    if (!(input >> recordedStep) || recordedStep != step) return fail("out-of-order or invalid step");
    if (kind == "result") {
      int saved = 0, required = 0, won = 0, completed = 0;
      std::string reason, trailing;
      if (!(input >> saved >> required >> won >> completed >> std::quoted(reason)) ||
          (won != 0 && won != 1) || (completed != 0 && completed != 1) || (input >> trailing))
        return fail("invalid result/trailing data");
      const auto& snapshot = runtime.endTransition().snapshot();
      const bool actualWon = snapshot ? snapshot->success : world.won();
      if (saved != world.saved() || required != world.setup().saveTarget || won != actualWon ||
          completed != bool(snapshot)) {
        std::ostringstream detail;
        detail << "result expected saved=" << saved << " required=" << required << " won=" << won
            << " completed=" << completed << "; actual saved=" << world.saved()
            << " required=" << world.setup().saveTarget << " won=" << actualWon << " completed=" << bool(snapshot);
        return fail(detail.str());
      }
      log << "replay " << level << " step " << step << ": PASS saved=" << saved << '/' << required
          << " won=" << won << " completed=" << completed << " (" << reason << ")\n";
      return 0;
    }
    ReplayRecord row;
    if (kind.size() != 1) return fail("unknown input " + kind);
    row.kind = kind[0];
    for (int& value : row.values) if (!(input >> value)) return fail("truncated input " + kind);
    if (!(input >> row.clockMs >> row.seconds) || !std::isfinite(row.seconds)) return fail("invalid input " + kind);
    const auto& v = row.values;
    switch (row.kind) {
      case 's':
        if (v[0] < 0 || v[0] >= 8 || world.setup().skills[v[0]] <= 0) return fail("skill selection refused");
        selected = v[0]; break;
      case 'p':
        if ((v[0] != 0 && v[0] != 1) || v[0] == runtime.paused()) return fail("invalid pause command");
        runtime.togglePause(); break;
      case 'f':
        if ((v[0] != 0 && v[0] != 1) || v[0] == runtime.fastForward()) return fail("invalid fast-forward command");
        runtime.toggleFastForward(); break;
      case 'n':
        if (world.nukeRequested() || world.setup().nukeStock <= 0) return fail("nuke refused");
        world.nuke(); break;
      case 'r':
        if (v[0] < rate->minimum() || v[0] > rate->maximum() ||
            row.seconds != rates->secondsPerLemming(v[0]) ||
            v[1] != rates->intervalTicks(v[0], world.rules().ticksPerSecond)) return fail("release-rate input changed");
        world.setReleaseInterval(row.seconds, v[1]); break;
      case 'a': {
        if (v[0] < 0 || static_cast<std::size_t>(v[0]) >= world.lemmings().size() ||
            v[1] != selected || (v[2] != 0 && v[2] != 1))
          return fail("assignment actor=" + std::to_string(v[0]) + " skill=" + std::to_string(v[1]) +
                      " unavailable (selected=" + std::to_string(selected) + ")");
        const auto& actor = world.lemmings()[v[0]];
        if (actor.x != v[3] || actor.y != v[4] || static_cast<int>(actor.state) != v[5]) {
          std::ostringstream detail;
          detail << "assignment actor=" << v[0] << " expected x/y/state=" << v[3] << '/' << v[4] << '/' << v[5]
              << "; actual=" << actor.x << '/' << actor.y << '/' << static_cast<int>(actor.state);
          return fail(detail.str());
        }
        const bool accepted = world.assign(v[0], static_cast<sim::Skill>(v[1]));
        if (accepted != bool(v[2])) return fail("assignment actor " + std::to_string(v[0]) + " acceptance changed");
        drain();
        // Successful assignments resume through an explicit following p row.
        break;
      }
      case 't': {
        if (v[0] < 0 || v[0] > 60000 || (v[1] != 0 && v[1] != 1) || runtime.paused())
          return fail("invalid simulation budget/gate");
        // Reconstitute only the already-quantized integer budget. This does
        // not reconstruct or approximate the discarded real-clock fraction.
        if (!runtime.advanceElapsed(world, v[0] / 1000.0, connected, drain, 0, v[1]))
          return fail("simulation update refused");
        if (world.spawned() != v[2] || world.saved() != v[3] || world.dead() != v[4] ||
            world.levelClockMs() != row.clockMs || static_cast<int>(runtime.endTransition().phase()) != v[5]) {
          std::ostringstream detail;
          detail << "counts expected spawned/saved/dead=" << v[2] << '/' << v[3] << '/' << v[4]
              << " clock=" << row.clockMs << " phase=" << v[5] << "; actual=" << world.spawned()
              << '/' << world.saved() << '/' << world.dead() << " clock=" << world.levelClockMs()
              << " phase=" << static_cast<int>(runtime.endTransition().phase());
          return fail(detail.str());
        }
        ++step; break;
      }
      default: return fail("unknown input " + kind);
    }
  }
  return fail("missing final result (incomplete recording)");
}

} // namespace webo::app
