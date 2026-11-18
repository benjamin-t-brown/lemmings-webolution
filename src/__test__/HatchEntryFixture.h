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

#include <cmath>
#include <stdexcept>

namespace webo::test {
inline Image hatchEntryFixture(const AssetLibrary& library,
                               const app::CameraParams& cameraParams,
                               int hudTicks, double openingMs, bool connectedEntry = false,
                               const std::string& levelName = "MM_CLIMB_002", bool hoverFeedback = false,
                               double panelSeconds = 0) {
  const auto require = []<class T>(std::expected<T, std::string> value) -> T {
    if (!value) throw std::runtime_error(value.error());
    return std::move(*value);
  };
  const auto level = require(sim::resolveRetailLevel(library, levelName));
  const auto bytes = require(library.read(level.file));
  const auto mesh = require(findCylinderMesh(bytes));
  const auto index = require(indexLevel(bytes));
  auto tube = require(app::TubeScene::build(library, mesh, bytes, index.grid0, true));
  auto backdrop = require(app::BackdropScene::build(library, bytes, levelName));
  const auto effects = require(backdrop.entranceExitPresentation(bytes));
  sim::World world(require(sim::loadRetailLevel(library, levelName)));
  for (int i = 0; i < hudTicks; ++i) world.tick();
  // These entry fixtures precede release. Actors would require their own
  // animation/motion clocks, so refuse to silently fake a later gameplay frame.
  if (world.spawned() != 0 || !std::isfinite(openingMs) || openingMs < 0)
    throw std::runtime_error("hatch entry fixture is restricted to pre-release frames");
  // R2 corrects connected entry's compressed door expectation. Direct-level
  // diagnostics retain the old arm mapping. Zero-time control stays identical.
  const double arm = connectedEntry ? 2.799999952316284 : world.setup().releaseArmSeconds;
  // Y8: this expectation formerly encoded the static exit-balloon defect.
  // Sample its bob at the entry frame's admitted whole milliseconds, while
  // doors retain their independent sub-ms clock. Zero-time controls stay exact.
  const double levelSeconds = std::floor(openingMs + 1e-6) / 1000.0;
  backdrop.setEntranceExitPresentation(effects, arm > 0 ? openingMs / 1000.0 / arm : 1, 0,
                                      levelSeconds);
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
  const auto font = require(loadTexture(library, "TEXTURES\\^FONT.BMP"));
  // R6: the ordinary window's initial pointer is centered on this hatch.
  // Keep the historical headless closed control unchanged by opting in only
  // for expected window frames. Full-frame equality assertions remain exact.
  bool overHatch = false;
  if (hoverFeedback) {
    auto hatches = backdrop.hatchHoverTargets(camera, raster);
    if (const auto hit = app::hoveredHatch(hatches, image.width/2, image.height/2)) {
      auto& h = hatches[*hit];
      h.remaining = world.remainingFromEntrance(h.index);
      app::drawHatchHover(image, font, h);
      overHatch = true;
    }
  }
  const auto panel = require(app::RetailSkillPanel::create(library, levelName));
  auto icons = require(app::SkillPanelIconAnimation::create(library));
  // R7: the old fixed icon expectation encoded the playtest defect. Sample
  // explicit UI time; retain the zero-time control and exact full-frame checks.
  icons.advanceIcons(panelSeconds, 0, -1);
  panel.draw(image, panel.initialCounts(), 0, false, &icons);
  // Playtest 13 C6: the expected window frame includes the three HUD icons.
  // In this fixture, direct window samples request hover feedback and the
  // direct headless control requests neither connected entry nor hover.
  if (connectedEntry || hoverFeedback) {
    app::compositeIconRegion(image, require(loadTexture(library, "TEXTURES\\^OUT.BMP")),
                             {557, 15, 0, 0, 26, 32});
    app::compositeIconRegion(image, require(loadTexture(library, "TEXTURES\\^SAVED.BMP")),
                             {560, 53, 0, 0, 20, 32});
    app::compositeIconRegion(image, require(loadTexture(library, "TEXTURES\\^TIME.BMP")),
                             {579, 395, 0, 0, 26, 32});
  }
  app::drawRuntimeHud(image, font, world, false);
  const auto cursor = require(app::RuntimeCursor::load(library));
  // C7: initial hatch hover displays the installed entrance cursor.
  cursor.draw(image, overHatch ? app::CursorState::Entrance : app::CursorState::NoLemming,
              image.width / 2, image.height / 2);
  return image;
}
}  // namespace webo::test
