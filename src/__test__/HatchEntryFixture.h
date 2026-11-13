// Independent, headless entry-frame composition for the existing wiring tests.
// P9 changes their former closed-door expectation, not the camera, HUD or input
// contract. Explicit milliseconds keep this fixture independent of the SDL loop.
#pragma once

#include "app/BackdropScene.h"
#include "app/Raster.h"
#include "app/RuntimeCursor.h"
#include "app/RuntimeHud.h"
#include "app/SceneFrame.h"
#include "app/SkillPanel.h"
#include "app/TubeScene.h"
#include "assets/Cylinder.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "sim/RetailLevel.h"

#include <stdexcept>

namespace webo::test {
inline Image hatchEntryFixture(const AssetLibrary& library,
                               const app::CameraParams& cameraParams,
                               int hudTicks, int openingMs) {
  const auto require = []<class T>(std::expected<T, std::string> value) -> T {
    if (!value) throw std::runtime_error(value.error());
    return std::move(*value);
  };
  const auto level = require(sim::resolveRetailLevel(library, "MM_CLIMB_002"));
  const auto bytes = require(library.read(level.file));
  const auto mesh = require(findCylinderMesh(bytes));
  const auto index = require(indexLevel(bytes));
  auto tube = require(app::TubeScene::build(library, mesh, bytes, index.grid0, true));
  auto backdrop = require(app::BackdropScene::build(library, bytes, "MM_CLIMB_002"));
  const auto effects = require(backdrop.entranceExitPresentation(bytes));
  sim::World world(require(sim::loadRetailLevel(library, "MM_CLIMB_002")));
  for (int i = 0; i < hudTicks; ++i) world.tick();
  // These entry fixtures precede release. Actors would require their own
  // animation/motion clocks, so refuse to silently fake a later gameplay frame.
  if (world.spawned() != 0 || openingMs < 0 || openingMs > 208)
    throw std::runtime_error("hatch entry fixture is restricted to pre-release frames");
  const double arm = world.setup().releaseArmSeconds;
  backdrop.setEntranceExitPresentation(effects, arm > 0 ? openingMs / 1000.0 / arm : 1, 0);
  app::SceneOptions options;
  options.drawCellRoute = true;
  options.drawWall = options.drawOuter = options.drawInner = false;
  const auto camera = app::buildCamera(cameraParams);
  app::Raster raster(camera.width, camera.height);
  raster.clear(0, 0, 0);
  app::BackdropStats stats;
  app::scheduleSceneFrame(
      [&] { backdrop.renderStage(raster, camera, library, stats, app::BackdropScene::Stage::Primary); },
      [&] { backdrop.renderStage(raster, camera, library, stats, app::BackdropScene::Stage::LegacyUnattributed); },
      [&] { tube.render(raster, camera, options, &library); },
      [&] { backdrop.renderStage(raster, camera, library, stats, app::BackdropScene::Stage::FluidGate); },
      [&] { backdrop.renderStage(raster, camera, library, stats, app::BackdropScene::Stage::Secondary); });
  Image image;
  image.width = camera.width;
  image.height = camera.height;
  for (std::size_t i = 0; i < raster.framebuffer().rgb.size(); i += 3) {
    const auto& rgb = raster.framebuffer().rgb;
    image.rgba.insert(image.rgba.end(), {rgb[i], rgb[i + 1], rgb[i + 2], 255});
  }
  const auto panel = app::drawRetailSkillPanel(image, library, "MM_CLIMB_002");
  if (!panel) throw std::runtime_error(panel.error());
  const auto font = require(loadTexture(library, "TEXTURES\\^FONT.BMP"));
  app::drawRuntimeHud(image, font, world, false);
  const auto cursor = require(app::RuntimeCursor::load(library));
  cursor.draw(image, app::CursorState::NoLemming, image.width / 2, image.height / 2);
  return image;
}
}  // namespace webo::test
