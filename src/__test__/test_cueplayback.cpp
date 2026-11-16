// native-022: sim cue -> retail cue name routing and the silent-degrade paths.
//
// Asset-free except where WEBOLUTION_GAME_DIR names an install, which is only
// read. Run under SDL_AUDIODRIVER=dummy: no real device is ever opened.
//
// Expectations are LITERAL retail cue names. They are not computed by another
// recovered invocation. This establishes cue routing only -- not native sample
// timing, volume, mixing or resampling fidelity.

#include "assets/Box.h"
#include "assets/Wav.h"
#include "audio/CuePlayback.h"
#include "sim/RetailLevel.h"
#include "sim/Terrain.h"
#include "sim/World.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {

int g_failures = 0;

void check(bool cond, const std::string& what) {
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

void put32(std::vector<std::byte>& v, std::uint32_t n) {
  for (int i = 0; i < 4; ++i) v.push_back(static_cast<std::byte>((n >> (8 * i)) & 0xFF));
}
void putBytes(std::vector<std::byte>& v, std::string_view s) {
  for (char c : s) v.push_back(static_cast<std::byte>(c));
}
void put16(std::vector<std::byte>& v, std::uint16_t n) {
  v.push_back(static_cast<std::byte>(n & 0xFF));
  v.push_back(static_cast<std::byte>((n >> 8) & 0xFF));
}

// A real, tiny 22050 Hz mono 16-bit PCM WAV.
std::string makeWav() {
  constexpr int kSamples = 64;
  std::vector<std::byte> v;
  putBytes(v, "RIFF");
  put32(v, 36 + kSamples * 2);
  putBytes(v, "WAVEfmt ");
  put32(v, 16);
  put16(v, 1);
  put16(v, 1);
  put32(v, 22050);
  put32(v, 22050 * 2);
  put16(v, 2);
  put16(v, 16);
  putBytes(v, "data");
  put32(v, kSamples * 2);
  for (int i = 0; i < kSamples; ++i) put16(v, static_cast<std::uint16_t>((i % 8) * 1000));
  return std::string(reinterpret_cast<const char*>(v.data()), v.size());
}

// Minimal genuine LEMBOX archive (same layout test_box.cpp builds).
std::vector<std::byte> makeBox(const std::vector<std::pair<std::string, std::string>>& files) {
  std::vector<std::byte> names;
  for (const auto& [name, _] : files) {
    put32(names, static_cast<std::uint32_t>(name.size()));
    putBytes(names, name);
  }
  const std::uint32_t count = static_cast<std::uint32_t>(files.size());
  const std::size_t headerAndNames = 14 + names.size();
  const std::size_t tables = 4 + count * 4u + 4 + count * 4u;
  std::uint32_t cursor = static_cast<std::uint32_t>(headerAndNames + tables);
  std::vector<std::uint32_t> offsets, sizes;
  for (const auto& [_, body] : files) {
    offsets.push_back(cursor);
    sizes.push_back(static_cast<std::uint32_t>(body.size()));
    cursor += static_cast<std::uint32_t>(body.size());
  }
  std::vector<std::byte> out;
  putBytes(out, "LEMBOX");
  put32(out, count);
  put32(out, static_cast<std::uint32_t>(names.size()));
  out.insert(out.end(), names.begin(), names.end());
  put32(out, count);
  for (auto o : offsets) put32(out, o);
  put32(out, count);
  for (auto s : sizes) put32(out, s);
  for (const auto& [_, body] : files) putBytes(out, body);
  return out;
}

fs::path scratchDir() {
  std::error_code ec;
  fs::path base = fs::temp_directory_path(ec);
  if (ec) base = fs::current_path(ec);
  const fs::path dir = base / "webolution_test" / "cueplayback";
  fs::create_directories(dir, ec);
  return dir;
}

fs::path writeBox(const std::string& stem,
                  const std::vector<std::pair<std::string, std::string>>& files) {
  const fs::path p = scratchDir() / (stem + ".box");
  const auto data = makeBox(files);
  std::ofstream f(p, std::ios::binary);
  f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
  return p;
}

void record(audio::CuePlayback& playback, std::vector<std::string>& heard) {
  playback.setObserver([&heard](std::string_view name) { heard.emplace_back(name); });
}

void testTable() {
  std::cout << "cue table\n";
  const auto routes = audio::cueRoutes();
  check(routes.size() == 3, "exactly three cues have retained play-site evidence");

  // Literal retail names, not values produced by another recovered invocation.
  check(audio::routedCueName(sim::SoundCue::OhNo) == std::optional<std::string_view>("Ohno"),
        "OhNo routes to Ohno");
  check(audio::routedCueName(sim::SoundCue::Splat) == std::optional<std::string_view>("FallSplat"),
        "the sim's Splat is corrected to FallSplat");
  check(audio::routedCueName(sim::SoundCue::Splat) != std::optional<std::string_view>("Splat"),
        "the retail Splat cue (zero play references) is never played");
  check(audio::routedCueName(sim::SoundCue::Exit) == std::optional<std::string_view>("Yippee"),
        "saving at the exit routes to Yippee (playtest 6 V4)");
  for (sim::SoundCue unevidenced :
       {sim::SoundCue::Assign, sim::SoundCue::Explode})
    check(!audio::routedCueName(unevidenced).has_value(), "no retained play site => unrouted");
  check(audio::unroutedCues().size() == 2, "Assign and Explode are listed as unknown");

  for (const auto& route : routes) {
    check(!route.evidence.empty() && route.evidence.find("0x") != std::string_view::npos,
          std::format("route {} cites a retained address", route.name));
    check(audio::routedCueName(route.cue) == std::optional<std::string_view>(route.name),
          std::format("route {} resolves through the lookup", route.name));
  }

  check(soundAssetName("Ohno") == "SOUNDS\\OHNO.WAV", "Ohno resolves to its archive entry name");
  check(soundAssetName("FallSplat") == "SOUNDS\\FALLSPLAT.WAV",
        "FallSplat resolves to its archive entry name");

  // Synthetic archive carrying exactly the routed names: every route resolves.
  const std::string wav = makeWav();
  const auto path = writeBox("table", {{"SOUNDS\\OHNO.WAV", wav}, {"SOUNDS\\FALLSPLAT.WAV", wav},
                                      {"SOUNDS\\YIPPEE.WAV", wav}});
  {
    auto lib = AssetLibrary::open(path);
    check(lib.has_value(), "synthetic archive opens");
    if (lib) {
      for (const auto& route : routes)
        check(lib->contains(soundAssetName(route.name)),
              std::format("synthetic archive has an entry for routed cue {}", route.name));
    }
  }
  std::error_code ec;
  fs::remove(path, ec);

  // Installed archives, read-only: report the actual availability of each cue.
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (dir && fs::exists(fs::path(dir) / "lemmings.box")) {
    auto lib = AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
    check(lib.has_value(), "installed archives open read-only");
    if (lib) {
      for (const auto& route : routes) {
        const auto entry = soundAssetName(route.name);
        const bool present = lib->contains(entry);
        bool decodable = false;
        std::string detail = "NOT AVAILABLE";
        if (present) {
          auto sound = loadSound(*lib, entry);
          if (sound) {
            decodable = true;
            auto bytes = lib->read(entry);
            check(bytes.has_value(), "installed WAV bytes readable");
            const bool patch = lib->patch() && lib->patch()->contains(entry);
            detail = std::format("present, bytes={}, {} Hz {} ch frames={} samples={} seconds={:.6f}, source={}",
                                 bytes ? bytes->size() : 0, sound->sampleRate,
                                 sound->channels, sound->frameCount(), sound->samples.size(),
                                 sound->seconds(), patch ? "patch.box" : "lemmings.box");
          } else {
            detail = "present but undecodable: " + sound.error();
          }
        }
        std::cout << std::format("  installed {} -> {}: {}\n", route.name, entry, detail);
        check(decodable, std::format("installed archive has a decodable {}", route.name));
      }
    }
  } else {
    std::cout << "  installed archives: skipped, WEBOLUTION_GAME_DIR unset\n";
  }
}

void testDegrade() {
  std::cout << "degrade silently\n";
  const std::string wav = makeWav();
  // FallSplat is deliberately missing: a missing WAV must not stop anything.
  const auto path = writeBox("degrade", {{"SOUNDS\\OHNO.WAV", wav}});
  {
    auto lib = AssetLibrary::open(path);
    check(lib.has_value(), "degrade archive opens");
    if (!lib) return;

    {  // device requested, one WAV missing
      audio::CuePlayback playback;
      std::vector<std::string> heard, diagnostics;
      record(playback, heard);
      playback.setDiagnosticSink([&](const std::string& m) { diagnostics.push_back(m); });
      playback.start(*lib, true);
      playback.start(*lib, true);  // idempotent: no second device lifecycle
      check(playback.deviceReady(), "dummy audio driver opens one device");
      check(std::any_of(diagnostics.begin(), diagnostics.end(),
                        [](const std::string& m) { return m.find("FallSplat") != std::string::npos; }),
            "a missing WAV is reported");
      check(std::none_of(diagnostics.begin(), diagnostics.end(),
                         [](const std::string& m) { return m.find("Ohno") != std::string::npos; }),
            "the present WAV loads without complaint");
      playback.play(sim::SoundCue::OhNo);
      playback.play(sim::SoundCue::Splat);  // WAV missing: still routed, silently skipped
      playback.play(sim::SoundCue::Assign);
      check((heard == std::vector<std::string>{"Ohno", "FallSplat"}),
            "routed cues reach the sink by literal name; the unrouted Assign does not");
      check(playback.drained() == 3 && playback.routed() == 2, "drained and routed counts");
    }

    {  // no device requested (the default for every test that does not opt in)
      audio::CuePlayback playback;
      std::vector<std::string> heard, diagnostics;
      record(playback, heard);
      playback.setDiagnosticSink([&](const std::string& m) { diagnostics.push_back(m); });
      playback.start(*lib, false);
      check(!playback.deviceReady() && diagnostics.empty(), "no opt-in: no device, no noise");
      playback.play(sim::SoundCue::OhNo);
      check(heard.size() == 1 && heard[0] == "Ohno", "sink still sees routed cues without a device");
    }

    {  // --no-audio
      audio::CuePlayback playback;
      std::vector<std::string> heard, diagnostics;
      record(playback, heard);
      playback.setDiagnosticSink([&](const std::string& m) { diagnostics.push_back(m); });
      playback.disable();
      playback.start(*lib, true);
      playback.play(sim::SoundCue::OhNo);
      check(!playback.deviceReady() && diagnostics.empty() && heard.empty(),
            "--no-audio: no device, no diagnostic, no cue");
    }

    {  // absent backend
      SDL_SetHintWithPriority(SDL_HINT_AUDIODRIVER, "webolution-no-such-driver", SDL_HINT_OVERRIDE);
      audio::CuePlayback playback;
      std::vector<std::string> heard, diagnostics;
      record(playback, heard);
      playback.setDiagnosticSink([&](const std::string& m) { diagnostics.push_back(m); });
      playback.start(*lib, true);
      check(!playback.deviceReady(), "absent-backend override must prevent opening a device");
      if (!playback.deviceReady()) {
        check(std::any_of(diagnostics.begin(), diagnostics.end(),
                          [](const std::string& m) {
                            return m.find("continuing silently") != std::string::npos;
                          }),
              "an absent backend degrades to a diagnostic");
        playback.play(sim::SoundCue::OhNo);
        check(heard.size() == 1, "and routing continues without a device");
      }
      SDL_SetHintWithPriority(SDL_HINT_AUDIODRIVER, "dummy", SDL_HINT_OVERRIDE);
    }
  }
  std::error_code ec;
  fs::remove(path, ec);
}

sim::LevelSetup flatLevel(int floorY) {
  sim::LevelSetup s;
  s.terrain = sim::Terrain(200, 100);
  s.terrain.fillBox(0, floorY, 200, 100 - floorY, sim::Material::Earth);
  const sim::World::Rules r;
  const int drop = std::min(r.fallSpeed * 3, r.splatDistance - 1);
  s.entrances.push_back({100, floorY - drop});
  s.totalLemmings = 1;
  s.saveTarget = 1;
  s.releaseIntervalTicks = 1;
  return s;
}

void testWorldCues() {
  std::cout << "world cues through the adapter\n";

  {  // bomber fuse on a synthetic world: Ohno, and nothing else is played
    sim::LevelSetup s = flatLevel(80);
    sim::World w(s);
    for (int i = 0; i < 30; ++i) w.tick();
    audio::CuePlayback playback;
    std::vector<std::string> heard;
    record(playback, heard);
    check(w.assign(0, sim::Skill::Bomber), "bomber assigns");
    playback.drain(w);
    check(heard.empty(), "the assign cue (Chink) has no retained play site and stays silent");
    for (int i = 0; i < 40; ++i) {
      w.tick();
      playback.drain(w);
    }
    check(std::count(heard.begin(), heard.end(), "Ohno") == 1, "the bomber fuse plays Ohno once");
    check(std::none_of(heard.begin(), heard.end(),
                       [](const std::string& n) { return n == "Explode" || n == "Chink"; }),
          "Explode and Chink are not played");
  }

  {  // long fall on a synthetic world: FallSplat, never Splat
    sim::LevelSetup tall = flatLevel(80);
    tall.entrances = {{50, 0}};
    sim::World w(tall);
    audio::CuePlayback playback;
    std::vector<std::string> heard;
    record(playback, heard);
    for (int i = 0; i < 80; ++i) {
      w.tick();
      playback.drain(w);
    }
    check(std::count(heard.begin(), heard.end(), "FallSplat") >= 1,
          "a long-fall splat plays FallSplat");
    check(std::count(heard.begin(), heard.end(), "Splat") == 0, "and never the retail Splat cue");
  }

  // Real installed level: the retail setup of MM_CLIMB_002 with its exits
  // removed so the lemming cannot leave before the fuse ends.
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !fs::exists(fs::path(dir) / "lemmings.box")) {
    std::cout << "  installed level: skipped, WEBOLUTION_GAME_DIR unset\n";
    return;
  }
  auto lib = AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
  check(lib.has_value(), "installed archives open");
  if (!lib) return;
  auto setup = sim::loadRetailLevel(*lib, "MM_CLIMB_002");
  check(setup.has_value(), "MM_CLIMB_002 loads");
  if (!setup) return;
  setup->exits.clear();
  setup->skills.fill(-1);  // synthetic: the retail stock has no bombers
  sim::World w(*setup);
  audio::CuePlayback playback;
  std::vector<std::string> heard;
  record(playback, heard);
  int guard = 0;
  while (w.activeCount() < 1 && guard++ < 400) w.tick();
  check(w.activeCount() >= 1, "a lemming is released on the retail level");
  if (w.activeCount() < 1) return;
  playback.drain(w);
  check(w.assign(0, sim::Skill::Bomber), "bomber assigns on the retail level");
  playback.drain(w);
  for (int i = 0; i < 40; ++i) {
    w.tick();
    playback.drain(w);
  }
  const bool ohno = std::count(heard.begin(), heard.end(), "Ohno") == 1;
  std::cout << std::format("  retail MM_CLIMB_002 bomber: Ohno {}, played {} routed cue(s)\n",
                           ohno ? "heard" : "not heard (lemming died first)", heard.size());
  check(ohno || !w.lemmings()[0].alive(),
        "a live bomber on the retail level plays Ohno when the fuse ends");
}

}  // namespace

int main() {
  const char* driver = std::getenv("SDL_AUDIODRIVER");
  if (!driver || std::string(driver) != "dummy") {
    std::cerr << "CuePlayback: refusing to run without SDL_AUDIODRIVER=dummy\n";
    return 1;
  }
  testTable();
  testDegrade();
  testWorldCues();
  if (g_failures != 0) {
    std::cerr << std::format("{} failure(s)\n", g_failures);
    return 1;
  }
  std::cout << "PASS: cue routing table, degrade paths and world cues\n";
  return 0;
}
