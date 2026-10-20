#include "app/ActorPose.h"

#include <cmath>
#include <cstdint>

namespace webo {
namespace {

int ftolTowardZero(double v) {
  if (v >= 0.0)
    return static_cast<int>(v);
  return -static_cast<int>(-v);
}

int sarBucketX(double x) {
  const int iv = ftolTowardZero(x);
  const int bias = iv >> 31 & 7;
  return (iv + bias) >> 3;
}

int sarBucketY(double y, int gridHeight) {
  int iv = ftolTowardZero(y);
  const int bias = iv >> 31 & 0xF;
  int row = (iv + bias) >> 4;
  if (gridHeight <= iv)
    row = ((gridHeight + (gridHeight >> 31 & 0xF)) >> 4) - 1;
  return row;
}

std::optional<Vec3> vertexAt(const ActorPoseLevel& level, int x, int y, int layer) {
  x &= 0xFF;
  if (y >= level.gridHeight)
    y = level.gridHeight - 1;
  if (y < 0)
    y = 0;
  const std::size_t key = static_cast<std::size_t>(layer + (x + y * 0x101) * 2);
  if (key >= level.grid0.size())
    return std::nullopt;
  const std::uint32_t index = level.grid0[key];
  if (index == 0xFFFFFFFFu || index >= level.surface.size())
    return std::nullopt;
  return level.surface[index];
}

// Native `0x0043D2B8`: bilinear on the cylinder template band at `world+0x18`
// for integer grid `(x,y)` and shell layer `param_4`. Offline we read the
// retail constant `CylinderMesh::vertices` (byte-identical template).
std::optional<Vec3> interpolateCylinderTemplateD2B8(const CylinderMesh& cylinder, int gridX,
                                                    int gridY, int layer, int gridHeight) {
  const int bucketX = sarBucketX(static_cast<double>(gridX));
  const int col0 = bucketX & 0x1F;
  const int col1 = (col0 + 1) & 0x1F;
  int row0 = sarBucketY(static_cast<double>(gridY), gridHeight);
  int row1 = row0 + 1;
  const int maxRow = ((gridHeight + (gridHeight >> 31 & 0xF)) >> 4) - 1;
  if (row1 > maxRow)
    row1 = maxRow;

  const double tx = (static_cast<double>(gridX) - static_cast<double>(bucketX << 3)) / 8.0;
  const double ty =
      (static_cast<double>(gridY) - static_cast<double>(row0 << 4)) / 16.0;
  const bool outer = layer == 1;

  const auto corner = [&](int col, int row) -> Vec3 {
    return cylinder.vertex(static_cast<std::uint32_t>(row),
                           static_cast<std::uint32_t>(col & 0x1F), outer);
  };

  const Vec3 p00 = corner(col0, row0);
  const Vec3 p10 = corner(col1, row0);
  const Vec3 p01 = corner(col0, row1);
  const Vec3 p11 = corner(col1, row1);

  const auto lerp3 = [](const Vec3& a, const Vec3& b, double t) {
    return Vec3{a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t};
  };
  const Vec3 top = lerp3(p00, p10, tx);
  const Vec3 bot = lerp3(p01, p11, tx);
  return lerp3(top, bot, ty);
}

std::optional<Vec3> vertexAtWithFallback(const ActorPoseLevel& level, int x, int y, int layer) {
  if (auto direct = vertexAt(level, x, y, layer))
    return direct;
  if (!level.cylinderTemplate)
    return std::nullopt;
  return interpolateCylinderTemplateD2B8(*level.cylinderTemplate, x, y, layer,
                                         level.gridHeight);
}

std::array<double, 3> vecAdd(const std::array<double, 3>& a, const std::array<double, 3>& b) {
  return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

std::array<double, 3> vecSub(const std::array<double, 3>& a, const std::array<double, 3>& b) {
  return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

std::array<double, 3> vecScale(const std::array<double, 3>& a, double t) {
  return {a[0] * t, a[1] * t, a[2] * t};
}

std::array<double, 3> vecLerp(const std::array<double, 3>& a, const std::array<double, 3>& b,
                              double t) {
  return vecAdd(a, vecScale(vecSub(b, a), t));
}

std::array<double, 3> toArray(const Vec3& v) {
  return {v.x, v.y, v.z};
}

std::array<double, 3> normalize(const std::array<double, 3>& v) {
  const double len = std::hypot(v[0], std::hypot(v[1], v[2]));
  if (len <= 0.0)
    return {0, 0, 0};
  return {v[0] / len, v[1] / len, v[2] / len};
}

}  // namespace

ActorTerrainSample terrainSampleFromLemming(const sim::Lemming& lemming, int terrainHeight,
                                            double zSlab) {
  ActorTerrainSample out;
  out.x = static_cast<double>(lemming.x);
  const int fileRow = terrainHeight - 1 - lemming.y;
  out.y = static_cast<double>(fileRow);
  out.z = zSlab;
  return out;
}

std::optional<std::array<double, 3>> interpolateCellMeshPosition(
    const ActorPoseLevel& level, const ActorTerrainSample& sample) {
  if (level.grid0.empty() || level.surface.empty())
    return std::nullopt;

  const int bucketX = sarBucketX(sample.x);
  const int col0 = bucketX & 0x1F;
  const int col1 = (col0 + 1) & 0x1F;
  const int row0 = sarBucketY(sample.y, level.gridHeight);
  int row1 = row0 + 1;
  const int maxRow = ((level.gridHeight + (level.gridHeight >> 31 & 0xF)) >> 4) - 1;
  if (row1 > maxRow)
    row1 = maxRow;

  const int gridX0 = col0 * 8;
  const int gridX1 = col1 * 8;
  const int gridY0 = row0 * 16;
  const int gridY1 = row1 * 16;

  const double tx = (sample.x - static_cast<double>(bucketX << 3)) / 8.0;
  const double ty = (sample.y - static_cast<double>(row0 << 4)) / 16.0;

  std::array<double, 3> layerPos[2];
  for (int layer = 0; layer < 2; ++layer) {
    const auto p00 = vertexAtWithFallback(level, gridX0, gridY0, layer);
    const auto p10 = vertexAtWithFallback(level, gridX1, gridY0, layer);
    const auto p01 = vertexAtWithFallback(level, gridX0, gridY1, layer);
    const auto p11 = vertexAtWithFallback(level, gridX1, gridY1, layer);
    if (!p00 || !p10 || !p01 || !p11)
      return std::nullopt;
    const auto top = vecLerp(toArray(*p00), toArray(*p10), tx);
    const auto bot = vecLerp(toArray(*p01), toArray(*p11), tx);
    layerPos[layer] = vecLerp(top, bot, ty);
  }

  return vecLerp(layerPos[0], layerPos[1], sample.z);
}

ActorPoseResult actorPoseFromNativeSample(const ActorPoseLevel& level,
                                          const ActorTerrainSample& plus8, int facing) {
  ActorPoseResult out{};
  const auto model = interpolateCellMeshPosition(level, plus8);
  if (!model)
    return out;
  out.modelPosition = *model;

  ActorTerrainSample basisSample = plus8;
  basisSample.x += static_cast<double>(facing) * 0.1;
  const auto surface = interpolateCellMeshPosition(level, basisSample);
  if (!surface)
    return out;
  out.basis = normalize(vecSub(*surface, out.modelPosition));
  return out;
}

ActorPoseResult actorPoseFromLemming(const ActorPoseLevel& level, const sim::Lemming& lemming,
                                     double zSlab) {
  const auto sample = terrainSampleFromLemming(lemming, level.gridHeight, zSlab);
  return actorPoseFromNativeSample(level, sample, static_cast<int>(lemming.facing));
}

}  // namespace webo
