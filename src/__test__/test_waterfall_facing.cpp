#include "app/BackdropScene.h"
#include "app/Raster.h"
#include "assets/Box.h"
#include "assets/LevelObjects.h"
#include "sim/RetailLevel.h"

#include <filesystem>
#include <iostream>

using namespace webo;
using namespace webo::app;

int main(int argc, char** argv) try {
  if (argc != 2) return 2;
  const std::filesystem::path game = argv[1];
  const auto library = AssetLibrary::open(game / "lemmings.box", game / "patch.box").value();
  constexpr auto level = "E18_KF_2";
  const auto bytes = library.read(sim::resolveRetailLevel(library, level).value().file).value();
  auto backdrop = BackdropScene::build(library, bytes, level).value();
  // Isolate the sheet across the opening hatch so other depth writers cannot
  // hide a submission from either side of it.
  backdrop.setSecondaryHidden([](const SceneNode& node) { return node.name != "transWaterFall_01_"; });
  CameraParams pose;
  pose.levelEyeHeight = sim::loadRetailLevel(library, level).value().terrain.height();
  pose.yawUnits = readSelectedHatchCamera(bytes).value().yawUnits;
  const auto render = [&] {
    Raster raster(pose.width, pose.height);
    raster.clear(0, 0, 0);
    BackdropStats stats;
    backdrop.renderStage(raster, buildCamera(pose), library, stats, BackdropScene::Stage::Secondary);
    return stats;
  };
  const auto rear = render();
  pose.yawUnits += pose.yawUnitsPerTurn / 2;
  const auto front = render();
  if (rear.predicateRejected == 0 || rear.pixels != 0 || front.pixels == 0) {
    std::cerr << "Waterfall rear rejected=" << rear.predicateRejected << " rear writes=" << rear.pixels
              << " front writes=" << front.pixels << '\n';
    return 1;
  }
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << '\n';
  return 1;
}
