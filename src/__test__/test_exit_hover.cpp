#include "app/BackdropScene.h"
#include "app/Raster.h"
#include "assets/Box.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>

using namespace webo;

namespace {
template<class T>
T require(std::expected<T, std::string> result) {
  if (!result) throw std::runtime_error(result.error());
  return std::move(*result);
}

void check(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void render(const app::BackdropScene& scene, app::Raster& raster,
            const app::CameraBasis& camera, const AssetLibrary& library) {
  raster.clear(0, 0, 0);
  app::BackdropStats stats;
  scene.render(raster, camera, library, stats);
}
}  // namespace

int main(int argc, char** argv) try {
  check(argc == 2, "Expected the game data directory");
  const std::filesystem::path game(argv[1]);
  const auto library = require(AssetLibrary::open(game / "lemmings.box", game / "patch.box"));
  const auto single = require(library.read("COMPILED\\MM_CLIMB_002.LVL"));
  auto scene = require(app::BackdropScene::build(library, single, "MM_CLIMB_002"));
  const auto presentation = require(scene.entranceExitPresentation(single));
  const auto camera = app::buildCamera({.yawUnits = 240});
  app::Raster raster(camera.width, camera.height);

  scene.setEntranceExitPresentation(presentation, 0, 0, 1.5);
  render(scene, raster, camera, library);
  const auto low = scene.exitHoverTarget(camera, raster, 190, 238);
  scene.setEntranceExitPresentation(presentation, 0, 0, 0.5);
  render(scene, raster, camera, library);
  const auto high = scene.exitHoverTarget(camera, raster, 190, 238);
  check(low && high && high->top < low->top, "The hit box must follow the balloon's bob");
  check(!scene.exitHoverTarget(camera, raster, high->right, 238), "The right edge is outside");
  std::ranges::fill(raster.mutableDepthBuffer(), 0.0);
  check(!scene.exitHoverTarget(camera, raster, 190, 238), "A fully occluded exit has no target");

  scene.setEntranceExitPresentation(presentation, 0, 1.5, 0.5, 0.0);
  render(scene, raster, camera, library);
  const auto noSway = scene.exitHoverTarget(camera, raster, 190, 210);
  scene.setEntranceExitPresentation(presentation, 0, 1.5, 0.5);
  render(scene, raster, camera, library);
  const auto swayed = scene.exitHoverTarget(camera, raster, 190, 210);
  check(noSway && swayed && (noSway->left != swayed->left || noSway->right != swayed->right),
        "The hit box must follow the departing balloon's sway");

  const auto multi = require(library.read("COMPILED\\MM_E1_5_SKILLS.LVL"));
  const auto twoExits = require(app::BackdropScene::build(library, multi, "MM_E1_5_SKILLS"));
  const auto twoExitCamera = app::buildCamera({.yawUnits = 128});
  render(twoExits, raster, twoExitCamera, library);
  const auto left = twoExits.exitHoverTarget(twoExitCamera, raster, 135, 349);
  const auto right = twoExits.exitHoverTarget(twoExitCamera, raster, 502, 349);
  check(left && right && left->right < right->left, "Both exit balloons must be independently hoverable");
  check(!twoExits.exitHoverTarget(twoExitCamera, raster, 320, 390), "Empty space has no target");
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << '\n';
  return 1;
}
