// D3c: offline click-to-assign wiring. Loads MM_CLIMB_002, ticks to a fixed
// count where two lemmings are alive, and checks that each alive lemming's
// on-screen sprite box (the same placement drawInjectedWalkers computes)
// feeds pickHoveredLemming. No stock game, no window.
//
// Tick count 25, counted before this test was run, from the loader's seconds
// release model and tickDtSeconds() == 0.2:
//   releaseArmSeconds 0.723061695938398 arms on tick 4 (elapsed 0.8) and
//   seeds releaseFirstCountdownSeconds 1.3 with no decrement that tick;
//   tick 11 spawns the first lemming; releaseIntervalSeconds
//   2.8864866275534937 then crosses zero again on tick 25, spawning the
//   second. continuousStepperClock stays false (the loader does not set it).
//   After 25 ticks both lemmings are alive, so the alive-only walker list is
//   index-aligned with World::lemmings().
//
// Camera: tools/harness/fixtures/camera_a1/camera-fixture-params.json, the
// A1/A4 params file loadCameraParams already reads (test_actorcompose's
// "initial" view). reference-state.json is a live memory dump and is not
// that schema.
//
// priority and distanceSquared are 0 on every candidate. That is the
// inherited D2a limitation: first eligible overlapping candidate in list
// order wins only when every candidate is tied on both keys. This test does
// not resolve the priority table or screen+0x58.

#include "app/Camera.h"
#include "app/Pick.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#ifndef WEBOLUTION_SOURCE_DIR
#define WEBOLUTION_SOURCE_DIR "."
#endif

namespace {

constexpr int kTickCount = 25;

void check(bool condition, const char* message) {
  if (!condition) {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}

// Toward-zero truncation, matching the conversion Pick.h documents for the
// original screen-box fields. Not a new exe read.
webo::app::PickBox pickBoxFromPlacement(const webo::app::ActorSpritePlacement& placement) {
  return webo::app::PickBox{
      static_cast<int>(placement.left),
      static_cast<int>(placement.top),
      static_cast<int>(placement.right),
      static_cast<int>(placement.bottom),
  };
}

}  // namespace

int main() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir || !std::filesystem::exists(std::filesystem::path(dir) / "lemmings.box")) {
    std::cout << "click-to-assign: skipped, WEBOLUTION_GAME_DIR has no lemmings.box\n";
    return 0;
  }

  const std::filesystem::path gameDir(dir);
  auto library = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  check(library.has_value(), "open lemmings.box");
  auto setup = webo::sim::loadRetailLevel(*library, "MM_CLIMB_002");
  check(setup.has_value(), "loadRetailLevel MM_CLIMB_002");

  webo::sim::World world(*setup);
  for (int i = 0; i < kTickCount; ++i)
    world.tick();

  const auto& lemmings = world.lemmings();
  std::size_t live = 0;
  for (const auto& lemming : lemmings)
    if (lemming.alive())
      ++live;
  std::cout << "click-to-assign: ticks=" << kTickCount << " lemmings=" << lemmings.size()
            << " live=" << live << " spawned=" << world.spawned() << "\n";
  for (std::size_t i = 0; i < lemmings.size(); ++i) {
    const auto& lemming = lemmings[i];
    std::cout << "  lemming " << i << " cell=(" << lemming.x << "," << lemming.y
              << ") state=" << webo::sim::stateName(lemming.state)
              << " alive=" << (lemming.alive() ? 1 : 0) << "\n";
  }
  check(lemmings.size() == 2 && live == 2, "tick 25 has exactly two lemmings, both alive");

  std::string levelPath;
  for (const auto& name : webo::levelFileNames(*library)) {
    if (name.find("MM_CLIMB_002") != std::string::npos) {
      levelPath = name;
      break;
    }
  }
  check(!levelPath.empty(), "MM_CLIMB_002.LVL is in the archive");
  auto blob = library->read(levelPath);
  check(blob.has_value(), "read the level blob");
  auto mesh = webo::findCylinderMesh(*blob);
  check(mesh.has_value(), "find the cylinder mesh");
  auto index = webo::indexLevel(*blob);
  auto scene = webo::app::TubeScene::build(
      *library, *mesh, *blob,
      index ? std::span<const std::uint32_t>(index->grid0) : std::span<const std::uint32_t>{},
      true);
  check(scene.has_value(), "build TubeScene");

  const std::filesystem::path cameraPath =
      std::filesystem::path(WEBOLUTION_SOURCE_DIR) /
      "tools/harness/fixtures/camera_a1/camera-fixture-params.json";
  webo::app::CameraParams params;
  std::string cameraError;
  check(webo::app::loadCameraParams(cameraPath, params, cameraError), "load A1 camera-fixture-params.json");
  const webo::app::CameraBasis camera = webo::app::buildCamera(params);

  webo::app::SceneOptions options;
  options.injectedWalkers =
      webo::app::injectedWalkersFromWorld(world, world.terrain().height());
  check(options.injectedWalkers.size() == lemmings.size(),
        "alive-only walkers are index-aligned while every lemming is alive");

  const auto boxes = scene->injectedWalkerScreenBoxes(camera, options, *library);
  check(boxes.size() == lemmings.size(), "one screen box slot per lemming");

  std::vector<webo::app::PickCandidate> candidates(boxes.size());
  for (std::size_t i = 0; i < boxes.size(); ++i) {
    check(boxes[i].has_value(), "each alive lemming has a screen box");
    const webo::app::PickBox box = pickBoxFromPlacement(*boxes[i]);
    std::cout << "  box " << i << " [" << box.left << "," << box.top << "," << box.right << ","
              << box.bottom << "]\n";
    candidates[i].id = reinterpret_cast<void*>(static_cast<std::uintptr_t>(i));
    candidates[i].box = box;
    candidates[i].eligible = true;
    candidates[i].priority = 0;
    candidates[i].distanceSquared = 0.0;
  }

  // Lemming 0's own box, used as the cursor. With every tie-break key equal
  // to 0, an earlier lemming wins an exact tie, so this cursor selects
  // lemming 0 whenever the box is wide enough to overlap itself after the
  // 4px x shrink (right - left >= 4).
  constexpr std::size_t kChosen = 0;
  const webo::app::PickBox& chosen = candidates[kChosen].box;
  check(chosen.right - chosen.left >= 4, "chosen box is at least 4px wide");
  const webo::app::PickCandidate* hit = webo::app::pickHoveredLemming(candidates, chosen);
  check(hit != nullptr && hit->id == candidates[kChosen].id,
        "cursor coinciding with lemming 0 selects lemming 0");

  webo::app::PickBox far = chosen;
  far.left += 10000;
  far.right += 10000;
  check(webo::app::pickHoveredLemming(candidates, far) == nullptr,
        "cursor translated +10000 in x selects nothing");

  std::cout << "click-to-assign: chosen=" << kChosen << " far=nullptr\n";
  return 0;
}
