#include "Png.h"

#include "Bitmap.h"

#include <array>
#include <cstring>
#include <format>
#include <fstream>

namespace webo {
namespace {

std::uint32_t crc32(const std::uint8_t* data, std::size_t n, std::uint32_t crc = 0) {
  static const std::array<std::uint32_t, 256> table = [] {
    std::array<std::uint32_t, 256> t{};
    for (std::uint32_t i = 0; i < 256; ++i) {
      std::uint32_t c = i;
      for (int k = 0; k < 8; ++k)
        c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
      t[i] = c;
    }
    return t;
  }();

  crc = ~crc;
  for (std::size_t i = 0; i < n; ++i)
    crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
  return ~crc;
}

std::uint32_t adler32(const std::uint8_t* data, std::size_t n) {
  std::uint32_t a = 1, b = 0;
  for (std::size_t i = 0; i < n; ++i) {
    a = (a + data[i]) % 65521;
    b = (b + a) % 65521;
  }
  return (b << 16) | a;
}

void put32(std::vector<std::uint8_t>& v, std::uint32_t x) {
  v.push_back(static_cast<std::uint8_t>(x >> 24));
  v.push_back(static_cast<std::uint8_t>(x >> 16));
  v.push_back(static_cast<std::uint8_t>(x >> 8));
  v.push_back(static_cast<std::uint8_t>(x));
}

void chunk(std::vector<std::uint8_t>& out, const char tag[4], const std::vector<std::uint8_t>& body) {
  put32(out, static_cast<std::uint32_t>(body.size()));
  const std::size_t crcStart = out.size();
  out.insert(out.end(), tag, tag + 4);
  out.insert(out.end(), body.begin(), body.end());
  put32(out, crc32(out.data() + crcStart, out.size() - crcStart));
}

// zlib stream of stored deflate blocks: no compression, but always correct.
std::vector<std::uint8_t> zlibStored(const std::vector<std::uint8_t>& raw) {
  std::vector<std::uint8_t> z;
  z.push_back(0x78);  // CM = deflate, CINFO = 32K window
  z.push_back(0x01);  // FCHECK so that 0x7801 % 31 == 0, no preset dictionary

  constexpr std::size_t kMaxBlock = 65535;
  std::size_t pos = 0;
  do {
    const std::size_t n = std::min(kMaxBlock, raw.size() - pos);
    const bool last = (pos + n) >= raw.size();
    z.push_back(last ? 1 : 0);
    z.push_back(static_cast<std::uint8_t>(n & 0xFF));
    z.push_back(static_cast<std::uint8_t>(n >> 8));
    z.push_back(static_cast<std::uint8_t>(~n & 0xFF));
    z.push_back(static_cast<std::uint8_t>((~n >> 8) & 0xFF));
    z.insert(z.end(), raw.begin() + static_cast<std::ptrdiff_t>(pos),
             raw.begin() + static_cast<std::ptrdiff_t>(pos + n));
    pos += n;
  } while (pos < raw.size());

  put32(z, adler32(raw.data(), raw.size()));
  return z;
}

bool fullyOpaque(const Image& image) {
  const std::size_t pixels = image.pixelCount();
  for (std::size_t i = 0; i < pixels; ++i)
    if (image.rgba[i * 4 + 3] != 0xFF)
      return false;
  return true;
}

}  // namespace

std::vector<std::uint8_t> encodePng(const Image& image) {
  const bool opaque = fullyOpaque(image);
  const int channels = opaque ? 3 : 4;
  const std::uint8_t colourType = opaque ? 2 : 6;  // truecolour / truecolour+alpha

  // Raw scanlines, each prefixed by its filter byte (0 = None).
  std::vector<std::uint8_t> raw;
  raw.reserve(static_cast<std::size_t>(image.height) * (1 + image.width * channels));
  for (int y = 0; y < image.height; ++y) {
    raw.push_back(0);
    const std::uint8_t* src = image.rgba.data() + static_cast<std::size_t>(y) * image.width * 4;
    for (int x = 0; x < image.width; ++x) {
      raw.push_back(src[x * 4 + 0]);
      raw.push_back(src[x * 4 + 1]);
      raw.push_back(src[x * 4 + 2]);
      if (!opaque)
        raw.push_back(src[x * 4 + 3]);
    }
  }

  std::vector<std::uint8_t> out = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};

  std::vector<std::uint8_t> ihdr;
  put32(ihdr, static_cast<std::uint32_t>(image.width));
  put32(ihdr, static_cast<std::uint32_t>(image.height));
  ihdr.push_back(8);           // bit depth
  ihdr.push_back(colourType);
  ihdr.push_back(0);           // deflate
  ihdr.push_back(0);           // adaptive filtering
  ihdr.push_back(0);           // no interlace
  chunk(out, "IHDR", ihdr);
  chunk(out, "IDAT", zlibStored(raw));
  chunk(out, "IEND", {});
  return out;
}

std::expected<void, std::string> writePng(const std::filesystem::path& path, const Image& image) {
  if (image.width <= 0 || image.height <= 0 || image.empty())
    return std::unexpected("refusing to write an empty image");

  std::error_code ec;
  if (path.has_parent_path())
    std::filesystem::create_directories(path.parent_path(), ec);

  const auto bytes = encodePng(image);
  std::ofstream f(path, std::ios::binary);
  if (!f)
    return std::unexpected(std::format("{}: cannot open for writing", path.string()));
  f.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  if (!f)
    return std::unexpected(std::format("{}: write failed", path.string()));
  return {};
}

}  // namespace webo
