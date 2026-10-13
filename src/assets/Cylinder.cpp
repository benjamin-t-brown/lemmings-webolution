#include "Cylinder.h"

#include <cmath>
#include <cstring>
#include <format>
#include <limits>

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

std::expected<CylinderMesh, std::string> findCylinderMesh(std::span<const std::byte> data) {
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
    mesh.endOffset = at + count * kVertexBytes;
    mesh.vertices.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
      const std::size_t p = at + i * kVertexBytes;
      mesh.vertices.push_back({readAt<double>(data, p), readAt<double>(data, p + 8),
                               readAt<double>(data, p + 16)});
    }
    return mesh;
  }

  return std::unexpected("no cylinder mesh found");
}

}  // namespace webo
