// Tests for surface face reconstruction and rasterisation.
//
// Synthetic cases lock the lattice → quad → mask path without game data.
// The retail pass asserts that every level yields at least some faces and
// that MM_CLIMB_002 still rebuilds the quads the RE notes counted by hand.

#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/SurfaceMesh.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <numbers>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

// Asset-free serialized retention fixtures. These specify file bits, not
// original x87 conversions, radial layers, geometry arithmetic or topology.
using TripleBits = std::array<std::uint64_t, 3>;
constexpr TripleBits ringBits(std::size_t i) {
  // Finite, non-axis and nonzero-height even for the first vertex. Every
  // component/position is distinguishable, including across the block boundary.
  return {0x3FF4000000000000ull + i, 0x400C000000000000ull + i,
          0x4049200000000000ull + i};
}
constexpr std::array<TripleBits, 3> kSurfaceBits = {{
    {0x3FF0000000000000ull, 0x4000000000000000ull, 0x4008000000000000ull},
    {0xC010000000000000ull, 0x4014000000000000ull, 0x4018000000000000ull},
    {0x401C000000000000ull, 0xC020000000000000ull, 0x4022000000000000ull}}};
constexpr std::uint64_t radiusBits(std::size_t i) { return 0x4049000000000000ull + i; }
constexpr std::uint64_t doubleBlockBits(std::size_t i) {
  if (i == 0 || i == 63)
    return 0x8000000000000000ull;
  if (i == 31 || i == 64)
    return 0x7FF8000000001234ull + i;
  return radiusBits(i);
}

void appendLE(std::vector<std::byte>& out, std::uint64_t value, std::size_t width) {
  for (std::size_t i = 0; i < width; ++i)
    out.push_back(static_cast<std::byte>((value >> (8 * i)) & 0xFF));
}
void appendU32(std::vector<std::byte>& out, std::uint32_t value) {
  appendLE(out, value, 4);
}
void appendBits(std::vector<std::byte>& out, std::uint64_t bits) {
  appendLE(out, bits, 8);
}
void appendName(std::vector<std::byte>& out, std::string_view name) {
  appendU32(out, static_cast<std::uint32_t>(name.size()));
  for (unsigned char c : name)
    out.push_back(static_cast<std::byte>(c));
}
void appendTriple(std::vector<std::byte>& out, const TripleBits& bits) {
  for (const auto component : bits)
    appendBits(out, component);
}

struct CylinderFixture {
  std::vector<std::byte> bytes;
  std::size_t worldStart = 0;
  std::size_t radiusStart = 0;
  std::size_t surfaceStart = 0;
  std::size_t secondaryStart = 0;
  std::size_t tileStart = 0;
  std::size_t tileEnd = 0;
};

CylinderFixture cylinderFixture(std::uint32_t rings = 2,
                                std::uint64_t thickness = 0x400A000000000000ull,
                                std::uint32_t radiusRings = 2,
                                std::span<const TripleBits> secondary = {},
                                bool specialDoubles = false) {
  CylinderFixture f;
  auto& out = f.bytes;
  appendName(out, "Level 23");
  for (std::uint32_t i = 0; i < 6; ++i)
    appendU32(out, 17 + i);  // Header words do not determine stream counts.
  appendLE(out, 0x01000100, 4);  // Four byte flags.
  for (int i = 0; i < 6; ++i)
    appendU32(out, 0);  // Three tables, no object, fourth and fifth tables.
  appendName(out, "Cylinder");
  f.worldStart = out.size();  // Independently expected to be byte 76 below.
  appendU32(out, 0x12345678);  // Existing unidentified faceCount field.
  appendBits(out, thickness);
  appendU32(out, rings);
  for (std::size_t i = 0; i < static_cast<std::size_t>(rings) * 64; ++i)
    appendTriple(out, ringBits(i));
  f.radiusStart = out.size();
  appendU32(out, radiusRings);
  for (std::size_t i = 0; i < static_cast<std::size_t>(radiusRings) * 32; ++i)
    appendBits(out, specialDoubles ? doubleBlockBits(i) : radiusBits(i));
  appendU32(out, 0x87654321);  // Existing unidentified 'one', not normalized.
  f.surfaceStart = out.size();
  appendU32(out, static_cast<std::uint32_t>(kSurfaceBits.size()));
  for (const auto& bits : kSurfaceBits)
    appendTriple(out, bits);
  f.secondaryStart = out.size();
  appendU32(out, static_cast<std::uint32_t>(secondary.size()));
  for (const auto& bits : secondary)
    appendTriple(out, bits);  // Opaque class-one second triples.
  appendU32(out, 2);  // Two references / two slots in each compatibility block.
  f.tileStart = out.size();
  appendName(out, "OUT_fixture");
  appendU32(out, 0);
  appendName(out, "INN_fixture");
  appendU32(out, 0);
  f.tileEnd = out.size();
  appendU32(out, 0);  // No class-one polygons.
  for (int i = 0; i < 12; ++i)
    appendBits(out, 0);
  appendName(out, "");
  appendBits(out, 0);
  appendBits(out, 0);
  appendU32(out, 0);  // No nested class-one records.
  appendU32(out, 96);
  appendBits(out, 0x3FD70A3D70A3D70Aull);
  for (int i = 0; i < 4; ++i)
    appendU32(out, 0);  // Integer, integer triple, integer and byte arrays.
  out.insert(out.end(), 6144, std::byte{0});
  appendName(out, "INN_world");
  appendName(out, "OUT_world");
  for (int i = 0; i < 32; ++i)
    appendU32(out, 0);  // Complete world tail, no later level sections needed.
  return f;
}

