#include "Bitmap.h"

#include "Box.h"
#include "Lrz.h"

#include <cstring>
#include <format>

namespace webo {
namespace {

std::uint16_t le16(const std::byte* p) {
  std::uint16_t v;
  std::memcpy(&v, p, 2);
  return v;
}
std::uint32_t le32(const std::byte* p) {
  std::uint32_t v;
  std::memcpy(&v, p, 4);
  return v;
}
std::int32_t lei32(const std::byte* p) {
  std::int32_t v;
  std::memcpy(&v, p, 4);
  return v;
}

}  // namespace

std::string_view alphaModeName(AlphaMode m) {
  switch (m) {
    case AlphaMode::Opaque: return "opaque";
    case AlphaMode::AlfFile: return "alf";
    case AlphaMode::Embedded: return "embedded";
    case AlphaMode::KeyBlack: return "key-black";
    case AlphaMode::KeyGrey: return "key-grey";
    case AlphaMode::Additive: return "additive";
  }
  return "?";
}

AlphaMode alphaModeFor(std::string_view assetName) {
  const auto slash = assetName.find_last_of("\\/");
  const std::size_t start = (slash == std::string_view::npos) ? 0 : slash + 1;
  if (start >= assetName.size())
    return AlphaMode::Opaque;
  switch (assetName[start]) {
    case '^': return AlphaMode::AlfFile;
    case '+': return AlphaMode::Embedded;
    case '_': return AlphaMode::KeyBlack;
    case '$': return AlphaMode::KeyGrey;
    case '~': return AlphaMode::Additive;
    default: return AlphaMode::Opaque;
  }
}

std::expected<Image, std::string> decodeBmp(std::span<const std::byte> bmp) {
  if (bmp.size() < 54)
    return std::unexpected("too small to be a BMP");
  const auto* p = bmp.data();
  if (std::to_integer<char>(p[0]) != 'B' || std::to_integer<char>(p[1]) != 'M')
    return std::unexpected("missing BM signature");

  const std::uint32_t offBits = le32(p + 10);
  const std::uint32_t hdrSize = le32(p + 14);
  if (hdrSize < 40)
    return std::unexpected(std::format("unsupported DIB header size {}", hdrSize));

  const std::int32_t width = lei32(p + 18);
  const std::int32_t rawHeight = lei32(p + 22);
  const std::uint16_t bpp = le16(p + 28);
  const std::uint32_t compression = le32(p + 30);
  std::uint32_t paletteEntries = le32(p + 46);

  if (width <= 0 || rawHeight == 0)
    return std::unexpected(std::format("implausible dimensions {}x{}", width, rawHeight));
  if (compression != 0)
    return std::unexpected(std::format("compressed BMP (BI_* {}) not supported", compression));

  const bool bottomUp = rawHeight > 0;
  const std::int32_t height = bottomUp ? rawHeight : -rawHeight;

  // Rows are padded to a 4-byte boundary.
  const std::size_t rowBytes = ((static_cast<std::size_t>(width) * bpp + 31) / 32) * 4;
  const std::size_t needed = offBits + rowBytes * static_cast<std::size_t>(height);
  if (bmp.size() < needed)
    return std::unexpected(
        std::format("{} bytes but {}x{}x{} needs {}", bmp.size(), width, height, bpp, needed));

  // Palette, when there is one, sits between the header and the pixel data as
  // BGRX quads.
  const std::byte* palette = p + 14 + hdrSize;
  if (bpp <= 8) {
    if (paletteEntries == 0)
      paletteEntries = 1u << bpp;
    if (14 + hdrSize + paletteEntries * 4 > bmp.size())
      return std::unexpected("palette runs past the end of the bitmap");
  }

  Image img;
  img.width = width;
  img.height = height;
  img.rgba.resize(static_cast<std::size_t>(width) * height * 4, std::uint8_t{0});

  const auto putPalette = [&](std::uint8_t* out, std::uint32_t index) -> bool {
    if (index >= paletteEntries)
      return false;
    const std::byte* e = palette + index * 4;
    out[0] = std::to_integer<std::uint8_t>(e[2]);  // R
    out[1] = std::to_integer<std::uint8_t>(e[1]);  // G
    out[2] = std::to_integer<std::uint8_t>(e[0]);  // B
    out[3] = 0xFF;
    return true;
  };

  for (std::int32_t y = 0; y < height; ++y) {
    const std::byte* row = p + offBits + rowBytes * static_cast<std::size_t>(y);
    // A positive height means the first stored row is the bottom one.
    const std::int32_t dstY = bottomUp ? (height - 1 - y) : y;
    std::uint8_t* out = img.rgba.data() + static_cast<std::size_t>(dstY) * width * 4;

    switch (bpp) {
      case 24:
        for (std::int32_t x = 0; x < width; ++x) {
          const std::byte* s = row + static_cast<std::size_t>(x) * 3;
          out[x * 4 + 0] = std::to_integer<std::uint8_t>(s[2]);
          out[x * 4 + 1] = std::to_integer<std::uint8_t>(s[1]);
          out[x * 4 + 2] = std::to_integer<std::uint8_t>(s[0]);
          out[x * 4 + 3] = 0xFF;
        }
        break;
      case 32:
        for (std::int32_t x = 0; x < width; ++x) {
          const std::byte* s = row + static_cast<std::size_t>(x) * 4;
          out[x * 4 + 0] = std::to_integer<std::uint8_t>(s[2]);
          out[x * 4 + 1] = std::to_integer<std::uint8_t>(s[1]);
          out[x * 4 + 2] = std::to_integer<std::uint8_t>(s[0]);
          out[x * 4 + 3] = std::to_integer<std::uint8_t>(s[3]);
        }
        break;
      case 8:
        for (std::int32_t x = 0; x < width; ++x)
          if (!putPalette(out + x * 4, std::to_integer<std::uint8_t>(row[x])))
            return std::unexpected("palette index out of range");
        break;
      case 4:
        for (std::int32_t x = 0; x < width; ++x) {
          const std::uint8_t byte = std::to_integer<std::uint8_t>(row[x / 2]);
          const std::uint32_t idx = (x & 1) ? (byte & 0x0F) : (byte >> 4);
          if (!putPalette(out + x * 4, idx))
            return std::unexpected("palette index out of range");
        }
        break;
      default:
        return std::unexpected(std::format("unsupported bit depth {}", bpp));
    }
  }

  return img;
}

std::expected<void, std::string> applyAlfAlpha(Image& image, std::span<const std::byte> alf) {
  const std::size_t pixels = image.pixelCount();
  if (alf.size() < pixels)
    return std::unexpected(
        std::format("alpha plane has {} bytes for {} pixels", alf.size(), pixels));
  for (std::size_t i = 0; i < pixels; ++i)
    image.rgba[i * 4 + 3] = std::to_integer<std::uint8_t>(alf[i]);
  return {};
}

namespace {

void applyColourKey(Image& image, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
  const std::size_t pixels = image.pixelCount();
  for (std::size_t i = 0; i < pixels; ++i) {
    std::uint8_t* px = image.rgba.data() + i * 4;
    if (px[0] == r && px[1] == g && px[2] == b)
      px[3] = 0;
  }
}

}  // namespace

std::expected<Image, std::string> loadTexture(const AssetLibrary& lib, std::string_view name) {
  auto raw = lib.read(name);
  if (!raw)
    return std::unexpected(raw.error());

  auto bmpBytes = lrzDecode(*raw);
  if (!bmpBytes)
    return std::unexpected(std::format("{}: {}", name, bmpBytes.error()));

  auto img = decodeBmp(*bmpBytes);
  if (!img)
    return std::unexpected(std::format("{}: {}", name, img.error()));

  img->alpha = alphaModeFor(name);
  switch (img->alpha) {
    case AlphaMode::KeyBlack:
      applyColourKey(*img, 0x00, 0x00, 0x00);
      break;
    case AlphaMode::KeyGrey:
      applyColourKey(*img, 0x40, 0x40, 0x40);
      break;
    case AlphaMode::AlfFile: {
      // Same path and stem, .ALF instead of .BMP.
      std::string alfName(name);
      alfName.replace(alfName.size() - 4, 4, ".ALF");
      auto alf = lib.read(alfName);
      if (!alf)
        return std::unexpected(std::format("{}: no alpha plane ({})", name, alf.error()));
      if (auto r = applyAlfAlpha(*img, *alf); !r)
        return std::unexpected(std::format("{}: {}", name, r.error()));
      break;
    }
    case AlphaMode::Embedded:
    case AlphaMode::Additive:
    case AlphaMode::Opaque:
      break;
  }

  return img;
}

}  // namespace webo
