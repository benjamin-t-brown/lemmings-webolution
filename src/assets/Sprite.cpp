#include "Sprite.h"

#include "Box.h"

#include <cmath>
#include <cstring>
#include <format>

namespace webo {
namespace {

constexpr std::size_t kPaletteDoubles = 48;  // 16 RGB triples
constexpr std::uint32_t kMaxCount = 1u << 20;

class Reader {
public:
  explicit Reader(std::span<const std::byte> d) : d_(d) {}

  std::uint32_t u32() {
    if (!need(4))
      return 0;
    std::uint32_t v;
    std::memcpy(&v, d_.data() + pos_, 4);
    pos_ += 4;
    return v;
  }
  double f64() {
    if (!need(8))
      return 0;
    double v;
    std::memcpy(&v, d_.data() + pos_, 8);
    pos_ += 8;
    return v;
  }
  std::string str() {
    const std::uint32_t len = u32();
    if (!ok_)
      return {};
    if (len > 4096) {
      fail(std::format("implausible string length {} at offset {}", len, pos_ - 4));
      return {};
    }
    if (!need(len))
      return {};
    std::string s(reinterpret_cast<const char*>(d_.data() + pos_), len);
    pos_ += len;
    return s;
  }

  bool ok() const { return ok_; }
  std::size_t pos() const { return pos_; }
  std::size_t size() const { return d_.size(); }
  const std::string& error() const { return error_; }

  void fail(std::string what) {
    if (ok_) {
      ok_ = false;
      error_ = std::move(what);
    }
  }

private:
  bool need(std::size_t n) {
    if (!ok_)
      return false;
    if (pos_ + n > d_.size()) {
      fail(std::format("ran past the end of the file at offset {}", pos_));
      return false;
    }
    return true;
  }

  std::span<const std::byte> d_;
  std::size_t pos_ = 0;
  bool ok_ = true;
  std::string error_;
};

std::uint32_t recoverDimension(std::uint32_t pixels, double normalised) {
  if (normalised <= 0.0)
    return 0;
  return static_cast<std::uint32_t>(std::lround(pixels / normalised));
}

}  // namespace

std::uint32_t SpriteFrame::sheetWidth() const {
  // Prefer the right edge; it is non-zero far more often than the left.
  if (const std::uint32_t w = recoverDimension(right, uRight))
    return w;
  return recoverDimension(left, uLeft);
}

std::uint32_t SpriteFrame::sheetHeight() const {
  if (const std::uint32_t h = recoverDimension(bottom, uBottom))
    return h;
  return recoverDimension(top, uTop);
}

std::size_t Sprite::frameCount() const {
  std::size_t n = 0;
  for (const auto& a : animations)
    n += a.frames.size();
  return n;
}

const Sprite* SpriteSheet::find(std::string_view name) const {
  for (const auto& s : sprites)
    if (s.name == name)
      return &s;
  return nullptr;
}

std::size_t SpriteSheet::frameCount() const {
  std::size_t n = 0;
  for (const auto& s : sprites)
    n += s.frameCount();
  return n;
}

std::string sheetAssetName(std::string_view sheet) {
  std::string out = "TEXTURES\\";
  out += normaliseAssetName(sheet);
  out += ".BMP";
  return out;
}

std::expected<SpriteSheet, std::string> parseSpriteSheet(std::span<const std::byte> data) {
  Reader r(data);
  SpriteSheet sheet;

  const std::uint32_t spriteCount = r.u32();
  if (!r.ok())
    return std::unexpected(r.error());
  if (spriteCount > kMaxCount)
    return std::unexpected(std::format("implausible sprite count {}", spriteCount));

  sheet.sprites.reserve(spriteCount);
  for (std::uint32_t s = 0; s < spriteCount; ++s) {
    Sprite sprite;
    const std::uint32_t animCount = r.u32();
    if (!r.ok())
      return std::unexpected(r.error());
    if (animCount > kMaxCount)
      return std::unexpected(std::format("sprite {}: implausible animation count {}", s, animCount));

    sprite.animations.reserve(animCount);
    for (std::uint32_t a = 0; a < animCount; ++a) {
      SpriteAnimation anim;
      const std::uint32_t frameCount = r.u32();
      if (!r.ok())
        return std::unexpected(r.error());
      if (frameCount > kMaxCount)
        return std::unexpected(
            std::format("sprite {} animation {}: implausible frame count {}", s, a, frameCount));

      anim.frames.reserve(frameCount);
      for (std::uint32_t f = 0; f < frameCount; ++f) {
        SpriteFrame frame;
        // Y first, then X: dividing each integer by its normalised twin
        // recovers the sheet size, and only this order gives the sheet's
        // actual dimensions rather than them transposed.
        frame.bottom = r.u32();
        frame.top = r.u32();
        frame.left = r.u32();
        frame.right = r.u32();
        frame.uBottom = r.f64();
        frame.uTop = r.f64();
        frame.uLeft = r.f64();
        frame.uRight = r.f64();
        frame.unknown0 = r.u32();
        frame.unknown1 = r.u32();
        frame.width = r.u32();
        frame.height = r.u32();
        frame.sheet = r.str();
        if (!r.ok())
          return std::unexpected(r.error());
        anim.frames.push_back(std::move(frame));
      }
      sprite.animations.push_back(std::move(anim));
    }

    sprite.name = r.str();
    sprite.rate = r.f64();
    if (!r.ok())
      return std::unexpected(r.error());
    sheet.sprites.push_back(std::move(sprite));
  }

  // A trailing palette, present only on the lemming sheets.
  const std::uint32_t paletteCount = r.u32();
  if (!r.ok())
    return std::unexpected(r.error());
  if (paletteCount > 16)
    return std::unexpected(std::format("implausible palette count {}", paletteCount));

  for (std::uint32_t p = 0; p < paletteCount; ++p) {
    r.u32();  // block header; zero in every retail file
    for (std::size_t i = 0; i < kPaletteDoubles / 3; ++i) {
      PaletteColour c{};
      c[0] = r.f64();
      c[1] = r.f64();
      c[2] = r.f64();
      if (!r.ok())
        return std::unexpected(r.error());
      sheet.palette.push_back(c);
    }
  }

  if (r.pos() != r.size())
    return std::unexpected(
        std::format("consumed {} of {} bytes -- the layout does not fit", r.pos(), r.size()));

  return sheet;
}

std::expected<SpriteSheet, std::string> loadSpriteSheet(const AssetLibrary& lib,
                                                        std::string_view name) {
  auto bytes = lib.read(name);
  if (!bytes)
    return std::unexpected(bytes.error());
  auto parsed = parseSpriteSheet(*bytes);
  if (!parsed)
    return std::unexpected(std::format("{}: {}", name, parsed.error()));
  return parsed;
}

std::vector<std::string> spriteSheetNames(const AssetLibrary& lib) {
  std::vector<std::string> out;
  for (const auto& name : lib.names()) {
    const auto upper = normaliseAssetName(name);
    if (upper.starts_with("SPRITES\\") && upper.ends_with(".SPR"))
      out.push_back(name);
  }
  return out;
}

}  // namespace webo
