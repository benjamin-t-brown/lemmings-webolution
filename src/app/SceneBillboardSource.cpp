#include "SceneBillboardSource.h"

#include <cmath>
namespace webo::app {
namespace {

using Point = std::array<double, 3>;

bool finite(const BillboardAffine3x4& m) {
  for (double x : m.values) if (!std::isfinite(x)) return false;
  return true;
}

Point apply(const BillboardAffine3x4& m, Point p) {
  Point out{};
  for (std::size_t r = 0; r < 3; ++r)
    out[r] = m.values[r * 4] * p[0] + m.values[r * 4 + 1] * p[1] +
             m.values[r * 4 + 2] * p[2] + m.values[r * 4 + 3];
  return out;
}

BillboardAffine3x4 compose(const BillboardAffine3x4& a, const BillboardAffine3x4& b) {
  BillboardAffine3x4 out;
  for (std::size_t r = 0; r < 3; ++r) {
    for (std::size_t c = 0; c < 3; ++c)
      out.values[r * 4 + c] = a.values[r * 4] * b.values[c] +
          a.values[r * 4 + 1] * b.values[4 + c] + a.values[r * 4 + 2] * b.values[8 + c];
    out.values[r * 4 + 3] = a.values[r * 4] * b.values[3] +
        a.values[r * 4 + 1] * b.values[7] + a.values[r * 4 + 2] * b.values[11] + a.values[r * 4 + 3];
  }
  return out;
}

bool finite(Point p) { return std::isfinite(p[0]) && std::isfinite(p[1]) && std::isfinite(p[2]); }

// SceneNode stores a native row-vector 3x3 followed by translation at 9..11.
// Convert it to the adapter's column-vector row-major 3x4 affine convention.
BillboardAffine3x4 fromNativeSceneMatrix(const std::array<double,12>& m) {
  return {{{m[0],m[3],m[6],m[9],
            m[1],m[4],m[7],m[10],
            m[2],m[5],m[8],m[11]}}};
}

}  // namespace

std::expected<SceneBillboardSource, std::string> makeSceneBillboardSource(
    const SceneNode& node, const BillboardSourceInputs& in) {
  if (node.type != 2) return std::unexpected("billboard source requires a type-2 node");
  if (node.type2.branch != 0) return std::unexpected("nonzero type-2 branch is unsupported");
  if (!finite(in.parentCamera) || !finite(in.sourceTransform) || !finite(in.projectionTransform) ||
      !std::isfinite(in.center[0]) || !std::isfinite(in.center[1]))
    return std::unexpected("billboard inputs must be finite");

  // The leading triple stores the extents; branch-0 doubles are the UV tuple.
  const double a = node.type2.leading[0];
  const double b = node.type2.leading[1];
  const double h = node.type2.leading[2];
  for (double v : node.matrix) if (!std::isfinite(v)) return std::unexpected("node matrix must be finite");
  if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(h))
    return std::unexpected("billboard extents must be finite");
  for (double v : node.type2.branchDoubles) if (!std::isfinite(v))
    return std::unexpected("billboard UV inputs must be finite");

  const BillboardAffine3x4 nodeMatrix = fromNativeSceneMatrix(node.matrix);
  const auto toSource = compose(in.sourceTransform, compose(in.parentCamera, nodeMatrix));
  const Point origin = apply(toSource, {0, 0, 0});
  if (!finite(origin)) return std::unexpected("transformed billboard center is not finite");

  // Source point offsets in the established (a,b,h) convention.
  const std::array<Point, 4> offsets{{{-a/2, -h/2, -a/2}, {-b/2, h/2, -b/2},
                                      { b/2, h/2, -b/2}, { a/2, -h/2, -a/2}}};
  SceneBillboardSource out;
  out.sourceOffset = node.sourceOffset;
  out.sourceName = node.name;
  out.textureName = node.type2.branchName;
  out.sourceVisibility = node.type2.visibility;
  out.blendSelector = node.type2.trailingWord;
  std::array<Point, 4> p{};
  for (std::size_t i = 0; i < 4; ++i) {
    Point local{origin[0] + offsets[i][0], origin[1] + offsets[i][1], origin[2] + offsets[i][2]};
    out.generatedPoints[i] = local;
    if (!finite(out.generatedPoints[i])) return std::unexpected("generated billboard point is not finite");
    p[i] = apply(in.projectionTransform, out.generatedPoints[i]);
    if (!finite(p[i])) return std::unexpected("projected billboard point is not finite");
  }

  const std::array<double, 4> uv = node.type2.branchDoubles;
  // Native slot XY uses crossed Y sources and shared P0/P1 reciprocals.
  const std::array<double, 4> sx = {p[0][0]/p[0][2], p[1][0]/p[1][2], p[2][0]/p[1][2], p[3][0]/p[0][2]};
  const std::array<double, 4> sy = {p[1][1]/p[1][2], p[0][1]/p[0][2], p[3][1]/p[0][2], p[2][1]/p[1][2]};
  const std::array<std::array<double, 2>, 4> slotUv{{{uv[0],uv[3]}, {uv[0],uv[2]}, {uv[1],uv[2]}, {uv[1],uv[3]}}};
  const std::array<std::size_t, 4> ownDepth{0,1,2,3};
  for (std::size_t i = 0; i < 4; ++i) {
    // Every divisor used by native X/Y must be finite and nonzero; negative
    // finite depths are retained without a guessed near-plane rule.
    const std::size_t xd = i == 0 || i == 3 ? 0 : 1;
    const std::size_t yd = i == 0 || i == 3 ? 1 : 0;
    if (p[xd][2] == 0 || p[yd][2] == 0) return std::unexpected("billboard projection divisor is zero");
    out.slots[i] = {in.center[0] + sx[i], in.center[1] - sy[i], p[ownDepth[i]][2], slotUv[i][0], slotUv[i][1]};
    const auto& v = out.slots[i];
    if (!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z))
      return std::unexpected("billboard slot is not finite");
  }
  return out;
}

}  // namespace webo::app