// A complete non-level mesh fragment accepted by the authentic locator. Its
// two plausible vertices give one ring/one angular step, independently of the
// structured block width. The radius word ends the plausible vertex run.
std::vector<std::byte> legacyCylinderFragment() {
  std::vector<std::byte> out;
  appendU32(out, 112);
  appendBits(out, 0x4024000000000000ull);  // 10.
  appendU32(out, 1);
  appendTriple(out, {0x8000000000000000ull, 0, 0x4049000000000000ull});
  appendTriple(out, {0, 0, 0x404E000000000000ull});
  appendU32(out, 1);
  appendBits(out, 0x4049000000000000ull);
  appendU32(out, 7);
  appendU32(out, 0);  // Empty surface.
  appendU32(out, 0);
  appendU32(out, 2);
  for (const auto name : {"OUT_old", "", "", "INN_old"})
    appendName(out, name);
  return out;
}

void checkTripleBits(const webo::Vec3& actual, const TripleBits& expected,
                     std::string_view label) {
  check(std::bit_cast<std::uint64_t>(actual.x) == expected[0], std::format("{} x bits", label));
  check(std::bit_cast<std::uint64_t>(actual.y) == expected[1], std::format("{} y bits", label));
  check(std::bit_cast<std::uint64_t>(actual.z) == expected[2], std::format("{} z bits", label));
}

