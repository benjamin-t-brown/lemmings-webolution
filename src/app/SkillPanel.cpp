#include "app/SkillPanel.h"
#include "app/LevelLayout.h"

#include "assets/Box.h"
#include "assets/LevelObjects.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <map>
#include <optional>

namespace webo::app {

std::size_t compositeIconRegion(Image& dest, const Image& source, const SkillIconPlacement& placement) {
  if (dest.empty() || source.empty() || placement.width <= 0 || placement.height <= 0)
    return 0;

  // Clip source and destination together to keep their pixel offsets aligned.
  int srcX0 = placement.srcX;
  int srcY0 = placement.srcY;
  int destX0 = placement.destX;
  int destY0 = placement.destY;
  int width = placement.width;
  int height = placement.height;

  if (srcX0 < 0) {
    width += srcX0;
    destX0 -= srcX0;
    srcX0 = 0;
  }
  if (srcY0 < 0) {
    height += srcY0;
    destY0 -= srcY0;
    srcY0 = 0;
  }
  if (destX0 < 0) {
    width += destX0;
    srcX0 -= destX0;
    destX0 = 0;
  }
  if (destY0 < 0) {
    height += destY0;
    srcY0 -= destY0;
    destY0 = 0;
  }
  width = std::min({width, source.width - srcX0, dest.width - destX0});
  height = std::min({height, source.height - srcY0, dest.height - destY0});
  if (width <= 0 || height <= 0)
    return 0;

  std::size_t written = 0;
  for (int row = 0; row < height; ++row) {
    const std::size_t srcRowBase = (static_cast<std::size_t>(srcY0 + row) * source.width + srcX0) * 4;
    const std::size_t destRowBase = (static_cast<std::size_t>(destY0 + row) * dest.width + destX0) * 4;
    for (int col = 0; col < width; ++col) {
      const std::uint8_t* s = source.rgba.data() + srcRowBase + static_cast<std::size_t>(col) * 4;
      std::uint8_t* d = dest.rgba.data() + destRowBase + static_cast<std::size_t>(col) * 4;
      const std::uint8_t a = s[3];
      if (a == 0) {
        // Fully transparent: dest keeps whatever it already had.
      } else if (a == 255) {
        d[0] = s[0];
        d[1] = s[1];
        d[2] = s[2];
        d[3] = 255;
      } else {
        // Blend straight alpha, as supplied by decodeBmp and applyAlfAlpha.
        for (int c = 0; c < 3; ++c) {
          const int blended = (static_cast<int>(s[c]) * a + static_cast<int>(d[c]) * (255 - a)) / 255;
          d[c] = static_cast<std::uint8_t>(blended);
        }
        d[3] = static_cast<std::uint8_t>(std::max<int>(d[3], a));
      }
      ++written;
    }
  }
  return written;
}

SkillPanelRect skillPanelRect(const webo::SpriteFrame& f, double x, double y, int scaleDelta) {
  const double scale = scaleDelta == 0 ? 1.0 : scaleDelta < 0 ? 0.5 : 2.0;
  const double left = x - f.unknown0 * scale;
  // Unmirrored Y keeps the full dy offset. Only frame height is scaled (0x47AA59..AA86).
  const double top = y - f.unknown1 - f.height * scale;
  return {left, top, left + f.width * scale, top + f.height * scale};
}

namespace {

void drawIconRect(Image& dest, const Image& sheet, const SpriteFrame& f, SkillPanelRect r) {
  if (dest.empty() || sheet.empty() || f.width == 0 || f.height == 0) return;
  const int x0 = std::max(0, static_cast<int>(std::ceil(r.left)));
  const int x1 = std::min(dest.width, static_cast<int>(std::ceil(r.right)));
  const int y0 = std::max(0, static_cast<int>(std::ceil(r.top)));
  const int y1 = std::min(dest.height, static_cast<int>(std::ceil(r.bottom)));
  const bool additive = sheet.alpha == AlphaMode::Additive;
  for (int py = y0; py < y1; ++py) {
    const double v = (py - r.top) / (r.bottom - r.top);
    const int sy = std::clamp(static_cast<int>(std::floor((f.uTop + v * (f.uBottom - f.uTop)) * sheet.height)),
                              static_cast<int>(f.top), static_cast<int>(f.bottom) - 1);
    for (int px = x0; px < x1; ++px) {
      const double u = (px - r.left) / (r.right - r.left);
      const int sx = std::clamp(static_cast<int>(std::floor((f.uLeft + u * (f.uRight - f.uLeft)) * sheet.width)),
                                static_cast<int>(f.left), static_cast<int>(f.right) - 1);
      const auto* src = sheet.rgba.data() + (static_cast<std::size_t>(sy) * sheet.width + sx) * 4;
      auto* dst = dest.rgba.data() + (static_cast<std::size_t>(py) * dest.width + px) * 4;
      if (additive) {
        // Armageddon's ~Additive0 texture uses selector 2 (ONE/ONE, 0x45A8D8 -> 0x414D80).
        for (int c = 0; c < 3; ++c)
          dst[c] = static_cast<std::uint8_t>(std::min(255, dst[c] + src[c]));
      } else {
        const int a = src[3];
        for (int c = 0; c < 3; ++c)
          dst[c] = static_cast<std::uint8_t>((dst[c] * (255 - a) + src[c] * a + 127) / 255);
      }
    }
  }
}

void drawPortraitIcon(Image& dest, const Image& sheet, const SpriteFrame& frame, std::size_t index,
                      LevelLayout layout) {
  constexpr double scales[] = {0.81, 0.59, 1.0, 0.76, 0.79, 0.78, 0.68, 0.85, 1.2, 1.25};
  const auto plate = skillPlate(static_cast<int>(index), layout);
  const double x = plate.x + (index >= 8 ? 23 : 29), y = plate.y + plate.height / 2.0;
  const double halfWidth = frame.width * scales[index] / 2;
  const double halfHeight = frame.height * scales[index] / 2;
  drawIconRect(dest, sheet, frame, {x - halfWidth, y - halfHeight, x + halfWidth, y + halfHeight});
}

void drawPanelCount(Image& dest, const Image& font, int count, int left, int top,
                    int digitWidth = 6, int digitHeight = 14) {
  if (count == 0) return; // Native skips text for zero stock.
  const int digits[] = {count / 10, count % 10};
  for (int d = 0; d < 2; ++d) {
    for (int y = 0; y < digitHeight; ++y) for (int x = 0; x < digitWidth; ++x) {
      const int sx = digits[d] * 6 + x * 6 / digitWidth, sy = y * 14 / digitHeight;
      const int dx = left + d * digitWidth + x, dy = top + y;
      if (sx < 0 || sx >= font.width || sy >= font.height ||
          dx < 0 || dx >= dest.width || dy < 0 || dy >= dest.height) continue;
      const auto* src = font.rgba.data() + (static_cast<std::size_t>(sy) * font.width + sx) * 4;
      auto* dst = dest.rgba.data() + (static_cast<std::size_t>(dy) * dest.width + dx) * 4;
      const int a = src[3];
      for (int c = 0; c < 3; ++c) dst[c] = static_cast<std::uint8_t>((dst[c] * (255 - a) + 255 * a + 127) / 255);
    }
  }
}

}  // namespace

void drawSkillPanelIcon(Image& dest, const Image& sheet, const SpriteFrame& frame,
                        double x, double y, int scaleDelta) {
  drawIconRect(dest, sheet, frame, skillPanelRect(frame, x, y, scaleDelta));
}

void drawSkillPanelCounts(Image& dest, const Image& font, const std::array<int, 9>& counts) {
  for (int i = 0; i < 9; ++i)
    drawPanelCount(dest, font, counts[static_cast<std::size_t>(i)], 66, 20 + 40 * i);
}

Image resizeSkillPlatePatch(const Image& source) {
  if (source.width != 72 || source.height != 32 || source.rgba.size() != 72u * 32u * 4u) return {};
  Image out; out.width = 128; out.height = 32; out.alpha = source.alpha;
  out.rgba.resize(128u * 32u * 4u);
  for (int y = 0; y < 32; ++y) for (int x = 0; x < 128; ++x) {
    // Provisional x87 model computes in p53-nearest, then stores binary32.
    // The original entry FPU state is unresolved.
    const float fx = static_cast<float>(static_cast<double>(x) / 127.0 * 71.0);
    const int x0 = static_cast<int>(fx), x1 = std::min(x0 + 1, 71);
    const float tx = static_cast<float>(static_cast<double>(fx) - x0);
    const float gx = static_cast<float>(1.0 - static_cast<double>(tx));
    for (int c = 0; c < 4; ++c) {
      const auto at = [&](int xx) { return source.rgba[(static_cast<std::size_t>(y) * 72 + xx) * 4 + c]; };
      const double value = static_cast<double>(at(x0)) * gx + static_cast<double>(at(x1)) * tx;
      out.rgba[(static_cast<std::size_t>(y) * 128 + x) * 4 + c] = static_cast<std::uint8_t>(value);
    }
  }
  return out;
}

namespace {

// Provisional horizontal linear sampling uses s = x * 128 / 72 with
// integer texel centres. Nearest uses min(127, (x * 128 + 36) / 72).
// D3D request 2 denotes LINEAR in d3dtypes.h. Y stays on the source row.
void drawSkillPanelPlateQuad(Image& dest, const Image& plate, LevelButton bounds) {
  const auto [left, top, width, height] = bounds;
  for (int y = 0; y < height && top + y < dest.height; ++y) for (int x = 0; x < width && left + x < dest.width; ++x) {
    const int sy = y * 32 / height;
    const int numer = x * 128;
    const int sx0 = std::min(127, numer / width);
    const int sx1 = std::min(127, sx0 + 1);
    const int rem = numer % width;
    const int w0 = width - rem;
    const auto* t0 = plate.rgba.data() + (static_cast<std::size_t>(sy) * 128 + sx0) * 4;
    const auto* t1 = plate.rgba.data() + (static_cast<std::size_t>(sy) * 128 + sx1) * 4;
    unsigned sample[4];
    for (int c = 0; c < 4; ++c)
      sample[c] = static_cast<unsigned>((t0[c] * w0 + t1[c] * rem) / width);
    auto* d = dest.rgba.data() + (static_cast<std::size_t>(top + y) * dest.width + left + x) * 4;
    const unsigned a = sample[3];
    for (int c = 0; c < 3; ++c)
      d[c] = static_cast<std::uint8_t>((sample[c] * a + d[c] * (255 - a)) / 255);
  }
}

}  // namespace

void drawSkillPanelPlates(Image& dest, const Image& plate) {
  for (int i = 0; i < 10; ++i)
    drawSkillPanelPlateQuad(dest, plate, skillPlate(i));
}

// The headless overlay starts enabled at index 0, rectangle 18,12,90,44.
void drawSkillPanelSelectedPlate(Image& dest, const Image& plate) {
  drawSkillPanelPlateQuad(dest, plate, skillPlate(0));
}

std::expected<RetailSkillPanel, std::string> RetailSkillPanel::create(
    const AssetLibrary& library, std::string_view levelId) {
  auto lvl = library.read("COMPILED\\" + std::string(levelId) + ".LVL");
  auto spriteBytes = library.read("SPRITES\\LEMMINGSHI.SPR");
  auto font = loadTexture(library, "TEXTURES\\_FONTSMALL.BMP");
  auto plateSource = loadTexture(library, "TEXTURES\\^TAB.BMP");
  auto selectedSource = loadTexture(library, "TEXTURES\\^TAB_SELECTED.BMP");
  if (!lvl || !spriteBytes || !font || !plateSource || !selectedSource)
    return std::unexpected(!lvl ? lvl.error() : (!spriteBytes ? spriteBytes.error() : (!font ? font.error() : (!plateSource ? plateSource.error() : selectedSource.error()))));
  auto skills = findLevelSkills(*lvl);
  auto spriteSheet = parseSpriteSheet(*spriteBytes);
  if (!skills || !spriteSheet)
    return std::unexpected(!skills ? skills.error() : spriteSheet.error());

  // Plate anchors are screen coordinates passed to 0x47A75D.
  constexpr int drawX[10] = {39,37,38,39,40,39,41,37,38,38};
  constexpr int drawY[10] = {40,92,121,158,197,238,279,317,364,410};
  // Controller fields are temporal sub-entry (+1CC) and selected frame (+1B8).
  constexpr int sub[10] = {4,0,0,0,15,0,21,1,0,6};
  constexpr int frameNo[10] = {2,4,0,4,2,2,2,4,0,0};
  // Skill action names and special sheet registrations.
  constexpr const char* names[10] = {"climb","float","bomb","block","build","bash","mine","dig","questionmark","armageddon"};
  constexpr const char* specialSpr[3] = {"SPRITES\\GREYICONS.SPR","SPRITES\\BLACKICONS.SPR","SPRITES\\ADDITIVEICONS.SPR"};

  RetailSkillPanel panel;
  for (std::size_t i = 0; i < skills->counts.size(); ++i)
    panel.initialCounts_[i] = static_cast<int>(skills->counts[i]); // READ: level .LVL trailer, findLevelSkills.
  panel.initialCounts_[8] = 0; // Default is zero, including MM_CLIMB_002. The native MM_DIG_000 value is unresolved.
  // Provisional overlay resizes and samples base and selected plates from ^tab.
  Image plate = resizeSkillPlatePatch(*plateSource);
  if (plate.empty()) return std::unexpected("skill-panel plate must be 72x32");
  Image selected = resizeSkillPlatePatch(*selectedSource);
  if (selected.empty()) return std::unexpected("skill-panel selected plate must be 72x32");
  panel.plate_ = std::move(plate);
  panel.selectedPlate_ = std::move(selected);
  panel.font_ = std::move(*font);

  for (int i = 0; i < 10; ++i) {
    const Sprite* sprite = nullptr;
    const SpriteSheet* selectedSheet = &*spriteSheet;
    std::optional<SpriteSheet> special;
    if (i == 2 || i == 8 || i == 9) {
      auto raw = library.read(specialSpr[i == 2 ? 0 : i == 8 ? 1 : 2]);
      if (!raw) return std::unexpected(raw.error());
      auto parsed = parseSpriteSheet(*raw);
      if (!parsed) return std::unexpected(parsed.error());
      special.emplace(std::move(*parsed));
      selectedSheet = &*special;
    }
    sprite = selectedSheet->find(names[i]);
    if (!sprite || static_cast<std::size_t>(sub[i]) >= sprite->animations.size())
      return std::unexpected(std::format("skill-panel sprite/animation missing {}/{}", names[i], sub[i]));
    const auto& frames = sprite->animations[static_cast<std::size_t>(sub[i])].frames;
    const auto fi = static_cast<std::size_t>(frameNo[i]);
    if (fi >= frames.size())
      return std::unexpected(std::format("skill-panel frame missing {}/{}", names[i], fi));
    const auto& frame = frames[fi];
    // Resolve the frame's sheet through the Sprite.h name mapping.
    auto texture = loadTexture(library, webo::sheetAssetName(frame.sheet));
    if (!texture)
      return std::unexpected(std::format("{}: {}", frame.sheet, texture.error()));
    // Generic icons use mode S=-1. Plates 2/8/9 use S=0.
    panel.icons_.push_back({std::move(*texture), frame, drawX[i], drawY[i],
                            (i == 2 || i == 8 || i == 9) ? 0 : -1});
  }
  return panel;
}

std::expected<SkillPanelIconAnimation, std::string> SkillPanelIconAnimation::create(const AssetLibrary& library) {
  auto sheet = loadSpriteSheet(library, "SPRITES\\LEMMINGSHI.SPR");
  auto bomb = loadSpriteSheet(library, "SPRITES\\GREYICONS.SPR");
  if (!sheet || !bomb) return std::unexpected(!sheet ? sheet.error() : bomb.error());
  // Provisional idle poses use the panel's preset anchors and facing. Climber
  // uses entry 4 here. The reset at 45AC5C selects entry 3.
  constexpr const char* names[] = {"climb","float","bomb","block","build","bash","mine","dig"};
  constexpr int idle[] = {4,0,0,0,15,0,21,1};
  constexpr int facing[] = {2,4,0,4,2,2,2,4};
  constexpr int x[] = {39,37,38,39,40,39,41,37};
  constexpr int y[] = {40,92,121,158,197,238,279,317};
  SkillPanelIconAnimation result;
  std::map<std::string, std::size_t> textures;
  for (int i = 0; i < 8; ++i) {
    const auto* sprite = (i == 2 ? *bomb : *sheet).find(names[i]);
    if (!sprite || std::size_t(idle[i]) >= sprite->animations.size())
      return std::unexpected(std::format("panel animation missing {}", names[i]));
    Icon icon;
    icon.idle = icon.current = idle[i]; icon.rate = sprite->rate;
    icon.x = x[i]; icon.y = y[i]; icon.scaleDelta = i == 2 ? 0 : -1;
    for (const auto& entry : sprite->animations) {
      if (std::size_t(facing[i]) >= entry.frames.size())
        return std::unexpected(std::format("panel facing missing {}", names[i]));
      const auto& frame = entry.frames[facing[i]];
      auto found = textures.find(frame.sheet);
      if (found == textures.end()) {
        auto texture = loadTexture(library, sheetAssetName(frame.sheet));
        if (!texture) return std::unexpected(texture.error());
        found = textures.emplace(frame.sheet, result.textures_.size()).first;
        result.textures_.push_back(std::move(*texture));
      }
      icon.poses.push_back({frame, found->second});
    }
    result.icons_.push_back(std::move(icon));
  }
  return result;
}

void SkillPanelIconAnimation::drawIcon(Image& dest, std::size_t index, LevelLayout layout) const {
  if (index >= icons_.size()) return;
  const auto& icon = icons_[index];
  const auto& pose = icon.poses[icon.current];
  if (layout.portrait()) drawPortraitIcon(dest, textures_[pose.texture], pose.frame, index, layout);
  else drawSkillPanelIcon(dest, textures_[pose.texture], pose.frame, icon.x, icon.y, icon.scaleDelta);
}

void SkillPanelIconAnimation::advanceIcons(double seconds, std::optional<int> selectedIndex, int hoveredIndex) {
  if (!std::isfinite(seconds) || seconds < 0) return;
  for (std::size_t i = 0; i < icons_.size(); ++i) {
    auto& icon = icons_[i];
    // 45AFCD..45AFE0 advances the selected icon through 468833.
    // Provisional feedback also animates hovered skills and resets them on leave.
    const bool active = i < 8 && ((selectedIndex && *selectedIndex == static_cast<int>(i)) ||
                                  hoveredIndex == static_cast<int>(i));
    if (!active || !icon.active) { icon.current = icon.idle; icon.phase = 0; }
    icon.active = active;
    if (!active || icon.poses.size() <= 1 || !std::isfinite(icon.rate) || icon.rate <= 0) continue;
    // 46889E..4688E6 accumulates dt*rate and steps strictly above 0.06.
    // Provisional playback loops and resets the fraction on activation.
    // Reduce whole cycles before multiplying to bound work on stalls.
    const double cycle = 0.06 * icon.poses.size();
    icon.phase += std::fmod(seconds, cycle / icon.rate) * icon.rate;
    while (icon.phase > 0.06) {
      icon.current = (icon.current + 1) % icon.poses.size();
      icon.phase -= 0.06;
      if (icon.phase <= 9.99999993922529e-09) icon.phase = 0;
    }
  }
}

void RetailSkillPanel::draw(Image& dest, const std::array<int, 9>& counts,
                            std::optional<int> selectedIndex) const {
  draw(dest, counts, selectedIndex, false);
}

void RetailSkillPanel::draw(Image& dest, const std::array<int, 9>& counts,
                            std::optional<int> selectedIndex, bool fastForward) const {
  draw(dest, counts, selectedIndex, fastForward, nullptr);
}

void RetailSkillPanel::draw(Image& dest, const std::array<int, 9>& counts,
                            std::optional<int> selectedIndex, bool fastForward,
                            const SkillPanelIconAnimation* animation, LevelLayout layout) const {
  for (int i = 0; i < 10; ++i)
    drawSkillPanelPlateQuad(dest, plate_, skillPlate(i, layout));
  if (selectedIndex && *selectedIndex >= 0 && *selectedIndex < 10)
    drawSkillPanelPlateQuad(dest, selectedPlate_, skillPlate(*selectedIndex, layout));
  if (fastForward) drawSkillPanelPlateQuad(dest, selectedPlate_, skillPlate(8, layout));
  if (layout.portrait()) {
    for (int i = 0; i < 8; ++i) {
      const auto plate = skillPlate(i, layout);
      drawPanelCount(dest, font_, counts[i], plate.x + 57, plate.y + 15, 8, 18);
    }
  } else drawSkillPanelCounts(dest, font_, counts);
  for (std::size_t i = 0; i < icons_.size(); ++i) {
    if (animation && i < 8) animation->drawIcon(dest, i, layout);
    else {
      const auto& icon = icons_[i];
      if (layout.portrait()) drawPortraitIcon(dest, icon.texture, icon.frame, i, layout);
      else drawSkillPanelIcon(dest, icon.texture, icon.frame, icon.x, icon.y, icon.scaleDelta);
    }
  }
}

std::expected<void, std::string> drawRetailSkillPanel(Image& dest, const AssetLibrary& library,
                                                       std::string_view levelId) {
  auto panel = RetailSkillPanel::create(library, levelId);
  if (!panel) return std::unexpected(panel.error());
  panel->draw(dest, panel->initialCounts(), 0);
  return {};
}

}  // namespace webo::app
