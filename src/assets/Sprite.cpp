#include "Sprite.h"

#include "Box.h"

#include "lemmings/sprite_resource.hpp"

#include <bit>
#include <cmath>
#include <format>
#include <stdexcept>

namespace webo {
namespace {

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
  lemmings::SpriteResource resource;
  try {
    // The caller owns data throughout this synchronous parse. Only serialized
    // values are recovered here; no original runtime objects are constructed.
    resource = lemmings::parse_sprite_resource(
        {reinterpret_cast<const std::uint8_t*>(data.data()), data.size()});
  } catch (const std::invalid_argument& error) {
    return std::unexpected(error.what());
  }

  SpriteSheet sheet;
  sheet.sprites.reserve(resource.clips.size());
  for (auto& clip : resource.clips) {
    Sprite sprite;
    sprite.name = std::move(clip.name);
    sprite.rate = std::bit_cast<double>(clip.speed_bits_10);
    sprite.animations.reserve(clip.frames.size());
    for (auto& recoveredFrame : clip.frames) {
      SpriteAnimation animation;
      animation.frames.reserve(recoveredFrame.pieces.size());
      for (auto& piece : recoveredFrame.pieces) {
        SpriteFrame frame;
        frame.bottom = piece.values_00[0];
        frame.top = piece.values_00[1];
        frame.left = piece.values_00[2];
        frame.right = piece.values_00[3];
        frame.uBottom = std::bit_cast<double>(piece.double_bits_10[0]);
        frame.uTop = std::bit_cast<double>(piece.double_bits_10[1]);
        frame.uLeft = std::bit_cast<double>(piece.double_bits_10[2]);
        frame.uRight = std::bit_cast<double>(piece.double_bits_10[3]);
        frame.unknown0 = piece.values_30[0];
        frame.unknown1 = piece.values_30[1];
        frame.width = piece.values_30[2];
        frame.height = piece.values_30[3];
        frame.sheet = std::move(piece.texture_name);
        animation.frames.push_back(std::move(frame));
      }
      sprite.animations.push_back(std::move(animation));
    }
    sheet.sprites.push_back(std::move(sprite));
  }

  // Compatibility projection: the recovered tag dispatch has already selected
  // 16 triples for tag 0, 256 for tag 1, and none for other tags. Flatten only
  // populated triples in block order; their runtime meaning remains separate.
  for (const auto& block : resource.triple_arrays)
    for (const auto& triple : block.triples)
      sheet.palette.push_back({std::bit_cast<double>(triple[0]),
                               std::bit_cast<double>(triple[1]),
                               std::bit_cast<double>(triple[2])});

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
