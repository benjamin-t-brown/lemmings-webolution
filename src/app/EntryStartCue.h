#pragma once

#include "assets/Bitmap.h"
#include "shell/MenuRender.h"

#include <array>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace webo::app {

// READ45C9AB/45CE7C/45E0E0, docs/scratch/level-entry-start-cue.md.
// vertexAlpha records the negative-phase request; nonnegative255 describes
// fresh glyph allocation only (native pooled glyphs may retain old alpha).
// The ordinary ^Font material selects texture alpha, so this is not opacity.
struct EntryCueFrame {
  std::array<int, 4> rect{};  // left, top, right, bottom
  bool additive = false;
  double vertexAlpha = 255.0;
};

class EntryStartCue {
 public:
  explicit EntryStartCue(bool introEnabled = true);

  // The caller supplies frame time, not simulation time. Invalid/negative
  // deltas are ignored. Non-playing states freeze the clock but may still draw.
  void advanceBeforeFrame(double seconds, bool playing);
  double seconds() const { return seconds_; }
  bool active() const { return active_; }
  std::optional<EntryCueFrame> frame(bool levelScenePresent = true) const;

 private:
  double seconds_;
  bool active_ = true;
};

// Useful independently for checking the native proportional cumulative fit.
// Unsupported/non-ASCII characters and zero-width cells are omitted.
std::vector<std::array<int, 4>> entryCueGlyphRects(std::string_view text, const EntryCueFrame& cue,
                                                    std::span<const shell::GlyphCell> cells = shell::fontCells());

// A bounded software approximation: POINT-sampled texture pixels, source
// alpha for source-over/additive blending, and no filled background. Native
// sampler state depends on renderer+3A1 and is not established by static
// evidence; this routine does not reproduce that conditional state.
void drawEntryStartCue(webo::Image& target, const webo::Image& font, std::string_view text,
                       const EntryCueFrame& cue,
                       std::span<const shell::GlyphCell> cells = shell::fontCells());

}  // namespace webo::app
