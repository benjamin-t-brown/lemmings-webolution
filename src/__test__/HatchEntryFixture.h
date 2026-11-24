// Compose the expected entry frame headlessly with explicit milliseconds.
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
#include <limits>
#include <stdexcept>

namespace webo::test {
inline Image hatchEntryFixture(const AssetLibrary& library,
                               const app::CameraParams& cameraParams,
                               int hudTicks, double openingMs, bool liveEntry = false,
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
  if (!std::isfinite(openingMs) || openingMs < 0 || openingMs > std::numeric_limits<int>::max())
    throw std::runtime_error("invalid hatch entry fixture time");
  const int levelMs = static_cast<int>(std::floor(openingMs + 1e-6));
  if (liveEntry) {
    // The live HUD consumes whole elapsed seconds. At 208 ms the World has
    // stepped once and the countdown still shows its initial value.
    world.advanceElapsedMs(levelMs);
  } else {
    for (int i = 0; i < hudTicks; ++i) world.tick();
  }
  // These entry fixtures precede release. Actors would require their own
  // animation/motion clocks, so refuse to silently fake a later gameplay frame.
  if (world.spawned() != 0)
    throw std::runtime_error("hatch entry fixture is restricted to pre-release frames");
  // Live entry uses the full clip. Pre-advanced diagnostics map it to the arm interval.
  const double arm = liveEntry ? 2.799999952316284 : world.setup().releaseArmSeconds;
  // Sample the exit balloon at whole milliseconds. Doors use a separate sub-millisecond clock.
  const double levelSeconds = levelMs / 1000.0;
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
  // Window frames start with the pointer centered on the hatch.
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
  // Sample the skill panel at explicit UI time.
  icons.advanceIcons(panelSeconds, 0, -1);
  panel.draw(image, panel.initialCounts(), 0, false, &icons);
  // Window frames with hover feedback include all three HUD icons.
  if (liveEntry || hoverFeedback) {
    app::compositeIconRegion(image, require(loadTexture(library, "TEXTURES\\^OUT.BMP")),
                             {557, 15, 0, 0, 26, 32});
    app::compositeIconRegion(image, require(loadTexture(library, "TEXTURES\\^SAVED.BMP")),
                             {560, 53, 0, 0, 20, 32});
    app::compositeIconRegion(image, require(loadTexture(library, "TEXTURES\\^TIME.BMP")),
                             {579, 395, 0, 0, 26, 32});
  }
  app::drawRuntimeHud(image, font, world, false);
  const auto cursor = require(app::RuntimeCursor::load(library));
  // Initial hatch hover uses the installed entrance cursor.
  cursor.draw(image, overHatch ? app::CursorState::Entrance : app::CursorState::NoLemming,
              image.width / 2, image.height / 2);
  return image;
}
}  // namespace webo::test
