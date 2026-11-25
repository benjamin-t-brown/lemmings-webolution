#pragma once

#include "app/EntryStartCue.h"
#include "app/HoverFeedback.h"
#include "app/SceneRaster.h"
#include "app/SoundMenu.h"
#include "shell/PauseCaption.h"

#include <utility>
#include <variant>

namespace webo::app {

// Layout and hover are resolved against the completed scene. Each record
// carries the operands for its compositor in presentation order.
struct PreparedLevelLayers {
  struct Text {
    const Image* font;
    std::vector<shell::HudGlyph> glyphs;
  };
  using Draw = std::variant<std::vector<HoverFragment>, PreparedSkillPanel, PreparedIconRegion,
      Text, shell::ScreenRect, PreparedEntryCue, shell::PreparedMenuRows, SoundMenu::Prepared,
      const Image*, shell::CaptionParams, std::vector<SceneHighlightId>>;
  std::vector<Draw> draws;

  void appendText(const Image& font, std::span<const shell::HudLine> lines) {
    Text text{&font, {}};
    shell::appendHudGlyphs(text.glyphs, lines, shell::fontCells());
    draws.emplace_back(std::move(text));
  }

  void draw(Image& image, std::span<const double> sceneDepth,
            std::span<const SceneHighlightPixel> highlights = {}) const {
    struct SoftwareDraw {
      Image& image;
      std::span<const double> depth;
      std::span<const SceneHighlightPixel> highlights;
      void operator()(const std::vector<HoverFragment>& draw) const { drawHoverFragments(image, draw); }
      void operator()(const PreparedSkillPanel& draw) const { draw.draw(image); }
      void operator()(const PreparedIconRegion& draw) const { compositeIconRegion(image, *draw.texture, draw.placement); }
      void operator()(const Text& draw) const { shell::drawHudGlyphs(image, *draw.font, draw.glyphs, depth); }
      void operator()(const shell::ScreenRect& draw) const { shell::drawScreenRect(image, draw); }
      void operator()(const PreparedEntryCue& draw) const { drawEntryStartCue(image, draw); }
      void operator()(const shell::PreparedMenuRows& draw) const { shell::drawMenuRows(image, draw); }
      void operator()(const SoundMenu::Prepared& draw) const { draw.draw(image); }
      void operator()(const Image* draw) const { image = *draw; }
      void operator()(const shell::CaptionParams& draw) const { shell::drawCaptionPanel(image, draw); }
      void operator()(const std::vector<SceneHighlightId>& targets) const {
        for (std::size_t p = 0; p < highlights.size(); ++p)
          if (std::ranges::find(targets, highlights[p].owner) != targets.end())
            addHoverRgb(image.rgba.data() + 4 * p, highlights[p].rgb.data());
      }
    };
    for (const auto& draw : draws) std::visit(SoftwareDraw{image, sceneDepth, highlights}, draw);
  }
};

}  // namespace webo::app
