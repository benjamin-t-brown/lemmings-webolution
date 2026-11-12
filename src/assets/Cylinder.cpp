#include "Cylinder.h"

#include "lemmings/level_name_tables.hpp"
#include "lemmings/world_geometry_prefix.hpp"

#include <bit>
#include <cmath>
#include <cstring>
#include <format>
#include <limits>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace webo {
namespace {

template <typename T>
T readAt(std::span<const std::byte> data, std::size_t offset) {
  T value{};
  std::memcpy(&value, data.data() + offset, sizeof(T));
  return value;
}

constexpr std::size_t kVertexBytes = 3 * sizeof(double);

// A vertex is plausible if it sits on a tube of sane radius at a sane height.
// The bounds are deliberately loose: they only have to reject the surrounding
// non-vertex data, and every accepted run is then checked for exact length.
bool plausibleVertex(const Vec3& v) {
  const double radius = std::hypot(v.x, v.z);
  if (!std::isfinite(radius) || !std::isfinite(v.y))
    return false;
  return radius > 5.0 && radius < 4096.0 && std::abs(v.y) < 4096.0;
}

// Both routes use the existing compatibility tail, including its two tile-slot
// blocks and boundary contract. This is separate from the recovered reader's
// class-one representation; no named-reference projection is made here.
std::expected<CylinderMesh, std::string> readSurfaceTail(
    std::span<const std::byte> data, CylinderMesh mesh, std::size_t p,
    std::optional<std::size_t> secondaryTripleCount = std::nullopt) {
  // Everything past the double blocks is addressed from a known offset rather
  // than searched for, so a short or malformed tail is a hard failure here
  // instead of being silently skipped.
  mesh.endOffset = p;

  const auto need = [&](std::size_t bytes) {
    return p <= data.size() && bytes <= data.size() - p;
  };

  // u32 one, u32 surfaceCount, then that many vertices.
  if (!need(8))
    return std::unexpected("cylinder: the file ends before the surface header");
  mesh.one = readAt<std::uint32_t>(data, p);
  const std::uint32_t surfaceCount = readAt<std::uint32_t>(data, p + 4);
  p += 8;
  if (!need(static_cast<std::size_t>(surfaceCount) * kVertexBytes))
    return std::unexpected("cylinder: the surface vertices run past the end of the file");
  mesh.surface.reserve(surfaceCount);
  for (std::size_t i = 0; i < surfaceCount; ++i) {
    const std::size_t v = p + i * kVertexBytes;
    mesh.surface.push_back({readAt<double>(data, v), readAt<double>(data, v + 8),
                            readAt<double>(data, v + 16)});
  }
  p += static_cast<std::size_t>(surfaceCount) * kVertexBytes;

  if (secondaryTripleCount) {
    // The recovered class-one reader already validated this count and payload.
    // Consume the u32 count and exactly 24 serialized bytes per opaque triple;
    // neither their values nor the reader's later named references are projected.
    if (!need(4))
      return std::unexpected("cylinder: the file ends before the secondary triple count");
    p += 4;
    if (*secondaryTripleCount > (data.size() - p) / kVertexBytes)
      return std::unexpected("cylinder: the secondary triples run past the end of the file");
    p += *secondaryTripleCount * kVertexBytes;
    if (!need(4))
      return std::unexpected("cylinder: the file ends before the tile texture list");
  } else {
    // Legacy-only fragments retain the zero-word policy and diagnostics.
    if (!need(8))
      return std::unexpected("cylinder: the file ends before the tile texture list");
    if (readAt<std::uint32_t>(data, p) != 0)
      return std::unexpected("cylinder: expected a zero word before the tile texture list");
    p += 4;
  }
  const std::uint32_t tileCount = readAt<std::uint32_t>(data, p);
  p += 4;
  if (tileCount > 4096)
    return std::unexpected("cylinder: implausible tile texture count");
  // Two blocks of `tileCount` slots: the outer skin, then the inner. The
  // count is stated once and the second block follows it with no header of
  // its own, which is why this reads the same loop twice rather than
  // looking for another count.
  const auto readTileBlock = [&](std::vector<std::string>& into) -> const char* {
    into.reserve(tileCount);
    for (std::uint32_t i = 0; i < tileCount; ++i) {
      if (!need(4))
        return "cylinder: the file ends inside a tile texture list";
      const std::uint32_t len = readAt<std::uint32_t>(data, p);
      p += 4;
      // An empty name is normal: half the slots in each block are empty,
      // because a slot names a tile on one surface only.
      if (len > 512 || !need(len))
        return "cylinder: a tile texture name has an implausible length";
      std::string name(len, '\0');
      std::memcpy(name.data(), data.data() + p, len);
      p += len;
      into.push_back(std::move(name));
    }
    return nullptr;
  };

  if (const char* err = readTileBlock(mesh.tileTextures))
    return std::unexpected(err);
  if (const char* err = readTileBlock(mesh.innerTileTextures))
    return std::unexpected(err);

  mesh.endOffset = p;
  return mesh;
}

// The locator retains its count-coupled radius stage before the shared tail.
std::expected<CylinderMesh, std::string> readMeshTail(
    std::span<const std::byte> data, CylinderMesh mesh, std::size_t p) {
  const std::uint32_t ringCount = mesh.ringCount;
  mesh.endOffset = p;
  const auto need = [&](std::size_t bytes) {
    return p <= data.size() && bytes <= data.size() - p;
  };

  // u32 ringCount again, then ringCount * angularSteps inner radii.
  if (!need(4) || readAt<std::uint32_t>(data, p) != ringCount)
    return std::unexpected("cylinder: the radius array's ring count does not match the mesh");
  p += 4;
  const std::size_t radiiCount = static_cast<std::size_t>(ringCount) * mesh.angularSteps;
  if (!need(radiiCount * sizeof(double)))
    return std::unexpected("cylinder: the radius array runs past the end of the file");
  mesh.radii.reserve(radiiCount);
  for (std::size_t i = 0; i < radiiCount; ++i)
    mesh.radii.push_back(readAt<double>(data, p + i * sizeof(double)));
  p += radiiCount * sizeof(double);

  return readSurfaceTail(data, std::move(mesh), p);
}

}  // namespace