void testCylinderLoading() {
  std::cout << "serialized cylinder loading (not original x87 conversion)\n";
  const auto f = cylinderFixture();
  const auto unchanged = f.bytes;
  auto mesh = webo::findCylinderMesh(f.bytes);
  check(mesh.has_value(), "non-axis/nonzero-height first vertex loads through public loader");
  if (mesh) {
    check(mesh->meshFallbackReason.empty(), "structured cylinder has no fallback diagnostic");
    check(mesh->faceCount == 0x12345678 && mesh->one == 0x87654321,
          "unidentified metadata retained without normalization");
    check(std::bit_cast<std::uint64_t>(mesh->thickness) == 0x400A000000000000ull,
          "thickness bits retained");
    check(mesh->ringCount == 2 && mesh->angularSteps == 32, "counted rings and block width");
    check(f.worldStart == 76 && mesh->offset == 76, "actual prefix/world boundary");
    check(f.radiusStart == 3164, "serialized first-block boundary");
    check(f.tileEnd == 3806 && mesh->endOffset == 3806 && mesh->endOffset < f.bytes.size(),
          "endOffset is compatibility tile boundary, not recovered final cursor");
    check(mesh->vertices.size() == 128, "complete serialized vertex count");
    for (std::size_t i = 0; i < mesh->vertices.size() && i < 128; ++i)
      checkTripleBits(mesh->vertices[i], ringBits(i), std::format("serialized vertex {}", i));
    check(mesh->radii.size() == 64, "complete repeated radius count");
    for (std::size_t i = 0; i < mesh->radii.size() && i < 64; ++i)
      check(std::bit_cast<std::uint64_t>(mesh->radii[i]) == radiusBits(i), "radius bits/order");
    check(mesh->surface.size() == kSurfaceBits.size(), "complete surface count");
    for (std::size_t i = 0; i < mesh->surface.size() && i < kSurfaceBits.size(); ++i)
      checkTripleBits(mesh->surface[i], kSurfaceBits[i], "surface bits/order");
    check(mesh->tileTextures == std::vector<std::string>{"OUT_fixture", ""}, "outer slots unchanged");
    check(mesh->innerTileTextures == std::vector<std::string>{"INN_fixture", ""}, "inner slots unchanged");
  }
  check(f.bytes == unchanged, "loader preserves input bytes");

  auto trailing = f.bytes;
  const auto fragment = legacyCylinderFragment();
  trailing.insert(trailing.end(), fragment.begin(), fragment.end());
  auto extra = webo::findCylinderMesh(trailing);
  check(extra && extra->meshFallbackReason.empty() && extra->offset == 76 && extra->endOffset == 3806,
        "later usable locator fragment accepted as trailing input, not selected");

  auto special = f.bytes;
  // Patch only vertex component bits. Never pass these through geometry math.
  const auto putBits = [](std::vector<std::byte>& into, std::size_t at, std::uint64_t bits) {
    for (std::size_t i = 0; i < 8; ++i)
      into[at + i] = static_cast<std::byte>((bits >> (8 * i)) & 0xFF);
  };
  putBits(special, f.worldStart + 16, 0x8000000000000000ull);
  putBits(special, f.worldStart + 16 + 65 * 24 + 8, 0x7FF8000000001234ull);
  const auto specialUnchanged = special;
  auto retained = webo::findCylinderMesh(special);
  check(retained && retained->meshFallbackReason.empty(), "special-bit vertices decoded without plausibility filter");
  check(retained && retained->vertices.size() == 128, "complete special-bit vertex count");
  if (retained && retained->vertices.size() == 128) {
    for (std::size_t i = 0; i < 128; ++i) {
      auto expected = ringBits(i);
      if (i == 0) expected[0] = 0x8000000000000000ull;
      if (i == 65) expected[1] = 0x7FF8000000001234ull;
      checkTripleBits(retained->vertices[i], expected, "serialized special retention");
    }
  }
  check(special == specialUnchanged, "special-bit input unchanged");

  auto old = webo::findCylinderMesh(fragment);
  check(old.has_value(), "non-level legacy mesh fragment still loads");
  if (old) {
    check(old->meshFallbackReason == "cylinder: recovered prefix reader at byte 120: truncated level header",
          "legacy success retains exact recovered rejection and cursor");
    check(old->faceCount == 112 && old->ringCount == 1 && old->angularSteps == 1 && old->one == 7,
          "legacy metadata preserved");
    check(old->offset == 0 && old->endOffset == fragment.size(), "legacy offset/endOffset preserved");
    check(old->tileTextures == std::vector<std::string>{"OUT_old", ""} &&
              old->innerTileTextures == std::vector<std::string>{"", "INN_old"},
          "legacy two-block tile slots preserved");
  }
  const auto empty = webo::findCylinderMesh({});
  check(!empty && empty.error() ==
        "cylinder: recovered prefix reader at byte 0: invalid level string read; legacy: no cylinder mesh found",
        "combined failure retains both causes through expected");

  auto badWorld = f.bytes;
  for (std::size_t i = 0; i < 4; ++i)
    badWorld[f.worldStart + 12 + i] = std::byte{0xFF};
  badWorld.insert(badWorld.end(), fragment.begin(), fragment.end());
  auto worldFallback = webo::findCylinderMesh(badWorld);
  check(worldFallback && worldFallback->offset == f.bytes.size() &&
            worldFallback->endOffset == badWorld.size() &&
            worldFallback->meshFallbackReason ==
                "cylinder: recovered world reader at byte 92: invalid world triple block count",
        "world-reader rejection enables diagnosed legacy loading at its actual offset");

  // Complete decoded worlds with invalid adapter metadata/tail. Each includes
  // a later fragment proved usable above: these errors must never enable it.
  const auto noFallback = [&](CylinderFixture invalid, std::string_view expected) {
    invalid.bytes.insert(invalid.bytes.end(), fragment.begin(), fragment.end());
    auto result = webo::findCylinderMesh(invalid.bytes);
    check(!result && result.error() == expected, std::format("decoded validation returns directly: {}", expected));
  };
  noFallback(cylinderFixture(0), "cylinder: ring count must be in [1,64]");
  noFallback(cylinderFixture(65), "cylinder: ring count must be in [1,64]");
  for (const auto bits : {0ull, 0x8000000000000000ull, 0xBFF0000000000000ull,
                          0x7FF0000000000000ull, 0x7FF800000000ABCDull})
    noFallback(cylinderFixture(2, bits), "cylinder: thickness must be finite and positive");
  // native-016 supersedes only the deliberate structured count-mismatch
  // rejection. Expectations below describe the complete compatibility output.
  const auto checkDoubleMesh = [&](const webo::CylinderMesh& actual,
                                   std::size_t secondCount, std::size_t secondaryCount) {
    check(actual.faceCount == 0x12345678 && actual.one == 0x87654321 &&
              actual.ringCount == 2 && actual.angularSteps == 32 &&
              std::bit_cast<std::uint64_t>(actual.thickness) == 0x400A000000000000ull,
          "independent double blocks preserve all metadata");
    const std::size_t expectedEnd = 76 + 16 + 2 * 1536 + 4 + secondCount * 256 +
                                    8 + 3 * 24 + 4 + secondaryCount * 24 +
                                    4 + 4 * 4 + 2 * 11;
    check(actual.offset == 76 && actual.endOffset == expectedEnd,
          "independently counted double/secondary payloads establish exact tile boundary");
    check(actual.meshFallbackReason.empty(), "independent double blocks use structured loading");
    check(actual.vertices.size() == 128 && actual.radii.size() == secondCount * 32 &&
              actual.surface.size() == 3,
          "complete independent double-block output array lengths, including zero");
    for (std::size_t i = 0; i < actual.vertices.size() && i < 128; ++i)
      checkTripleBits(actual.vertices[i], ringBits(i), "independent double-block vertex bits/order");
    for (std::size_t i = 0; i < actual.radii.size() && i < secondCount * 32; ++i)
      check(std::bit_cast<std::uint64_t>(actual.radii[i]) == doubleBlockBits(i),
            "complete double-block component bits/order without arithmetic");
    for (std::size_t i = 0; i < actual.surface.size() && i < 3; ++i)
      checkTripleBits(actual.surface[i], kSurfaceBits[i], "independent double-block surface bits/order");
    check(actual.tileTextures == std::vector<std::string>{"OUT_fixture", ""} &&
              actual.innerTileTextures == std::vector<std::string>{"INN_fixture", ""},
          "independent double blocks preserve both complete slot vectors");
  };
  // Supersedes native-014's intentional nonempty-secondary rejection. Expected
  // output is the independent zero-array baseline above, with only endOffset
  // advanced. Negative zero, NaNs and name/count-like words must stay opaque.
  constexpr std::array<TripleBits, 3> secondaryBits = {{
      {0x8000000000000000ull, 0x7FF8000000001234ull, 0x000000020000000Bull},
      {0x747869665F54554Full, 0x0000000000657275ull, 0xFFFFFFFFFFFFFFFFull},
      {0x7FF800000000ABCDull, 0x0000000100000000ull, 0x8000000000000000ull}}};
  for (const std::size_t secondCount : {0u, 1u, 2u, 3u}) {
    for (const std::size_t secondaryCount : {0u, 3u}) {
      auto independent = cylinderFixture(2, 0x400A000000000000ull, secondCount,
                                         std::span(secondaryBits).first(secondaryCount), true);
      for (const bool trailingInput : {false, true}) {
        if (trailingInput)
          independent.bytes.insert(independent.bytes.end(), fragment.begin(), fragment.end());
        const auto before = independent.bytes;
        auto result = webo::findCylinderMesh(independent.bytes);
        check(result.has_value(), std::format("two first blocks / {} second blocks / {} secondary triples load",
                                              secondCount, secondaryCount));
        if (result)
          checkDoubleMesh(*result, secondCount, secondaryCount);
        check(independent.bytes == before, "independent double-block input/trailing input unchanged");
      }
    }
    const auto independent = cylinderFixture(2, 0x400A000000000000ull, secondCount);
    // Every count/payload cut, including boundaries between double blocks.
    for (std::size_t cut = independent.radiusStart;
         cut < independent.radiusStart + 4 + secondCount * 256; ++cut) {
      auto result = webo::findCylinderMesh(std::span(independent.bytes).first(cut));
      const auto expected = cut < independent.radiusStart + 4
          ? "cylinder: recovered world reader at byte 3164: truncated world geometry field; legacy: no cylinder mesh found"
          : "cylinder: recovered world reader at byte 3168: invalid world double block count; legacy: no cylinder mesh found";
      check(!result && result.error() == expected,
            std::format("double-block truncation retains both exact causes at cut {}", cut));
    }
  }
  auto badSecondCount = f.bytes;
  for (std::size_t i = 0; i < 4; ++i)
    badSecondCount[f.radiusStart + i] = std::byte{0xFF};
  auto badSecond = webo::findCylinderMesh(badSecondCount);
  check(!badSecond && badSecond.error() ==
            "cylinder: recovered world reader at byte 3168: invalid world double block count; legacy: no cylinder mesh found",
        "malformed second count retains recovered and legacy causes through expected");
  auto invalidTail = cylinderFixture(2, 0x400A000000000000ull, 3);
  // Recovered reference values permit 513, while the compatibility slot reader
  // sees this as an implausible name length. A later usable locator must not run.
  invalidTail.bytes[invalidTail.tileStart + 4 + 11] = std::byte{1};
  invalidTail.bytes[invalidTail.tileStart + 4 + 11 + 1] = std::byte{2};
  noFallback(std::move(invalidTail), "cylinder: a tile texture name has an implausible length");
  auto legacyMismatch = fragment;
  legacyMismatch[64] = std::byte{2};
  const auto mismatchedLegacy = webo::findCylinderMesh(legacyMismatch);
  check(!mismatchedLegacy && mismatchedLegacy.error().find("cylinder: recovered prefix reader at byte ") == 0 &&
            mismatchedLegacy.error().find("; legacy: cylinder: the radius array's ring count does not match the mesh") !=
                std::string::npos,
        "legacy-only fragment retains radius-count equality and combined diagnostic");
  const auto checkSecondaryMesh = [&](const webo::CylinderMesh& actual, std::size_t count) {
    check(actual.meshFallbackReason.empty(), "secondary array uses structured loading without diagnostic");
    check(actual.faceCount == 0x12345678 && actual.one == 0x87654321 &&
              actual.ringCount == 2 && actual.angularSteps == 32 &&
              std::bit_cast<std::uint64_t>(actual.thickness) == 0x400A000000000000ull,
          "secondary array preserves all metadata against independent zero-array expectation");
    // Header + rings + radii + surface + secondary count/payload + tile count
    // + four slot lengths + two eleven-byte names. No recovered cursor is used.
    const std::size_t expectedEnd = 76 + 16 + 2 * 64 * 24 + 4 + 2 * 32 * 8 +
                                    8 + 3 * 24 + 4 + count * 24 + 4 + 4 * 4 + 2 * 11;
    check(actual.offset == 76 && actual.endOffset == expectedEnd,
          "secondary array preserves world start and exact independently calculated tile boundary");
    check(mesh && actual.endOffset == mesh->endOffset + count * 24,
          "secondary array advances baseline endOffset by exactly 24 bytes per entry");
    check(actual.vertices.size() == 128 && actual.radii.size() == 64 && actual.surface.size() == 3,
          "opaque secondary triples do not change any output array length");
    for (std::size_t i = 0; i < actual.vertices.size() && i < 128; ++i)
      checkTripleBits(actual.vertices[i], ringBits(i), "secondary case vertex bits/order");
    for (std::size_t i = 0; i < actual.radii.size() && i < 64; ++i)
      check(std::bit_cast<std::uint64_t>(actual.radii[i]) == radiusBits(i), "secondary case radius bits/order");
    for (std::size_t i = 0; i < actual.surface.size() && i < 3; ++i)
      checkTripleBits(actual.surface[i], kSurfaceBits[i], "secondary case surface bits/order");
    check(actual.tileTextures == std::vector<std::string>{"OUT_fixture", ""} &&
              actual.innerTileTextures == std::vector<std::string>{"INN_fixture", ""},
          "secondary case retains both complete zero-array tile-slot vectors");
  };
  for (const std::size_t count : {0u, 1u, 3u}) {
    auto populated = cylinderFixture(2, 0x400A000000000000ull, 2,
                                     std::span(secondaryBits).first(count));
    const auto before = populated.bytes;
    auto result = webo::findCylinderMesh(populated.bytes);
    check(result.has_value(), std::format("complete world with {} secondary triples loads", count));
    if (result)
      checkSecondaryMesh(*result, count);
    check(populated.bytes == before, "secondary-array input unchanged");
    populated.bytes.insert(populated.bytes.end(), fragment.begin(), fragment.end());
    const auto withTrailing = populated.bytes;
    result = webo::findCylinderMesh(populated.bytes);
    check(result.has_value(), "secondary-array world accepts later usable legacy fragment as trailing input");
    if (result)
      checkSecondaryMesh(*result, count);
    check(populated.bytes == withTrailing, "secondary-array trailing input unchanged");

    // Cut inside each count byte and each payload byte, including between
    // entries. Exercise the public fallback boundary and retain both causes.
    for (std::size_t cut = populated.secondaryStart;
         cut < populated.secondaryStart + 4 + count * 24; ++cut) {
      auto truncated = webo::findCylinderMesh(std::span(before).first(cut));
      const auto cause = cut < populated.secondaryStart + 4
                             ? "truncated level name count" : "invalid object triple count";
      check(!truncated && truncated.error().find("cylinder: recovered world reader at byte ") == 0 &&
                truncated.error().find(cause) != std::string::npos &&
                truncated.error().find("; legacy: no cylinder mesh found") != std::string::npos,
            std::format("secondary truncation retains both causes at cut {}", cut));
    }
  }
  auto nonzeroLegacy = fragment;
  nonzeroLegacy[84] = std::byte{1};  // After the legacy fragment's empty surface.
  auto legacyRejected = webo::findCylinderMesh(nonzeroLegacy);
  check(!legacyRejected && legacyRejected.error().find("cylinder: recovered prefix reader at byte ") == 0 &&
            legacyRejected.error().find("; legacy: cylinder: expected a zero word before the tile texture list") !=
                std::string::npos,
        "legacy fragment still rejects a nonzero secondary word with combined diagnostic");

  for (const auto rings : {1u, 64u}) {
    auto boundary = cylinderFixture(rings, 0x400A000000000000ull, rings);
    auto result = webo::findCylinderMesh(boundary.bytes);
    check(result && result->meshFallbackReason.empty() && result->ringCount == rings &&
              result->vertices.size() == rings * 64 && result->endOffset == boundary.tileEnd,
          "valid adapter ring-count boundary");
  }

  for (const auto cut : {std::size_t{0}, std::size_t{3}, f.worldStart - 1,
                         f.worldStart + 3, f.worldStart + 15, f.radiusStart - 1,
                         f.radiusStart + 3, f.surfaceStart + 4, f.tileStart + 3,
                         f.tileEnd - 1, f.bytes.size() - 1}) {
    auto result = webo::findCylinderMesh(std::span(f.bytes).first(cut));
    check(!result && result.error().find("recovered ") != std::string::npos &&
              result.error().find("reader at byte ") != std::string::npos &&
              result.error().find("; legacy: no cylinder mesh found") != std::string::npos,
          std::format("truncated prefix/world/tail returns both causes at cut {}", cut));
  }
  // Exercise the preserved hard tail failures through the legacy route too.
  for (const auto& [cut, cause] : std::array<std::pair<std::size_t, std::string_view>, 6>{{
           {64, "radius array's ring count does not match"},
           {70, "radius array runs past"},
           {78, "before the surface header"},
           {86, "before the tile texture list"},
           {92, "ends inside a tile texture list"},
           {96, "tile texture name has an implausible length"}}}) {
    auto result = webo::findCylinderMesh(std::span(fragment).first(cut));
    check(!result && result.error().find("; legacy: cylinder: ") != std::string::npos &&
              result.error().find(cause) != std::string::npos,
          std::format("legacy hard tail failure at cut {}", cut));
  }
}

