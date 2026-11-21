// Compositing for the ten skill-panel buttons. The native registrar
// FUN_0045e51f at 0x45e51f-0x45ed80 includes GreyIcons, BlackIcons and
// AdditiveIcons. Each .SPR holds one action. SPRITES\GREYICONS.SPR has
// "bomb", SPRITES\BLACKICONS.SPR has "questionmark", and
// SPRITES\ADDITIVEICONS.SPR has "armageddon".

#pragma once

#include "assets/Bitmap.h"
#include "assets/Sprite.h"

#include <array>
#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <optional>
#include <vector>

namespace webo { class AssetLibrary; }

namespace webo::app {

// Source rectangle and destination origin in top-down image pixels.
struct SkillIconPlacement {
  int destX = 0;
  int destY = 0;
  int srcX = 0;
  int srcY = 0;
  int width = 0;
  int height = 0;
};

// Composite a top-down RGBA8 source rectangle with straight alpha.
// Clip both images together and return the number of written pixels.
// KeyBlack '_' and KeyGrey '$' supply binary alpha. AlfFile '^' copies
// the sibling .ALF plane and can supply intermediate alpha.
std::size_t compositeIconRegion(Image& dest, const Image& source, const SkillIconPlacement& placement);

struct SkillPanelRect { double left = 0, top = 0, right = 0, bottom = 0; };
// At 0x47A75D (0x47AA12/59), mirror=0 uses x-dx*scale and y-dy-height*scale.
// Scale is 1 for mode S=0 and 0.5 for S=-1.
SkillPanelRect skillPanelRect(const webo::SpriteFrame& frame, double x, double y, int scaleDelta);
// Use stored frame UVs and screen anchors. Additive sheets use ONE/ONE
// (Armageddon at 0x45A8D8). Other sheets use decoded straight alpha.
void drawSkillPanelIcon(webo::Image& dest, const webo::Image& sheet,
                        const webo::SpriteFrame& frame, double x, double y, int scaleDelta);
// Render two ASCII digits in white 6x14 glyphs.
void drawSkillPanelCounts(webo::Image& dest, const webo::Image& font,
                          const std::array<int, 9>& counts);
// Provisional ^tab upload uses p53-nearest endpoint bilinear resizing
// from 72x32 to 128x32, with full texture UVs. Native FPU state is unresolved.
webo::Image resizeSkillPlatePatch(const webo::Image& source);
// Provisional linear sample at s = x * 128 / 72 (request 2, LINEAR).
void drawSkillPanelPlates(webo::Image& dest, const webo::Image& plate);
// Loads and owns the panel resources. draw() accepts selection [0,9] or nullopt.
class SkillPanelIconAnimation;
class RetailSkillPanel {
 public:
  static std::expected<RetailSkillPanel, std::string> create(
      const webo::AssetLibrary& library, std::string_view levelId);
  void draw(webo::Image& dest, const std::array<int, 9>& counts,
            std::optional<int> selectedIndex) const;
  // PROVISIONAL clone feedback: plate 8 uses the selected artwork while fast;
  // the actor-skill selection remains independently highlighted.
  void draw(webo::Image& dest, const std::array<int, 9>& counts,
            std::optional<int> selectedIndex, bool fastForward) const;
  const std::array<int, 9>& initialCounts() const { return initialCounts_; }
  void draw(webo::Image& dest, const std::array<int, 9>& counts,
            std::optional<int> selectedIndex, bool fastForward,
            const SkillPanelIconAnimation* animation) const;

 private:
  RetailSkillPanel() = default;
  struct Icon {
    Image texture;
    webo::SpriteFrame frame;
    int x = 0, y = 0, scaleDelta = 0;
  };
  Image plate_, selectedPlate_, font_;
  std::array<int, 9> initialCounts_{};
  std::vector<Icon> icons_;
};

// Per-level icon clocks for skill-panel presentation.
class SkillPanelIconAnimation {
 public:
  static std::expected<SkillPanelIconAnimation, std::string> create(const AssetLibrary& library);
  // Original 45AF0E advances selection; hover animation is provisional feedback.
  void advanceIcons(double seconds, std::optional<int> selectedIndex, int hoveredIndex);
  void drawIcon(Image& dest, std::size_t index) const;
 private:
  struct Icon {
    struct Pose { SpriteFrame frame; std::size_t texture; };
    std::vector<Pose> poses;
    std::size_t idle = 0, current = 0;
    double rate = 0, phase = 0;
    bool active = false;
    int x = 0, y = 0, scaleDelta = 0;
  };
  std::vector<Icon> icons_;
  std::vector<Image> textures_;
};

// Headless overlay with level counts and selected index 0.
std::expected<void, std::string> drawRetailSkillPanel(webo::Image& dest,
                                                      const webo::AssetLibrary& library,
                                                      std::string_view levelId);

}  // namespace webo::app