const Vec3& CylinderMesh::vertex(std::uint32_t ring, std::uint32_t step, bool outer) const {
  const std::size_t index =
      (static_cast<std::size_t>(ring) * angularSteps + step) * 2 + (outer ? 1 : 0);
  return vertices[index];
}

double CylinderMesh::innerRadius() const {
  if (vertices.empty())
    return 0;
  return std::hypot(vertices[0].x, vertices[0].z);
}

double CylinderMesh::outerRadius() const { return innerRadius() + thickness; }

double CylinderMesh::ringSpacing() const {
  if (ringCount < 2)
    return 0;
  return vertex(1, 0, false).y - vertex(0, 0, false).y;
}

double CylinderMesh::height() const {
  if (ringCount == 0)
    return 0;
  return vertex(ringCount - 1, 0, false).y - vertex(0, 0, false).y;
}

namespace {

std::expected<CylinderMesh, std::string> findLegacyCylinderMesh(std::span<const std::byte> data) {
  // The header is u32 faceCount, f64 thickness, u32 ringCount, so the vertex
  // run starts 16 bytes in. Scan for the run and validate backwards.
  constexpr std::size_t kHeaderBytes = 16;

  for (std::size_t at = kHeaderBytes; at + kVertexBytes <= data.size(); ++at) {
    const Vec3 first{readAt<double>(data, at), readAt<double>(data, at + 8),
                     readAt<double>(data, at + 16)};
    // The first vertex of the first ring is on the axis' zero angle at the
    // inner radius: x and y are zero (x as a negative zero, in the data) and
    // z is the radius.
    if (first.x != 0.0 || first.y != 0.0 || !(first.z > 10.0 && first.z < 4096.0))
      continue;

    const std::uint32_t ringCount = readAt<std::uint32_t>(data, at - 4);
    const double thickness = readAt<double>(data, at - 12);
    const std::uint32_t faceCount = readAt<std::uint32_t>(data, at - 16);
    if (ringCount == 0 || ringCount > 64 || !std::isfinite(thickness) || thickness <= 0)
      continue;

    // Accept the run only if it is exactly ringCount whole rings long and the
    // ring width is consistent, which is what makes this a match rather than
    // a coincidence.
    std::size_t count = 0;
    while (at + (count + 1) * kVertexBytes <= data.size()) {
      const std::size_t p = at + count * kVertexBytes;
      const Vec3 v{readAt<double>(data, p), readAt<double>(data, p + 8),
                   readAt<double>(data, p + 16)};
      if (!plausibleVertex(v))
        break;
      ++count;
    }
    if (count == 0 || count % (static_cast<std::size_t>(ringCount) * 2) != 0)
      continue;
    const std::size_t perRing = count / ringCount;
    if (perRing % 2 != 0)
      continue;

    CylinderMesh mesh;
    mesh.faceCount = faceCount;
    mesh.thickness = thickness;
    mesh.ringCount = ringCount;
    mesh.angularSteps = static_cast<std::uint32_t>(perRing / 2);
    mesh.offset = at - kHeaderBytes;
    mesh.vertices.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
      const std::size_t p = at + i * kVertexBytes;
      mesh.vertices.push_back({readAt<double>(data, p), readAt<double>(data, p + 8),
                               readAt<double>(data, p + 16)});
    }

    return readMeshTail(data, std::move(mesh), at + count * kVertexBytes);
  }

