#include "app/AppEntry.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#include <cstdlib>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

bool pushKey() {
  SDL_Event event{};
  event.type = SDL_KEYDOWN;
  event.key.type = SDL_KEYDOWN;
  event.key.keysym.sym = SDLK_RETURN;
  return SDL_PushEvent(&event) == 1;
}

bool pushMapClick() {
  SDL_Event event{};
  event.type = SDL_MOUSEBUTTONDOWN;
  event.button.type = SDL_MOUSEBUTTONDOWN;
  event.button.button = SDL_BUTTON_LEFT;
  event.button.x = 206;
  event.button.y = 204;
  return SDL_PushEvent(&event) == 1;
}

std::vector<char> bytes(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

int invoke(const std::vector<std::string>& args,
           webo::app::FrontendWindowObserver observer = {}) {
  std::vector<char*> argv;
  argv.reserve(args.size());
  for (const auto& arg : args)
    argv.push_back(const_cast<char*>(arg.c_str()));
  return webo::app::runApplication(static_cast<int>(argv.size()), argv.data(),
                                   std::move(observer));
}

}  // namespace

int main(int argc, char** argv) {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "FrontendEntry test requires SDL2\n";
  return 1;
#else
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir) {
    std::cout << "FrontendEntry: SKIPPED (set WEBOLUTION_GAME_DIR to retail assets)\n";
    return 0;
  }
  // This test must never initialize SDL against the user's desktop, including
  // when launched manually instead of through the configured CTest environment.
  const char* videoDriver = std::getenv("SDL_VIDEODRIVER");
  if (!videoDriver || std::string(videoDriver) != "dummy") {
    std::cerr << "FrontendEntry: refusing to run without SDL_VIDEODRIVER=dummy\n";
    return 1;
  }
  if (argc < 2 || !argv[1] || !*argv[1]) {
    std::cerr << "FrontendEntry: expected test output directory in argv[1]\n";
    return 1;
  }
  const fs::path outDir(argv[1]);
  std::error_code ec;
  fs::create_directories(outDir, ec);
  if (ec) { std::cerr << "cannot create test output directory: " << ec.message() << '\n'; return 1; }

  // A fresh exclusive child lets CTest repeat without overwriting earlier
  // diagnostic evidence or cleaning anything owned by the caller.
  const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
  const fs::path runDir = outDir / ("run-" + std::to_string(stamp));
  if (!fs::create_directory(runDir, ec) || ec) {
    std::cerr << "FrontendEntry: cannot create fresh output directory\n";
    return 1;
  }
  std::cout << "frontend entry evidence: " << runDir.string() << '\n';
  const fs::path entryShot = runDir / "entry.png";
  const fs::path resetShot = runDir / "entry-reset.png";
  const fs::path expectedShot = runDir / "expected.png";
  const fs::path expectedZeroShot = runDir / "expected-zero.png";
  const fs::path expectedCamera = runDir / "expected-camera.json";
  // Independent settled-pose equivalent of the opening move on frame30:
  // 29 prior frame deltas (pose is computed before increment), no yaw change
  // before intro second1. This does not invoke CameraIntro/buildCamera's intro
  // branch, and preserves the original direct-level diagnostic render route.
  double introSeconds = 0.0;
  for (int i = 0; i < 29; ++i) introSeconds += 1.0 / 144.0;
  const double q = introSeconds / 2.0;
  const double distance = 377.516 + (350.0 - (2.0-q)*q*350.0);
  {
    std::ofstream cameraFile(expectedCamera);
    cameraFile << std::setprecision(17) << "{\"eye_distance\":" << distance << "}\n";
    if (!cameraFile) { std::cerr << "cannot write entry camera oracle\n"; return 1; }
  }

  bool traceOk = true;
  int frameCalls = 0;
  const auto observer = [&](const webo::app::FrontendSession& session, int frame) {
    ++frameCalls;
    if (frameCalls > 3) traceOk = false;
    const auto screen = session.screen();
    const auto counter = session.constructionCounter();
    if (frame == 0) {
      traceOk &= screen == webo::shell::Screen::Menu && counter == 3;
      traceOk &= pushKey();
    } else if (frame == 1) {
      traceOk &= screen == webo::shell::Screen::Confirm && counter == 4;
      traceOk &= pushKey();
    } else if (frame == 2) {
      traceOk &= screen == webo::shell::Screen::Map && counter == 5;
      traceOk &= pushMapClick();
    } else {
      traceOk = false;
    }
  };

  const int entryResult = invoke({"webolution-test", "--frontend", "--level", "MM_DIG_000",
      "--frames", "30", "--window-shot", entryShot.string(),
      "--game-dir", gameDir}, observer);
  bool ok = entryResult == 0 && traceOk && frameCalls == 3 && fs::is_regular_file(entryShot);
  if (!ok) std::cerr << "FAIL: production frontend entry route (result=" << entryResult
                      << ", observer frames=" << frameCalls << ")\n";

  traceOk = true;
  frameCalls = 0;
  const int resetResult = invoke({"webolution-test", "--frontend", "--level", "MM_DIG_000",
      "--sim-ticks", "20", "--frames", "30", "--window-shot", resetShot.string(),
      "--game-dir", gameDir}, observer);
  ok &= resetResult == 0 && traceOk && frameCalls == 3 && fs::is_regular_file(resetShot);

  const auto render = [&](const fs::path& path, int ticks) {
    return invoke({"webolution-test", "--level", "MM_CLIMB_002", "--native", "--sim-ticks",
        std::to_string(ticks), "--skill-panel", "--hud-ticks", std::to_string(ticks), "--cursor-fixture", "--shot", path.string(),
        "--camera", expectedCamera.string(), "--game-dir", gameDir});
  };
  const int zeroResult = render(expectedZeroShot, 0);
  const int expectedResult = render(expectedShot, 1);
  const auto actualBytes = bytes(entryShot);
  const auto resetBytes = bytes(resetShot);
  const auto expectedBytes = bytes(expectedShot);
  const auto zeroBytes = bytes(expectedZeroShot);
  ok &= zeroResult == 0 && expectedResult == 0 && !actualBytes.empty() &&
        !expectedBytes.empty() && !zeroBytes.empty();
  ok &= actualBytes == expectedBytes && resetBytes == expectedBytes;
  ok &= expectedBytes != zeroBytes;  // positive control: one sim tick changes the render
  if (actualBytes != expectedBytes)
    std::cerr << "FAIL: frontend-selected level output differs from direct retail render\n";
  if (resetBytes != expectedBytes)
    std::cerr << "FAIL: diagnostic-start entry differs from fresh retail render\n";
  if (expectedBytes == zeroBytes)
    std::cerr << "FAIL: tick-0 positive control matched tick-1 render\n";

  if (ok) std::cout << "PASS: production frontend selection reaches matching retail level render\n";
  return ok ? 0 : 1;
#endif
}
