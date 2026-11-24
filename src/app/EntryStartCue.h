#pragma once

#include "assets/Bitmap.h"
#include "shell/MenuRender.h"

#include <array>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace webo::app {

// At 45C9AB/45CE7C/45E0E0, vertexAlpha records the negative-phase request.
// Fresh glyphs use 255 in the nonnegative phase. Pooled glyphs can retain alpha.
// The ordinary ^Font material uses texture alpha.
struct EntryCueFrame {
  std::array<int, 4> rect{};  // left, top, right, bottom
  bool additive = false;
  double vertexAlpha = 255.0;
};

struct PreparedEntryCue {
  struct Glyph { shell::GlyphCell source; std::array<int, 4> destination; };
  const Image* font;
  std::vector<Glyph> glyphs;
  bool additive;
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
  struct Presentation { PreparedEntryCue draw; bool playVoice; };
  Presentation preparePresentation(const Image& font, std::string_view text, const EntryCueFrame&);

 private:
  double seconds_;
  bool active_ = true;
  bool voicePending_ = true;
};

// The fit skips unsupported characters and zero-width cells.
std::vector<std::array<int, 4>> entryCueGlyphRects(std::string_view text, const EntryCueFrame& cue,
                                                    std::span<const shell::GlyphCell> cells = shell::fontCells());

PreparedEntryCue prepareEntryCue(const Image& font, std::string_view text, const EntryCueFrame&,
                                 std::span<const shell::GlyphCell> cells = shell::fontCells());
void drawEntryStartCue(Image&, const PreparedEntryCue&);

// Software approximation with POINT sampling and texture-alpha blending.
// The native sampler depends on renderer+3A1. Its active state is unresolved.
void drawEntryStartCue(webo::Image& target, const webo::Image& font, std::string_view text,
                       const EntryCueFrame& cue,
                       std::span<const shell::GlyphCell> cells = shell::fontCells());

}  // namespace webo::app