  return std::unexpected("no cylinder mesh found");
}

}  // namespace

std::expected<CylinderMesh, std::string> findCylinderMesh(std::span<const std::byte> data) {
  // LevelByteStream borrows its bytes. Keep the adapted storage alive for both
  // unchanged recovered reads; neither later level sections nor EOF is needed.
  std::vector<std::uint8_t> bytes;
  bytes.reserve(data.size());
  for (const auto byte : data)
    bytes.push_back(std::to_integer<std::uint8_t>(byte));
  lemmings::LevelByteStream stream{bytes};
  lemmings::WorldGeometryPrefix world;
  std::size_t worldStart = 0;
  const char* section = "prefix";
  try {
    (void)lemmings::read_compiled_level_prefix(stream);
    worldStart = stream.cursor;
    section = "world";
    world = lemmings::read_world_geometry_prefix(stream);
  } catch (const std::invalid_argument& error) {
    const auto reason = std::format("cylinder: recovered {} reader at byte {}: {}",
                                    section, stream.cursor, error.what());
    auto legacy = findLegacyCylinderMesh(data);
    if (!legacy)
      return std::unexpected(std::format("{}; legacy: {}", reason, legacy.error()));
    legacy->meshFallbackReason = reason;
    return legacy;
  }

  // These are existing portable adapter policies, not original validation.
  // Decoded metadata and compatibility-tail errors must not enter the locator.
  if (world.first_blocks.empty() || world.first_blocks.size() > 64)
    return std::unexpected("cylinder: ring count must be in [1,64]");
  const double thickness = std::bit_cast<double>(world.following_double_bits);
  if (!std::isfinite(thickness) || thickness <= 0)
    return std::unexpected("cylinder: thickness must be finite and positive");

  // 0x0043D09A reads 64 triples per counted block. Preserve serialized order
  // and binary64 bits; this does not establish the renderer's layer convention.
  constexpr std::size_t kTriplesPerBlock =
      std::tuple_size_v<lemmings::WorldGeometryPrefix::TripleBlock>;
  constexpr std::size_t kSerializedBlockBytes = kTriplesPerBlock * 3 * 8;
  static_assert(kTriplesPerBlock == 64 && kSerializedBlockBytes == 1536);
  constexpr std::size_t kSerializedHeaderBytes = 4 + 8 + 4;
  constexpr std::size_t kDoublesPerBlock =
      std::tuple_size_v<lemmings::WorldGeometryPrefix::DoubleBlock>;
  constexpr std::size_t kSerializedDoubleBlockBytes = kDoublesPerBlock * 8;
  static_assert(kDoublesPerBlock == 32 && kSerializedDoubleBlockBytes == 256);

  // Establish the actual post-second-array boundary without overflowing cursor
  // additions or payload multiplications, even though the reader bounded both.
  std::size_t tailStart = worldStart;
  const auto advance = [&](std::size_t count, std::size_t width) {
    if (tailStart > data.size() || count > (data.size() - tailStart) / width)
      return false;
    tailStart += count * width;
    return true;
  };
  if (!advance(1, kSerializedHeaderBytes) ||
      !advance(world.first_blocks.size(), kSerializedBlockBytes) ||
      !advance(1, 4) ||
      !advance(world.second_blocks.size(), kSerializedDoubleBlockBytes))
    return std::unexpected("cylinder: decoded block boundary runs past the end of the file");

  CylinderMesh mesh;
  mesh.faceCount = world.leading_value;  // Retain the existing unidentified field.
  mesh.thickness = thickness;
  mesh.ringCount = static_cast<std::uint32_t>(world.first_blocks.size());
  mesh.angularSteps = kTriplesPerBlock / 2;
  mesh.offset = worldStart;
  mesh.vertices.reserve(world.first_blocks.size() * kTriplesPerBlock);
  for (const auto& block : world.first_blocks) {
    for (const auto& triple : block) {
      mesh.vertices.push_back({std::bit_cast<double>(triple[0]),
                               std::bit_cast<double>(triple[1]),
                               std::bit_cast<double>(triple[2])});
    }
  }

  // 0x0043D060 reads 32 binary64 components per independently counted block.
  // Keep the compatibility field name without imposing a vertex relationship.
  mesh.radii.reserve(world.second_blocks.size() * kDoublesPerBlock);
  for (const auto& block : world.second_blocks) {
    for (const auto bits : block)
      mesh.radii.push_back(std::bit_cast<double>(bits));
  }

  return readSurfaceTail(data, std::move(mesh), tailStart,
                      world.following_class_one.second_triples.size());
}

}  // namespace webo
