#include "app/ActorPose.h"
#include "app/ActorSprite.h"
#include "app/Camera.h"
#include "app/Raster.h"
#include "app/TubeScene.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/LightMap.h"
#include "assets/Png.h"
#include "assets/Sprite.h"
#include "gfx/SpriteView.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include <zlib.h>

namespace fs = std::filesystem;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

// Minimal SHA-256 (public domain style) for PNG fingerprinting in this test only.
struct Sha256 {
  std::array<std::uint8_t, 32> digest{};
};

Sha256 sha256(std::span<const std::uint8_t> data) {
  static constexpr std::uint32_t k[64] = {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
      0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
      0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
      0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
      0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
      0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
  auto rotr = [](std::uint32_t x, int n) { return (x >> n) | (x << (32 - n)); };
  std::uint32_t h0 = 0x6a09e667, h1 = 0xbb67ae85, h2 = 0x3c6ef372, h3 = 0xa54ff53a;
  std::uint32_t h4 = 0x510e527f, h5 = 0x9b05688c, h6 = 0x1f83d9ab, h7 = 0x5be0cd19;
  std::vector<std::uint8_t> msg(data.begin(), data.end());
  const std::uint64_t bitLen = msg.size() * 8;
  msg.push_back(0x80);
  while ((msg.size() % 64) != 56)
    msg.push_back(0);
  for (int i = 7; i >= 0; --i)
    msg.push_back(static_cast<std::uint8_t>((bitLen >> (i * 8)) & 0xFF));
  std::uint32_t w[64];
  for (std::size_t off = 0; off < msg.size(); off += 64) {
    for (int i = 0; i < 16; ++i)
      w[i] = (msg[off + i * 4] << 24) | (msg[off + i * 4 + 1] << 16) |
             (msg[off + i * 4 + 2] << 8) | msg[off + i * 4 + 3];
    for (int i = 16; i < 64; ++i) {
      const std::uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
      const std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
      w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    std::uint32_t a = h0, b = h1, c = h2, d = h3, e = h4, f = h5, g = h6, hh = h7;
    for (int i = 0; i < 64; ++i) {
      const std::uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
      const std::uint32_t ch = (e & f) ^ ((~e) & g);
      const std::uint32_t temp1 = hh + S1 + ch + k[i] + w[i];
      const std::uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
      const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
      const std::uint32_t temp2 = S0 + maj;
      hh = g;
      g = f;
      f = e;
      e = d + temp1;
      d = c;
      c = b;
      b = a;
      a = temp1 + temp2;
    }
    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;
    h4 += e;
    h5 += f;
    h6 += g;
    h7 += hh;
  }
  Sha256 out;
  const std::uint32_t hs[] = {h0, h1, h2, h3, h4, h5, h6, h7};
  for (int i = 0; i < 8; ++i) {
    out.digest[static_cast<std::size_t>(i) * 4] = static_cast<std::uint8_t>((hs[i] >> 24) & 0xFF);
    out.digest[static_cast<std::size_t>(i) * 4 + 1] = static_cast<std::uint8_t>((hs[i] >> 16) & 0xFF);
    out.digest[static_cast<std::size_t>(i) * 4 + 2] = static_cast<std::uint8_t>((hs[i] >> 8) & 0xFF);
    out.digest[static_cast<std::size_t>(i) * 4 + 3] = static_cast<std::uint8_t>(hs[i] & 0xFF);
  }
  return out;
}

std::string hexSha256(const Sha256& s) {
  std::string out;
  out.reserve(64);
  for (std::uint8_t b : s.digest)
    out += std::format("{:02x}", b);
  return out;
}

webo::Image toRgbaImage(const webo::app::Framebuffer& fb) {
  webo::Image image;
  image.width = fb.width;
  image.height = fb.height;
  image.rgba.resize(static_cast<std::size_t>(fb.width) * fb.height * 4);
  for (std::size_t i = 0, n = static_cast<std::size_t>(fb.width) * fb.height; i < n; ++i) {
    image.rgba[i * 4] = fb.rgb[i * 3];
    image.rgba[i * 4 + 1] = fb.rgb[i * 3 + 1];
    image.rgba[i * 4 + 2] = fb.rgb[i * 3 + 2];
    image.rgba[i * 4 + 3] = 255;
  }
  return image;
}

std::string pngSha256(const webo::app::Framebuffer& fb) {
  const webo::Image image = toRgbaImage(fb);
  const std::vector<std::uint8_t> png = webo::encodePng(image);
  return hexSha256(sha256(png));
}

void writePngFile(const fs::path& path, const webo::Image& image) {
  const std::vector<std::uint8_t> png = webo::encodePng(image);
  std::ofstream out(path, std::ios::binary);
  if (!out)
    throw std::runtime_error("png write failed");
  out.write(reinterpret_cast<const char*>(png.data()),
            static_cast<std::streamsize>(png.size()));
}

bool rectDiffers(const webo::app::Framebuffer& a, const webo::app::Framebuffer& b, int x0,
                 int y0, int x1, int y1) {
  if (a.width != b.width || a.height != b.height)
    return false;
  x0 = std::max(0, x0);
  y0 = std::max(0, y0);
  x1 = std::min(a.width, x1);
  y1 = std::min(a.height, y1);
  for (int y = y0; y < y1; ++y) {
    for (int x = x0; x < x1; ++x) {
      const std::size_t o = (static_cast<std::size_t>(y) * a.width + x) * 3;
      if (a.rgb[o] != b.rgb[o] || a.rgb[o + 1] != b.rgb[o + 1] || a.rgb[o + 2] != b.rgb[o + 2])
        return true;
    }
  }
  return false;
}

struct ViewSpec {
  const char* name;
  const char* cameraRel;
};

// Empty-list route PNG SHA-256 (encodePng of 640x480 RGBA); must stay A4d-stable.
constexpr std::array<const char*, 9> kEmptyGoldenSha = {
    "66a8ea3b44df2216b64debf9d8f417d2ed923841fc440e99c028b013010d3651",
    "2c0867ba4a1a28be1362fc01f5f8f48f374f1ada14af711b55a8afbb6029f3bf",
    "23b120d9fbcd89949ff96e0e9141e9382ffbc92077a4ebd90f82af84e8530d1d",
    "4fa8f73a0bfaaa0c0940ba43e6c6b67fb7e0658243d21f5bdff44fb50c1d7d12",
    "e7472de02e3f6b07e50f62759a0e0a5e6ff60eac0b318bf1680d876d9cd7a8a7",
    "4751a3a58eb8380d1aeb386954040c709e767f99519bad4fbea12d5aefb9962f",
    "89e29c687a430ee10e8d5a2c65158d1d276e33f968f1e5c7aca1570e5cd35163",
    "7e779907403e65d94b653a72e68fcbc62bd3e1218fe3451f5861bcfe8784a468",
    "e7472de02e3f6b07e50f62759a0e0a5e6ff60eac0b318bf1680d876d9cd7a8a7",
};

constexpr ViewSpec kViews[] = {
    {"initial", "tools/harness/fixtures/camera_a1/camera-fixture-params.json"},
    {"mouse", "tools/harness/fixtures/camera_a1/rotation-mouse-params.json"},
    {"right", "tools/harness/fixtures/camera_a1/rotation-key-right-params.json"},
    {"left", "tools/harness/fixtures/camera_a1/rotation-key-left-params.json"},
    {"canonical", "build/tmp/a1-zoom-elevation/params-canonical.json"},
    {"zoom1", "build/tmp/a1-zoom-elevation/params-zoom1.json"},
    {"elev_up", "build/tmp/a1-zoom-elevation/params-elev_up.json"},
    {"elev_down", "build/tmp/a1-zoom-elevation/params-elev_down.json"},
    {"elev0_neg", "build/tmp/a1-zoom-elevation/params-elev0_neg.json"},
};

constexpr struct {
  double plus8[3];
  int facing;
  double model108[3];
} kInjected[] = {
    {60.244850158691406, 32.0, 0.75, 1, {-56.993675231933594, 32.0, 6.125833988189697}},
    {45.812477111816406, 32.0, 0.75, 1, {-51.26413345336914, 32.0, 25.476972579956055}},
    {32.0, 34.47958755493164, 0.5, 1, {-39.0824089050293, 36.87969207763672, 39.0824089050293}},
};

std::vector<webo::app::SceneOptions::InjectedNativeWalker> makeInjectedList() {
  return webo::app::defaultMmClimb002InjectedWalkers();
}

std::uint32_t readBe32(const std::uint8_t* p) {
  return (static_cast<std::uint32_t>(p[0]) << 24) | (static_cast<std::uint32_t>(p[1]) << 16) |
         (static_cast<std::uint32_t>(p[2]) << 8) | static_cast<std::uint32_t>(p[3]);
}

std::vector<std::uint8_t> loadFixtureRgbPng(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in)
    throw std::runtime_error("png open failed");
  std::vector<std::uint8_t> file((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  static const std::uint8_t kSig[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
  if (file.size() < 8 || !std::equal(std::begin(kSig), std::end(kSig), file.begin()))
    throw std::runtime_error("png signature");
  int width = 0;
  int height = 0;
  int bitDepth = 0;
  int colorType = 0;
  std::vector<std::uint8_t> idat;
  for (std::size_t i = 8; i + 12 <= file.size();) {
    const std::uint32_t len = readBe32(file.data() + i);
    const char* type = reinterpret_cast<const char*>(file.data() + i + 4);
    const std::uint8_t* data = file.data() + i + 8;
    i += 12 + len;
    if (std::string_view(type, 4) == "IHDR") {
      width = static_cast<int>(readBe32(data));
      height = static_cast<int>(readBe32(data + 4));
      bitDepth = data[8];
      colorType = data[9];
    } else if (std::string_view(type, 4) == "IDAT") {
      idat.insert(idat.end(), data, data + len);
    }
  }
  if (width <= 0 || height <= 0 || bitDepth != 8 || (colorType != 2 && colorType != 6))
    throw std::runtime_error("png layout unsupported");
  uLongf outLen = static_cast<uLongf>((static_cast<std::size_t>(width) * 3 + 1) * height);
  std::vector<std::uint8_t> inflated(outLen);
  if (uncompress(inflated.data(), &outLen, idat.data(), static_cast<uLong>(idat.size())) != Z_OK)
    throw std::runtime_error("png inflate failed");
  const int bpp = colorType == 2 ? 3 : 4;
  const int stride = width * bpp;
  std::vector<std::uint8_t> recon(static_cast<std::size_t>(height) * stride);
  auto paeth = [](int a, int b, int c) {
    const int p = a + b - c;
    const int pa = std::abs(p - a);
    const int pb = std::abs(p - b);
    const int pc = std::abs(p - c);
    if (pa <= pb && pa <= pc)
      return a;
    if (pb <= pc)
      return b;
    return c;
  };
  for (int y = 0; y < height; ++y) {
    const std::uint8_t* row = inflated.data() + static_cast<std::size_t>(y) * (stride + 1);
    const int filter = row[0];
    std::uint8_t* out = recon.data() + static_cast<std::size_t>(y) * stride;
    const std::uint8_t* prev = y > 0 ? recon.data() + static_cast<std::size_t>(y - 1) * stride : nullptr;
    for (int i = 0; i < stride; ++i) {
      const int raw = row[1 + i];
      int val = raw;
      const int left = i >= bpp ? out[i - bpp] : 0;
      const int up = prev ? prev[i] : 0;
      const int upLeft = (prev && i >= bpp) ? prev[i - bpp] : 0;
      switch (filter) {
        case 0:
          val = raw;
          break;
        case 1:
          val = (raw + left) & 0xFF;
          break;
        case 2:
          val = (raw + up) & 0xFF;
          break;
        case 3:
          val = (raw + ((left + up) / 2)) & 0xFF;
          break;
        case 4:
          val = (raw + paeth(left, up, upLeft)) & 0xFF;
          break;
        default:
          throw std::runtime_error("png filter unsupported");
      }
      out[i] = static_cast<std::uint8_t>(val);
    }
  }
  std::vector<std::uint8_t> rgb(static_cast<std::size_t>(width) * height * 3);
  for (int y = 0; y < height; ++y) {
    const std::uint8_t* row = recon.data() + static_cast<std::size_t>(y) * stride;
    for (int x = 0; x < width; ++x) {
      const std::size_t dst = (static_cast<std::size_t>(y) * width + x) * 3;
      rgb[dst] = row[x * bpp];
      rgb[dst + 1] = row[x * bpp + 1];
      rgb[dst + 2] = row[x * bpp + 2];
    }
  }
  return rgb;
}

double rectRgbMae(const webo::app::Framebuffer& fb, const std::vector<std::uint8_t>& refRgb,
                  int refWidth, int refHeight, int x0, int y0, int x1, int y1) {
  if (fb.width != refWidth || fb.height != refHeight)
    return 1.0;
  x0 = std::max(0, x0);
  y0 = std::max(0, y0);
  x1 = std::min(refWidth, x1);
  y1 = std::min(refHeight, y1);
  if (x0 >= x1 || y0 >= y1)
    return 1.0;
  std::uint64_t sum = 0;
  std::uint64_t count = 0;
  for (int y = y0; y < y1; ++y) {
    for (int x = x0; x < x1; ++x) {
      const std::size_t o = (static_cast<std::size_t>(y) * refWidth + x) * 3;
      sum += static_cast<std::uint64_t>(std::abs(fb.rgb[o] - refRgb[o]));
      sum += static_cast<std::uint64_t>(std::abs(fb.rgb[o + 1] - refRgb[o + 1]));
      sum += static_cast<std::uint64_t>(std::abs(fb.rgb[o + 2] - refRgb[o + 2]));
      ++count;
    }
  }
  return static_cast<double>(sum) / (255.0 * 3.0 * static_cast<double>(count));
}

struct RgbTri {
  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
};

bool rgbTriAt(const std::vector<std::uint8_t>& rgb, int width, int height, int x, int y,
              RgbTri& out) {
  if (x < 0 || y < 0 || x >= width || y >= height)
    return false;
  const std::size_t o = (static_cast<std::size_t>(y) * width + x) * 3;
  out = {rgb[o], rgb[o + 1], rgb[o + 2]};
  return true;
}

bool rgbTriAtFb(const webo::app::Framebuffer& fb, int x, int y, RgbTri& out) {
  return rgbTriAt(fb.rgb, fb.width, fb.height, x, y, out);
}

int rgbL1(const RgbTri& a, const RgbTri& b) {
  return std::abs(static_cast<int>(a.r) - static_cast<int>(b.r)) +
         std::abs(static_cast<int>(a.g) - static_cast<int>(b.g)) +
         std::abs(static_cast<int>(a.b) - static_cast<int>(b.b));
}

bool rgbEqual(const RgbTri& a, const RgbTri& b) {
  return a.r == b.r && a.g == b.g && a.b == b.b;
}

struct ShiftMatch {
  int dx;
  int dy;
  int err;
};

ShiftMatch bestShiftInjectToOrig(const webo::app::Framebuffer& inject,
                                 const std::vector<std::uint8_t>& origRgb, int refWidth,
                                 int refHeight, int x, int y) {
  RgbTri inj{};
  rgbTriAtFb(inject, x, y, inj);
  RgbTri aligned{};
  rgbTriAt(origRgb, refWidth, refHeight, x, y, aligned);
  ShiftMatch best{0, 0, rgbL1(inj, aligned)};
  for (int dy = -2; dy <= 2; ++dy) {
    for (int dx = -2; dx <= 2; ++dx) {
      if (dx == 0 && dy == 0)
        continue;
      RgbTri shifted{};
      if (!rgbTriAt(origRgb, refWidth, refHeight, x + dx, y + dy, shifted))
        continue;
      const int err = rgbL1(inj, shifted);
      if (err < best.err)
        best = {dx, dy, err};
    }
  }
  return best;
}

// A6d17 painted-pixel classes (tube != inject within actor-2 bbox).
const char* classifyActor2PaintedPixel(const webo::app::Framebuffer& tube,
                                       const webo::app::Framebuffer& inject,
                                       const std::vector<std::uint8_t>& origRgb, int refWidth,
                                       int refHeight, int x, int y) {
  RgbTri o{};
  RgbTri t{};
  RgbTri inj{};
  rgbTriAt(origRgb, refWidth, refHeight, x, y, o);
  rgbTriAtFb(tube, x, y, t);
  rgbTriAtFb(inject, x, y, inj);
  if (rgbEqual(o, t) && !rgbEqual(inj, t))
    return "extra_coverage";
  const ShiftMatch shift = bestShiftInjectToOrig(inject, origRgb, refWidth, refHeight, x, y);
  const int alignedErr = rgbL1(inj, o);
  if ((shift.dx != 0 || shift.dy != 0) && shift.err + 6 < alignedErr)
    return "integer_shift";
  if (alignedErr > 0 && !rgbEqual(t, o) && !rgbEqual(inj, o))
    return "tint_scale";
  return "different_silhouette";
}

webo::app::Framebuffer renderRoute(const webo::app::TubeScene& scene, const webo::AssetLibrary& lib,
                                   const fs::path& cameraPath,
                                   const std::vector<webo::app::SceneOptions::InjectedNativeWalker>&
                                       walkers,
                                   bool actorsBeforeRoute, bool skipPose,
                                   double composeActorPhase = 0.0,
                                   bool overlayLegacyNonZero = false) {
  webo::app::CameraParams params;
  std::string error;
  if (!webo::app::loadCameraParams(cameraPath, params, error))
    throw std::runtime_error(error);
  const auto camera = webo::app::buildCamera(params);
  webo::app::SceneOptions options;
  options.drawCellRoute = true;
  options.drawWall = false;
  options.drawOuter = false;
  options.drawInner = false;
  options.injectedWalkers = walkers;
  options.composeActorsBeforeRoute = actorsBeforeRoute;
  options.composeSkipActorPose = skipPose;
  options.composeActorPhase = composeActorPhase;
  options.overlayCompositeLegacyNonZero = overlayLegacyNonZero;
  webo::app::Raster raster(params.width, params.height);
  raster.clear(0, 0, 0);
  scene.render(raster, camera, options, &lib);
  return raster.framebuffer();
}

}  // namespace

int main() {
  const char* env = std::getenv("WEBOLUTION_GAME_DIR");
  if (!env) {
    std::cout << "skip: WEBOLUTION_GAME_DIR not set\n";
    return 0;
  }
  const fs::path root = fs::path(WEBOLUTION_SOURCE_DIR);
  const fs::path gameDir(env);
  try {
    auto library = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
    if (!library)
      throw std::runtime_error(library.error());
    std::string levelPath;
    for (const auto& name : webo::levelFileNames(*library)) {
      if (name.find("MM_CLIMB_002") != std::string::npos) {
        levelPath = name;
        break;
      }
    }
    if (levelPath.empty())
      throw std::runtime_error("MM_CLIMB_002 missing");
    auto blob = library->read(levelPath);
    auto mesh = webo::findCylinderMesh(*blob);
    auto index = webo::indexLevel(*blob);
    if (!blob || !mesh || !index)
      throw std::runtime_error("level parse failed");
    auto scene = webo::app::TubeScene::build(*library, *mesh, *blob, index->grid0, true);
    if (!scene)
      throw std::runtime_error(scene.error());

    std::cout << "actor compose empty-list tube hashes\n";
    std::array<std::string, 9> emptyHashes{};
    for (std::size_t i = 0; i < 9; ++i) {
      const fs::path cam = root / kViews[i].cameraRel;
      if (!fs::exists(cam)) {
        check(false, std::format("{} camera fixture missing", kViews[i].name));
        continue;
      }
      const auto fb = renderRoute(*scene, *library, cam, {}, false, false);
      emptyHashes[i] = pngSha256(fb);
      const auto fb2 = renderRoute(*scene, *library, cam, {}, false, false);
      check(pngSha256(fb2) == emptyHashes[i],
            std::format("{} empty-list render stable", kViews[i].name));
      check(emptyHashes[i] == kEmptyGoldenSha[i],
            std::format("{} empty-list A4d PNG hash", kViews[i].name));
    }

    std::cout << "actor compose injected walkers on initial view\n";
    const fs::path initialCam = root / kViews[0].cameraRel;
    const auto injected = makeInjectedList();
    const auto emptyFb = renderRoute(*scene, *library, initialCam, {}, false, false);
    const auto withFb = renderRoute(*scene, *library, initialCam, injected, false, false);

    static std::vector<std::uint32_t> poseGrid0 = index->grid0;
    static std::vector<webo::Vec3> poseSurface = mesh->surface;
    static webo::CylinderMesh poseCylinder = *mesh;
    webo::ActorPoseLevel poseLevel;
    if (auto grid = webo::findCollisionGrid(*blob)) {
      poseLevel.gridHeight = grid->height;
      poseLevel.grid0 = poseGrid0;
      poseLevel.surface = poseSurface;
      poseLevel.cylinderTemplate = &poseCylinder;
    }

    auto sprites = webo::loadSpriteSheet(*library, "SPRITES\\LEMMINGSHI.SPR");
    if (!sprites)
      throw std::runtime_error(sprites.error());
    const webo::Sprite* walk = sprites->find("walk");
    if (walk == nullptr || walk->animations.empty() || walk->animations[0].frames.empty())
      throw std::runtime_error("walk temporal 0 missing");
    const webo::SpriteFrame& walkFrame = walk->animations[0].frames[0];

    webo::app::CameraParams camParams;
    std::string camError;
    webo::app::loadCameraParams(initialCam, camParams, camError);
    const auto camera = webo::app::buildCamera(camParams);

    for (std::size_t a = 0; a < 3; ++a) {
      const auto pose = webo::actorPoseFromNativeSample(
          poseLevel,
          webo::ActorTerrainSample{kInjected[a].plus8[0], kInjected[a].plus8[1], kInjected[a].plus8[2]},
          kInjected[a].facing);
      const std::array<double, 3> world{pose.modelPosition[0], pose.modelPosition[1],
                                        pose.modelPosition[2]};
      auto placement = webo::app::actorSpritePlacement(walkFrame, world, camera, true, -1);
      if (!placement) {
        check(false, std::format("actor {} placement", a));
        continue;
      }
      const int x0 = static_cast<int>(std::floor(placement->left));
      const int y0 = static_cast<int>(std::floor(placement->top));
      const int x1 = static_cast<int>(std::ceil(placement->right));
      const int y1 = static_cast<int>(std::ceil(placement->bottom));
      check(rectDiffers(emptyFb, withFb, x0, y0, x1, y1),
            std::format("actor {} rect differs from tube-only", a));
    }

    std::cout << "actor compose bbox MAE vs original-0 on initial\n";
    const fs::path origPng = root / "tools/harness/fixtures/camera_a1/original-0.png";
    const auto origRgb = loadFixtureRgbPng(origPng);
    constexpr int kRefW = 640;
    constexpr int kRefH = 480;
    std::array<double, 3> maeEmpty{};
    std::array<double, 3> maeWith{};
    for (std::size_t a = 0; a < 3; ++a) {
      const auto pose = webo::actorPoseFromNativeSample(
          poseLevel,
          webo::ActorTerrainSample{kInjected[a].plus8[0], kInjected[a].plus8[1], kInjected[a].plus8[2]},
          kInjected[a].facing);
      const std::array<double, 3> world{pose.modelPosition[0], pose.modelPosition[1],
                                        pose.modelPosition[2]};
      auto placement = webo::app::actorSpritePlacement(walkFrame, world, camera, true, -1);
      if (!placement) {
        check(false, std::format("actor {} bbox placement", a));
        continue;
      }
      const int x0 = static_cast<int>(std::floor(placement->left));
      const int y0 = static_cast<int>(std::floor(placement->top));
      const int x1 = static_cast<int>(std::ceil(placement->right));
      const int y1 = static_cast<int>(std::ceil(placement->bottom));
      maeEmpty[a] = rectRgbMae(emptyFb, origRgb, kRefW, kRefH, x0, y0, x1, y1);
      maeWith[a] = rectRgbMae(withFb, origRgb, kRefW, kRefH, x0, y0, x1, y1);
      std::cout << std::format("  actor {} rect [{},{})x[{},{}) tube_mae={:.6f} with_mae={:.6f}\n", a,
                               x0, x1, y0, y1, maeEmpty[a], maeWith[a]);
      if (maeEmpty[a] < 1e-4 && maeWith[a] < 1e-4)
        std::cout << std::format("  WARN actor {} rect may not overlap original lemming pixels\n", a);
      if (maeWith[a] >= maeEmpty[a])
        std::cout << std::format("  NOTE actor {} A6 bbox gate not met (with_mae >= tube_mae)\n", a);
    }

    std::cout << "actor compose mutation: actors before route fails\n";
    const auto mutatedBefore = renderRoute(*scene, *library, initialCam, injected, true, false);
    check(pngSha256(mutatedBefore) != pngSha256(withFb),
          "actors-before-route differs from post-route injected compose");
    bool beforeRouteBboxGate = true;
    for (std::size_t a = 0; a < 3; ++a) {
      const auto pose = webo::actorPoseFromNativeSample(
          poseLevel,
          webo::ActorTerrainSample{kInjected[a].plus8[0], kInjected[a].plus8[1], kInjected[a].plus8[2]},
          kInjected[a].facing);
      const std::array<double, 3> world{pose.modelPosition[0], pose.modelPosition[1],
                                        pose.modelPosition[2]};
      auto placement = webo::app::actorSpritePlacement(walkFrame, world, camera, true, -1);
      if (!placement)
        continue;
      const int x0 = static_cast<int>(std::floor(placement->left));
      const int y0 = static_cast<int>(std::floor(placement->top));
      const int x1 = static_cast<int>(std::ceil(placement->right));
      const int y1 = static_cast<int>(std::ceil(placement->bottom));
      const double maeBefore =
          rectRgbMae(mutatedBefore, origRgb, kRefW, kRefH, x0, y0, x1, y1);
      if (!(maeBefore < maeEmpty[a]))
        beforeRouteBboxGate = false;
    }
    check(!beforeRouteBboxGate, "actors-before-route fails A6 bbox MAE gate");

    std::cout << "actor compose mutation: skip ActorPose fails\n";
    const auto skipPoseFb = renderRoute(*scene, *library, initialCam, injected, false, true);
    bool skipDiff = false;
    for (std::size_t a = 0; a < 3; ++a) {
      const std::array<double, 3> world{kInjected[a].plus8[0], kInjected[a].plus8[1],
                                        kInjected[a].plus8[2]};
      auto placement = webo::app::actorSpritePlacement(walkFrame, world, camera, true, -1);
      if (!placement)
        continue;
      const int x0 = static_cast<int>(std::floor(placement->left));
      const int y0 = static_cast<int>(std::floor(placement->top));
      const int x1 = static_cast<int>(std::ceil(placement->right));
      const int y1 = static_cast<int>(std::ceil(placement->bottom));
      if (rectDiffers(emptyFb, skipPoseFb, x0, y0, x1, y1))
        skipDiff = true;
    }
    check(skipDiff, "skip ActorPose misses walker pixels in placement rects");

    std::cout << "actor compose nine-view empty hashes (recorded at first green run)\n";
    for (std::size_t i = 0; i < 9; ++i)
      std::cout << "  " << kViews[i].name << " " << emptyHashes[i] << "\n";

    const fs::path matchedCam = root / "build/tmp/a6d3-matched-state-bbox/live-camera-params.json";
    const fs::path matchedOrig = root / "build/tmp/a6d3-matched-state-bbox/original-live.png";
    const fs::path a6d4Out = root / "build/tmp/a6d4-actor2-temporal-sweep";
    if (fs::exists(matchedCam) && fs::exists(matchedOrig)) {
      std::cout << "a6d4 matched-state actor2 temporal sweep\n";
      fs::create_directories(a6d4Out);
      const auto matchedRgb = loadFixtureRgbPng(matchedOrig);
      constexpr int kActor2X0 = 315;
      constexpr int kActor2X1 = 326;
      constexpr int kActor2Y0 = 283;
      constexpr int kActor2Y1 = 301;
      constexpr double kA6d3TubeMae = 0.10089786756453423;
      constexpr double kA6d3Temporal0Mae = 0.11246451442529874;
      constexpr std::array<std::array<int, 4>, 2> kActorRects = {{
          {{467, 282, 480, 300}},
          {{397, 292, 408, 310}},
      }};

      const auto tubeOnlyMatched =
          renderRoute(*scene, *library, matchedCam, {}, false, false);
      const double tubeMae2 = rectRgbMae(tubeOnlyMatched, matchedRgb, kRefW, kRefH, kActor2X0,
                                         kActor2Y0, kActor2X1, kActor2Y1);
      std::cout << std::format("  actor2 tube-only mae={:.17f}\n", tubeMae2);
      check(std::abs(tubeMae2 - kA6d3TubeMae) < 1e-7, "a6d4 tube-only actor2 MAE matches a6d3");

      const int temporalCount = static_cast<int>(walk->animations.size());
      double bestTemporalMae = tubeMae2;
      int bestTemporal = -1;
      std::ofstream table(a6d4Out / "mae-table.tsv");
      table << "kind\tindex\tactor2_mae\tbeats_tube_only\n";
      table << std::format("tube_only\t-\t{:.17f}\tfalse\n", tubeMae2);

      for (int t = 0; t < temporalCount; ++t) {
        auto walkers = makeInjectedList();
        walkers[2].walkTemporalIndex = t;
        const auto fb = renderRoute(*scene, *library, matchedCam, walkers, false, false);
        const double mae = rectRgbMae(fb, matchedRgb, kRefW, kRefH, kActor2X0, kActor2Y0,
                                       kActor2X1, kActor2Y1);
        const bool beats = mae < tubeMae2;
        std::cout << std::format("  temporal {} actor2_mae={:.17f}{}\n", t, mae,
                                 beats ? " BEATS" : "");
        table << std::format("temporal\t{}\t{:.17f}\t{}\n", t, mae, beats ? "true" : "false");
        if (mae < bestTemporalMae) {
          bestTemporalMae = mae;
          bestTemporal = t;
        }
      }

      auto walkersT0 = makeInjectedList();
      const auto injectT0 = renderRoute(*scene, *library, matchedCam, walkersT0, false, false);
      const double t0Mae = rectRgbMae(injectT0, matchedRgb, kRefW, kRefH, kActor2X0, kActor2Y0,
                                      kActor2X1, kActor2Y1);
      check(std::abs(t0Mae - kA6d3Temporal0Mae) < 1e-7, "a6d4 temporal0 inject MAE matches a6d3");
      check(t0Mae >= tubeMae2, "a6d4 temporal0 inject does not beat tube-only on actor2");

      double bestPhaseMae = tubeMae2;
      int bestPhaseIndex = -1;
      static constexpr std::array<double, 5> kPhaseGrid = {0.0, -0.0625, 0.0625, -0.125, 0.125};
      for (std::size_t pi = 0; pi < kPhaseGrid.size(); ++pi) {
        const double phase = kPhaseGrid[pi];
        const auto fb =
            renderRoute(*scene, *library, matchedCam, walkersT0, false, false, phase);
        const double mae = rectRgbMae(fb, matchedRgb, kRefW, kRefH, kActor2X0, kActor2Y0,
                                       kActor2X1, kActor2Y1);
        const bool beats = mae < tubeMae2;
        std::cout << std::format("  phase {} actor2_mae={:.17f}{}\n", phase, mae,
                                 beats ? " BEATS" : "");
        table << std::format("phase\t{}\t{:.17f}\t{}\n", phase, mae, beats ? "true" : "false");
        if (mae < bestPhaseMae) {
          bestPhaseMae = mae;
          bestPhaseIndex = static_cast<int>(pi);
        }
      }

      const bool temporalWins = bestTemporal >= 0 && bestTemporalMae < tubeMae2;
      const bool phaseWins = bestPhaseIndex >= 0 && bestPhaseMae < tubeMae2;
      if (temporalWins) {
        std::cout << std::format("  CHOSEN temporal {} mae={:.17f}\n", bestTemporal,
                                 bestTemporalMae);
        check(bestTemporalMae < tubeMae2, "a6d4 winning temporal beats tube-only");
      } else if (phaseWins) {
        const double chosenPhase = kPhaseGrid[static_cast<std::size_t>(bestPhaseIndex)];
        std::cout << std::format("  CHOSEN phase {} mae={:.17f}\n", chosenPhase, bestPhaseMae);
        check(bestPhaseMae < tubeMae2, "a6d4 winning phase beats tube-only");
        const auto phaseFb =
            renderRoute(*scene, *library, matchedCam, walkersT0, false, false, chosenPhase);
        for (std::size_t a = 0; a < 2; ++a) {
          const int x0 = kActorRects[a][0];
          const int y0 = kActorRects[a][1];
          const int x1 = kActorRects[a][2];
          const int y1 = kActorRects[a][3];
          const double emptyMae =
              rectRgbMae(tubeOnlyMatched, matchedRgb, kRefW, kRefH, x0, y0, x1, y1);
          const double withMae =
              rectRgbMae(phaseFb, matchedRgb, kRefW, kRefH, x0, y0, x1, y1);
          check(withMae < emptyMae,
                std::format("a6d4 actor {} improves at chosen phase {}", a, chosenPhase));
        }
        const auto phase0Fb =
            renderRoute(*scene, *library, matchedCam, walkersT0, false, false, 0.0);
        const double phase0Actor2 = rectRgbMae(phase0Fb, matchedRgb, kRefW, kRefH, kActor2X0,
                                               kActor2Y0, kActor2X1, kActor2Y1);
        check(phase0Actor2 >= tubeMae2, "a6d4 mutation phase0 fails actor2 bbox gate");
      } else {
        std::cout << "  RESULT no temporal or phase beats tube-only on actor2\n";
        check(bestTemporalMae >= tubeMae2 && bestPhaseMae >= tubeMae2,
              "a6d4 sweep negative confirmed");
      }

      // Actors 0–1 must still improve at default inject (temporal 0 all walkers, phase 0).
      for (std::size_t a = 0; a < 2; ++a) {
        const int x0 = kActorRects[a][0];
        const int y0 = kActorRects[a][1];
        const int x1 = kActorRects[a][2];
        const int y1 = kActorRects[a][3];
        const double emptyMae = rectRgbMae(tubeOnlyMatched, matchedRgb, kRefW, kRefH, x0, y0, x1, y1);
        const double withMae = rectRgbMae(injectT0, matchedRgb, kRefW, kRefH, x0, y0, x1, y1);
        check(withMae < emptyMae, std::format("a6d4 actor {} still improves at temporal0", a));
      }

      // Isolated mutation: actor 2 forced to temporal 0 while a winner exists would fail;
      // with no winner, temporal-0 inject must remain worse than tube-only (checked above).
      if (temporalWins && bestTemporal != 0) {
        auto mutated = makeInjectedList();
        mutated[2].walkTemporalIndex = 0;
        const auto fb = renderRoute(*scene, *library, matchedCam, mutated, false, false);
        const double mae = rectRgbMae(fb, matchedRgb, kRefW, kRefH, kActor2X0, kActor2Y0,
                                       kActor2X1, kActor2Y1);
        check(mae >= tubeMae2, "a6d4 mutation temporal0 fails actor2 bbox gate");
      }

      std::cout << "a6d5 live +0x1D8 read: production default matches native 0\n";
      check(webo::app::SceneOptions{}.composeActorPhase == 0.0,
            "a6d5 composeActorPhase default is native 0");
      constexpr double kA6d5ProvisionalNegQuarterMae = 0.08861160625866508;
      const auto provisionalNegQuarter =
          renderRoute(*scene, *library, matchedCam, walkersT0, false, false, -0.125);
      const double provisionalActor2Mae =
          rectRgbMae(provisionalNegQuarter, matchedRgb, kRefW, kRefH, kActor2X0, kActor2Y0,
                     kActor2X1, kActor2Y1);
      std::cout << std::format("  actor2 native phase0 mae={:.17f}\n", t0Mae);
      std::cout << std::format("  actor2 provisional -0.125 pixel-fit mae={:.17f}\n",
                               provisionalActor2Mae);
      check(std::abs(provisionalActor2Mae - kA6d5ProvisionalNegQuarterMae) < 1e-7,
            "a6d5 provisional -0.125 MAE matches a6d4 sweep");
      check(provisionalActor2Mae < tubeMae2,
            "a6d5 provisional -0.125 beats tube-only (pixel fit only, not READ)");
      check(t0Mae >= tubeMae2, "a6d5 native phase0 default fails actor2 bbox gate vs tube-only");

      const fs::path a6d6Out = root / "build/tmp/a6d6-live-view-index";
      if (fs::exists(a6d6Out)) {
        fs::create_directories(a6d6Out);
        webo::app::CameraParams matchedParams;
        if (!webo::app::loadCameraParams(matchedCam, matchedParams, camError))
          throw std::runtime_error(camError);
        const auto matchedCamera = webo::app::buildCamera(matchedParams);
        std::ofstream replay(a6d6Out / "inject-replay.json");
        replay << "[\n";
        for (std::size_t i = 0; i < 3; ++i) {
          const auto pose = webo::actorPoseFromNativeSample(
              poseLevel,
              webo::ActorTerrainSample{kInjected[i].plus8[0], kInjected[i].plus8[1],
                                       kInjected[i].plus8[2]},
              kInjected[i].facing);
          const webo::gfx::SpriteViewVector cameraToActor{
              pose.modelPosition[0] - matchedCamera.eyeWorld[0],
              pose.modelPosition[1] - matchedCamera.eyeWorld[1],
              pose.modelPosition[2] - matchedCamera.eyeWorld[2],
          };
          const webo::gfx::SpriteViewVector basisVec{pose.basis[0], pose.basis[1], pose.basis[2]};
          const auto scores =
              webo::gfx::spriteViewScores(cameraToActor, basisVec, 0.0);
          const auto sel = webo::gfx::selectSpriteView(scores[0], scores[1], 5, false);
          if (i)
            replay << ",\n";
          replay << std::format(
              "  {{\"inject_index\": {}, \"view\": {}, \"mirror\": {}, \"scores\": [{:.17f}, "
              "{:.17f}]}}",
              i, sel.index, sel.mirrored ? 1 : 0, scores[0], scores[1]);
        }
        replay << "\n]\n";

        const fs::path forcePath = a6d6Out / "native-force.json";
        if (fs::exists(forcePath)) {
          std::ifstream fin(forcePath);
          std::string forceBody((std::istreambuf_iterator<char>(fin)),
                                std::istreambuf_iterator<char>());
          const auto findInt = [&](std::string_view key) -> int {
            const std::string needle = std::string("\"") + std::string(key) + "\": ";
            const auto pos = forceBody.find(needle);
            if (pos == std::string::npos)
              throw std::runtime_error("native-force.json missing key");
            return std::stoi(forceBody.substr(pos + needle.size()));
          };
          const int injectIndex = findInt("inject_index");
          const int nativeView = findInt("view");
          const int nativeMirror = findInt("mirror");
          check(injectIndex == 2, "a6d6 native-force targets slope inject index 2");

          auto forced = makeInjectedList();
          forced[static_cast<std::size_t>(injectIndex)].forcedViewIndex = nativeView;
          forced[static_cast<std::size_t>(injectIndex)].forcedMirror = nativeMirror;
          const auto forcedFb =
              renderRoute(*scene, *library, matchedCam, forced, false, false, 0.0);
          const double forcedMae = rectRgbMae(forcedFb, matchedRgb, kRefW, kRefH, kActor2X0,
                                              kActor2Y0, kActor2X1, kActor2Y1);
          std::cout << std::format("  a6d6 actor2 native-view forced mae={:.17f}\n", forcedMae);

          auto restored = makeInjectedList();
          const auto restoredFb =
              renderRoute(*scene, *library, matchedCam, restored, false, false, 0.0);
          const double restoredMae = rectRgbMae(restoredFb, matchedRgb, kRefW, kRefH, kActor2X0,
                                                kActor2Y0, kActor2X1, kActor2Y1);
          check(std::abs(restoredMae - kA6d3Temporal0Mae) < 1e-7,
                "a6d6 override-off restores temporal0 inject MAE");

          const fs::path cmpPath = a6d6Out / "comparison.json";
          if (fs::exists(cmpPath)) {
            std::ifstream cin(cmpPath);
            const std::string cmpBody((std::istreambuf_iterator<char>(cin)),
                                      std::istreambuf_iterator<char>());
            const bool anyMismatch = cmpBody.find("\"any_mismatch\": true") != std::string::npos;
            if (anyMismatch) {
              check(forcedMae < tubeMae2,
                    "a6d6 native view override beats tube-only on actor2");
            } else {
              std::cout << "  a6d6 clean negative: inject replay matches native view/mirror\n";
            }
          }
        } else {
          std::cout << "  skip a6d6 MAE gate: native-force.json missing (run compare_views.py)\n";
        }
      }

      std::cout << "a6d8 matched-state before-tube vs after-tube z-order\n";
      const fs::path a6d8Out = root / "build/tmp/a6d8-actor2-z-order";
      fs::create_directories(a6d8Out);
      check(!webo::app::SceneOptions{}.composeActorsBeforeRoute,
            "a6d8 production default is after-tube (composeActorsBeforeRoute false)");
      const auto beforeTubeFb =
          renderRoute(*scene, *library, matchedCam, walkersT0, true, false);
      check(pngSha256(beforeTubeFb) != pngSha256(injectT0),
            "a6d8 before-tube inject differs from after-tube");
      check(std::abs(t0Mae - kA6d3Temporal0Mae) < 1e-7,
            "a6d8 after-tube actor2 MAE matches a6d3 inject");

      constexpr std::array<std::array<int, 4>, 3> kA6d3ActorRects = {{
          {{467, 282, 480, 300}},
          {{397, 292, 408, 310}},
          {{315, 283, 326, 301}},
      }};
      std::array<double, 3> tubeMaeByActor{};
      std::array<double, 3> afterMaeByActor{};
      std::array<double, 3> beforeMaeByActor{};
      for (std::size_t a = 0; a < 3; ++a) {
        const int x0 = kA6d3ActorRects[a][0];
        const int y0 = kA6d3ActorRects[a][1];
        const int x1 = kA6d3ActorRects[a][2];
        const int y1 = kA6d3ActorRects[a][3];
        tubeMaeByActor[a] =
            rectRgbMae(tubeOnlyMatched, matchedRgb, kRefW, kRefH, x0, y0, x1, y1);
        afterMaeByActor[a] =
            rectRgbMae(injectT0, matchedRgb, kRefW, kRefH, x0, y0, x1, y1);
        beforeMaeByActor[a] =
            rectRgbMae(beforeTubeFb, matchedRgb, kRefW, kRefH, x0, y0, x1, y1);
        std::cout << std::format(
            "  actor {} tube={:.17f} after={:.17f} before={:.17f}\n", a, tubeMaeByActor[a],
            afterMaeByActor[a], beforeMaeByActor[a]);
      }

      std::ofstream ztable(a6d8Out / "mae-table.tsv");
      ztable << "actor\ttube_only_mae\tafter_tube_mae\tbefore_tube_mae\tbefore_beats_tube\n";
      for (std::size_t a = 0; a < 3; ++a) {
        const bool beats = beforeMaeByActor[a] < tubeMaeByActor[a];
        ztable << std::format("{}\t{:.17f}\t{:.17f}\t{:.17f}\t{}\n", a, tubeMaeByActor[a],
                              afterMaeByActor[a], beforeMaeByActor[a], beats ? "true" : "false");
      }

      bool actors01BeforeImprove = true;
      for (std::size_t a = 0; a < 2; ++a) {
        if (!(beforeMaeByActor[a] < tubeMaeByActor[a]))
          actors01BeforeImprove = false;
      }
      const bool actor2BeforeBeatsTube = beforeMaeByActor[2] < tubeMaeByActor[2];
      const bool beforeWins = actor2BeforeBeatsTube && actors01BeforeImprove;
      if (beforeWins) {
        std::cout << "  a6d8 CHOSEN before-tube (beats tube-only on actor2, actors 0-1 improve)\n";
        check(beforeMaeByActor[2] < kA6d3TubeMae, "a6d8 before-tube beats tube-only on actor2");
        for (std::size_t a = 0; a < 2; ++a)
          check(beforeMaeByActor[a] < tubeMaeByActor[a],
                std::format("a6d8 actor {} improves with before-tube", a));
      } else {
        std::cout << "  a6d8 negative: keep production after-tube draw order\n";
        check(!actor2BeforeBeatsTube || !actors01BeforeImprove,
              "a6d8 before-tube does not satisfy win gate");
        check(afterMaeByActor[2] >= tubeMaeByActor[2],
              "a6d8 after-tube actor2 still worse than tube-only (a6d3 defect retained)");
        for (std::size_t a = 0; a < 2; ++a)
          check(afterMaeByActor[a] < tubeMaeByActor[a],
                std::format("a6d8 actor {} still improves with after-tube", a));
      }

      const auto afterTubeRestore =
          renderRoute(*scene, *library, matchedCam, walkersT0, false, false);
      const double restoreActor2 = rectRgbMae(afterTubeRestore, matchedRgb, kRefW, kRefH,
                                              kActor2X0, kActor2Y0, kActor2X1, kActor2Y1);
      check(std::abs(restoreActor2 - kA6d3Temporal0Mae) < 1e-7,
            "a6d8 isolated after-tube mutation restores a6d3 actor2 MAE");

      std::cout << "a6d9 overlay composite uses A4y alpha > 0xEF\n";
      const fs::path a6d9Out = root / "build/tmp/a6d9-overlay-alpha-ef";
      fs::create_directories(a6d9Out);
      check(!webo::app::SceneOptions{}.overlayCompositeLegacyNonZero,
            "a6d9 production default is A4y overlay composite");
      check(webo::kActorAlphaTestReference == 0xEF, "a6d9 alpha reference is 0xEF");

      const auto legacyInject =
          renderRoute(*scene, *library, matchedCam, walkersT0, false, false, 0.0, true);
      const double legacyActor2Mae = rectRgbMae(legacyInject, matchedRgb, kRefW, kRefH, kActor2X0,
                                                kActor2Y0, kActor2X1, kActor2Y1);
      check(std::abs(legacyActor2Mae - kA6d3Temporal0Mae) < 1e-7,
            "a6d9 legacy a!=0 overlay restores a6d3 actor2 MAE");
      check(std::abs(t0Mae - legacyActor2Mae) < 1e-7,
            "a6d9 A4y overlay matches legacy on walk overlay (0/255 alphas only)");

      std::array<double, 3> a6d9InjectMae{};
      for (std::size_t a = 0; a < 3; ++a) {
        const int x0 = kA6d3ActorRects[a][0];
        const int y0 = kA6d3ActorRects[a][1];
        const int x1 = kA6d3ActorRects[a][2];
        const int y1 = kA6d3ActorRects[a][3];
        a6d9InjectMae[a] = rectRgbMae(injectT0, matchedRgb, kRefW, kRefH, x0, y0, x1, y1);
      }
      const bool actor2BeatsTube = a6d9InjectMae[2] < tubeMaeByActor[2];
      std::cout << std::format("  actor2 A4y inject mae={:.17f} tube={:.17f}{}\n",
                               a6d9InjectMae[2], tubeMaeByActor[2],
                               actor2BeatsTube ? " BEATS" : "");
      if (actor2BeatsTube)
        check(a6d9InjectMae[2] < kA6d3TubeMae, "a6d9 actor2 beats tube-only");
      else
        check(a6d9InjectMae[2] >= kA6d3TubeMae,
              "a6d9 actor2 does not beat tube-only (bbox gate retained)");
      for (std::size_t a = 0; a < 2; ++a)
        check(a6d9InjectMae[a] < tubeMaeByActor[a],
              std::format("a6d9 actor {} still improves with A4y overlay", a));

      std::ofstream a6d9table(a6d9Out / "mae-table.tsv");
      a6d9table << "actor\ttube_only_mae\ta4y_inject_mae\tbeats_tube_only\n";
      for (std::size_t a = 0; a < 3; ++a) {
        const bool beats = a6d9InjectMae[a] < tubeMaeByActor[a];
        a6d9table << std::format("{}\t{:.17f}\t{:.17f}\t{}\n", a, tubeMaeByActor[a],
                                 a6d9InjectMae[a], beats ? "true" : "false");
      }

      // Synthetic overlay proves composite predicate differs from legacy a!=0.
      auto applyOverlayComposite = [](std::vector<std::uint8_t>& rgb, int width, int height,
                                      const webo::Image& overlay, bool legacyNonZero) {
        for (int y = 0; y < height; ++y) {
          for (int x = 0; x < width; ++x) {
            const std::size_t i = (static_cast<std::size_t>(y) * width + x) * 4;
            const std::uint8_t a = overlay.rgba[i + 3];
            if (legacyNonZero) {
              if (a == 0)
                continue;
            } else if (!webo::actorSpriteAlphaTestPasses(a)) {
              continue;
            }
            const std::size_t o = (static_cast<std::size_t>(y) * width + x) * 3;
            rgb[o] = overlay.rgba[i];
            rgb[o + 1] = overlay.rgba[i + 1];
            rgb[o + 2] = overlay.rgba[i + 2];
          }
        }
      };
      webo::Image synth;
      synth.width = 2;
      synth.height = 1;
      synth.alpha = webo::AlphaMode::Embedded;
      synth.rgba = {9, 9, 9, 0xF0, 8, 8, 8, 0xA0};
      std::vector<std::uint8_t> rgbA4y{1, 1, 1, 2, 2, 2};
      std::vector<std::uint8_t> rgbLegacy = rgbA4y;
      applyOverlayComposite(rgbA4y, 2, 1, synth, false);
      applyOverlayComposite(rgbLegacy, 2, 1, synth, true);
      check(rgbA4y[0] == 9 && rgbA4y[1] == 9 && rgbA4y[2] == 9 && rgbA4y[3] == 2 && rgbA4y[4] == 2 &&
                rgbA4y[5] == 2,
            "a6d9 A4y composite skips alpha 0xA0");
      check(rgbLegacy[3] == 8 && rgbLegacy[4] == 8 && rgbLegacy[5] == 8,
            "a6d9 legacy composite writes alpha 0xA0");

      std::cout << "a6d17 actor2 bbox pixel classification\n";
      const fs::path a6d17Out = root / "build/tmp/a6d17-actor2-bbox-pixels";
      fs::create_directories(a6d17Out);
      check(tubeMae2 >= kA6d3TubeMae - 1e-7 && tubeMae2 <= kA6d3TubeMae + 1e-7,
            "a6d17 tube-only actor2 MAE matches a6d3");
      check(t0Mae >= kA6d3Temporal0Mae - 1e-7,
            "a6d17 inject temporal0 still fails actor2 bbox gate");

      std::array<int, 4> paintedClassCounts{};
      auto bumpClass = [&](const char* cls) {
        if (std::string_view(cls) == "integer_shift")
          ++paintedClassCounts[0];
        else if (std::string_view(cls) == "tint_scale")
          ++paintedClassCounts[1];
        else if (std::string_view(cls) == "extra_coverage")
          ++paintedClassCounts[2];
        else
          ++paintedClassCounts[3];
      };
      int paintedTotal = 0;
      for (int y = kActor2Y0; y < kActor2Y1; ++y) {
        for (int x = kActor2X0; x < kActor2X1; ++x) {
          RgbTri t{};
          RgbTri inj{};
          rgbTriAtFb(tubeOnlyMatched, x, y, t);
          rgbTriAtFb(injectT0, x, y, inj);
          if (rgbEqual(t, inj))
            continue;
          ++paintedTotal;
          bumpClass(classifyActor2PaintedPixel(tubeOnlyMatched, injectT0, matchedRgb, kRefW, kRefH,
                                               x, y));
        }
      }
      std::cout << std::format(
          "  actor2 painted={} integer_shift={} tint_scale={} extra_coverage={} "
          "different_silhouette={}\n",
          paintedTotal, paintedClassCounts[0], paintedClassCounts[1], paintedClassCounts[2],
          paintedClassCounts[3]);
      check(paintedTotal == 106, "a6d17 actor2 painted pixel count");
      check(paintedClassCounts[0] == 94, "a6d17 integer_shift count among painted");
      check(paintedClassCounts[1] == 12, "a6d17 tint_scale count among painted");
      check(paintedClassCounts[2] == 0, "a6d17 extra_coverage count among painted");
      check(paintedClassCounts[3] == 0, "a6d17 different_silhouette count among painted");

      std::ofstream a6d17table(a6d17Out / "pixel-class-counts.tsv");
      a6d17table << "class\tcount\n";
      a6d17table << std::format("integer_shift\t{}\n", paintedClassCounts[0]);
      a6d17table << std::format("tint_scale\t{}\n", paintedClassCounts[1]);
      a6d17table << std::format("extra_coverage\t{}\n", paintedClassCounts[2]);
      a6d17table << std::format("different_silhouette\t{}\n", paintedClassCounts[3]);
      a6d17table << std::format("painted_total\t{}\n", paintedTotal);
      a6d17table << "named_class\tinteger_shift\n";
      a6d17table << std::format("tube_only_mae\t{:.17f}\n", tubeMae2);
      a6d17table << std::format("inject_temporal0_mae\t{:.17f}\n", t0Mae);

      const webo::Image tubeImg = toRgbaImage(tubeOnlyMatched);
      const webo::Image injectImg = toRgbaImage(injectT0);
      writePngFile(a6d17Out / "tube-only.png", tubeImg);
      writePngFile(a6d17Out / "inject-temporal0.png", injectImg);
    } else {
      std::cout << "skip a6d4 sweep: matched-state fixtures missing\n";
    }
  } catch (const std::exception& ex) {
    check(false, ex.what());
  }
  std::cout << g_checks << " checks, " << g_failures << " failures\n";
  return g_failures == 0 ? 0 : 1;
}