void testUnroll() {
  std::cout << "unroll\n";
  const auto [a0, y0] = webo::unrollCylinder({0, 16, 60});
  check(std::abs(a0) < 1e-9 && y0 == 16, "forward axis unrolls to ang 0");
  const auto [a1, y1] = webo::unrollCylinder({-60, 0, 0});
  check(std::abs(a1 - 0.25) < 1e-9 && y1 == 0, "-X is a quarter turn");
}

void testRetail(const fs::path& dir) {
  std::cout << "retail surfaces\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  std::size_t built = 0, withRing = 0, withQuads = 0, totalQuads = 0, totalSolid = 0;
  std::size_t pixels = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob)
      continue;
    auto mesh = webo::findCylinderMesh(*blob);
    auto index = webo::indexLevel(*blob);
    if (!mesh || !index || index->grid0.empty())
      continue;
    auto faces = webo::buildSurfaceMesh(*mesh, index->grid0, index->grid1);
    if (!faces) {
      std::cerr << std::format("  FAIL  {}: {}\n", name, faces.error());
      ++g_failures;
      continue;
    }
    ++built;
    if (faces->ringStride > 0)
      ++withRing;
    if (!faces->quads.empty())
      ++withQuads;
    totalQuads += faces->quads.size();

    const auto mask = webo::rasteriseSurface(*faces, *mesh, 2);
    totalSolid += mask.solidCells();
    pixels += static_cast<std::size_t>(mask.width) * mask.height;

    if (name.find("MM_CLIMB_002") != std::string::npos) {
      // Hand-counted on this level during RE: 28 angular + 12 vertical on the
      // old header[3] window. The lattice is now the whole 49858-word array,
      // which contains that window, so the floor still applies.
      check(faces->angularQuads >= 28,
            std::format("climb angular quads (>=28, got {})", faces->angularQuads));
      check(faces->ringStride == 8224,
            std::format("climb ring stride 8224 (got {})", faces->ringStride));
      check(faces->ringQuads >= 12,
            std::format("climb ring quads (>=12, got {})", faces->ringQuads));
      check(mask.solidCells() > 100, "climb rasterises to a visible mask");
      // The quad pass requires a thickness partner. Where that slot is empty
      // the outer key+16 link is still floor: the live walkers cross columns
      // 128..144, which this mask used to leave empty. The matching key+Y
      // link at column 128, from y 32 to y 48, is the wall they turn at.
      const auto cell = [&](int col, double worldY) {
        const int py = static_cast<int>(std::lround((mesh->height() - worldY) * 2.0));
        return mask.solid[static_cast<std::size_t>(py) * mask.width + col];
      };
      check(!faces->openEdges.empty(), "climb keeps the edges the quad pass drops");
      check(cell(132, 32.0) == 1, "climb: walked ledge at column 132 is solid");
      check(cell(128, 40.0) == 1, "climb: wall at column 128 rises above the ledge");
      check(cell(160, 40.0) == 0, "climb: column 160 has floor and no wall above it");
      check(cell(136, 40.0) == 0,
            "climb: column 136, which the walkers cross, has no wall");
      // The right turn. key+8 at column 168 holds the post at 172, and the
      // walkers stop on 168 rather than crossing to it.
      check(cell(168, 40.0) == 1, "climb: right turn at column 168 is a wall");
      check(cell(164, 40.0) == 0, "climb: column 164, on the walked ledge, has no wall");
      check(cell(164, 32.0) == 1, "climb: the ledge is still solid at column 164");
      // Negative RE result: Y is derived, not read. Climb's stride (and Y/16)
      // never appear as aligned u32s in the file, and no header word holds Y.
      std::size_t asU32 = 0, asY16 = 0;
      for (std::size_t o = 0; o + 4 <= blob->size(); o += 4) {
        std::uint32_t w;
        std::memcpy(&w, blob->data() + o, 4);
        if (w == 8224)
          ++asU32;
        if (w == 514)
          ++asY16;
      }
      check(asU32 == 0, "climb has no aligned u32 == ring stride");
      check(asY16 == 0, "climb has no aligned u32 == Y/16");
      bool headerHoldsY = false;
      for (std::uint32_t w : index->header)
        if (w == 8224 || w == 514)
          headerHoldsY = true;
      check(!headerHoldsY, "climb header does not store the ring stride");
      std::cout << std::format(
          "  MM_CLIMB_002: {} quads (ang {}, ring {}), stride {}, mask {}x{} solid {}\n",
          faces->quads.size(), faces->angularQuads, faces->ringQuads, faces->ringStride,
          mask.width, mask.height, mask.solidCells());
    }

    // Corpus: when a stride is found, it is never one of the level header words.
    if (faces->ringStride > 0) {
      bool headerHoldsY = false;
      for (std::uint32_t w : index->header)
        if (w == static_cast<std::uint32_t>(faces->ringStride))
          headerHoldsY = true;
      check(!headerHoldsY, std::format("{}: header must not equal derived ring stride {}",
                                       name, faces->ringStride));
    }
  }

  const double coverage =
      pixels ? 100.0 * static_cast<double>(totalSolid) / static_cast<double>(pixels) : 0;
  std::cout << std::format(
      "  {} levels parsed, {} with quads, {} with a ring stride, {} quads total, "
      "corpus mask coverage {:.2f}%\n",
      built, withQuads, withRing, totalQuads, coverage);
  check(built == 102, std::format("every level parses (got {})", built));
  check(withQuads > 90, std::format("most levels produce quads (got {})", withQuads));
  check(withRing > 10, "several levels expose a ring stride");
  // Coverage is expected to be low: the surface is sparse platforms, not a
  // filled tube. Record it so a sudden fill would be noticed.
  check(coverage < 40.0, "corpus coverage stays sparse (<40%)");
}

// Skin tiles wrap the whole tube and are fully opaque. They cannot be the
// walkable mask: surface rasterisation stays sparse while every named tile
// has AlphaMode::Opaque and essentially no translucent or colour-keyed holes.
void testSkinNotSolidity(const fs::path& dir) {
  std::cout << "skin is not solidity\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  std::size_t tiles = 0, translucent = 0, totalPx = 0, nonOpaqueMode = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob)
      continue;
    auto mesh = webo::findCylinderMesh(*blob);
    if (!mesh)
      continue;
    for (const auto* block : {&mesh->tileTextures, &mesh->innerTileTextures}) {
      for (const auto& tile : *block) {
        if (tile.empty())
          continue;
        auto img = webo::loadTexture(*lib, webo::levelTextureAssetName(tile));
        if (!img)
          continue;
        ++tiles;
        if (img->alpha != webo::AlphaMode::Opaque)
          ++nonOpaqueMode;
        for (std::size_t i = 0; i < img->pixelCount(); ++i) {
          ++totalPx;
          if (img->rgba[i * 4 + 3] < 250)
            ++translucent;
        }
      }
    }
  }
  check(tiles == 4896, std::format("every named skin tile loads (got {})", tiles));
  check(nonOpaqueMode == 0, "no skin tile uses a transparency sigil");
  check(translucent == 0, "no translucent skin texels");
  check(totalPx > 0, "counted skin pixels");

  // Spot-check: opaque outer skin covers the full unroll; surface does not.
  for (const auto& name : webo::levelFileNames(*lib)) {
    if (name.find("MM_CLIMB_002") == std::string::npos)
      continue;
    auto blob = lib->read(name);
    auto mesh = webo::findCylinderMesh(*blob);
    auto index = webo::indexLevel(*blob);
    auto faces = webo::buildSurfaceMesh(*mesh, index->grid0, index->grid1);
    const auto mask = webo::rasteriseSurface(*faces, *mesh, 2);
    const double surfPct = 100.0 * static_cast<double>(mask.solidCells()) /
                           (static_cast<double>(mask.width) * mask.height);
    // Walls are a full ring tall, so the mask is denser than the floor strokes
    // alone. A filled tube would be several times this.
    check(surfPct < 35.0, std::format("climb surface stays sparse ({:.1f}%)", surfPct));
    check(mesh->tileTextures.size() == 48, "outer skin slot table is 48");
    std::size_t named = 0;
    for (const auto& t : mesh->tileTextures)
      if (!t.empty())
        ++named;
    check(named == 24, std::format("climb has a full 8x3 outer skin (got {})", named));
    break;
  }
}

}  // namespace

int main() {
  testCylinderLoading();
  testUnroll();

  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    if (fs::exists(fs::path(dir) / "lemmings.box")) {
      testRetail(dir);
      testSkinNotSolidity(dir);
    } else
      std::cout << "retail surfaces: skipped, no lemmings.box\n";
  } else {
    std::cout << "retail surfaces: skipped, WEBOLUTION_GAME_DIR is not set\n";
  }

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
