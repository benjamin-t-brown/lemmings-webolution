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

bool latchSpriteFirstDrawFrame(SpriteTemporalState& state, std::uint32_t temporalCount,
                               double unitRandom) {
  if (state.firstDrawDone) return false;
  // ftol (0x0049C5AC) truncates; count >= 1 is an admission invariant, 0 degrades to index 0.
  const double scaled = static_cast<double>(temporalCount > 0 ? temporalCount - 1 : 0) * unitRandom;
  state.temporal = static_cast<std::uint32_t>(scaled);
  state.firstDrawDone = true;
  return true;
}

void advanceSpriteTemporal(SpriteTemporalState& state, double dt, double rate,
                           std::uint32_t temporalCount) {
  constexpr double kThreshold = 0.06;       // 0x004B1940
  constexpr double kEpsilon = 9.99999993922529e-09;  // 0x004B1928
  if (state.mode != 1 || temporalCount == 0) return;
  state.finished = false;
  double acc = state.accumulator + dt * rate;
  if (!std::isfinite(acc)) return;
  while (acc > kThreshold) {
    state.temporal += 1;
    if (state.temporal == temporalCount) {
      state.temporal = state.loop ? 0 : temporalCount - 1;
      state.finished = true;
    }
    acc -= kThreshold;
    if (acc <= kEpsilon) acc = 0.0;
  }
  state.accumulator = acc;
}

std::expected<SceneSpriteQuad, std::string> makeSceneSpriteQuad(
    const SceneNode& node, const BillboardSourceInputs& in, const SpriteFrame& image, bool mirror) {
  if (node.type != 2) return std::unexpected("sprite quad requires a type-2 node");
  if (node.type2.branch == 0) return std::unexpected("sprite quad requires a nonzero type-2 branch");
  if (!finite(in.parentCamera) || !finite(in.sourceTransform) || !finite(in.projectionTransform) ||
      !std::isfinite(in.center[0]) || !std::isfinite(in.center[1]))
    return std::unexpected("sprite inputs must be finite");
  const double a = node.type2.leading[0];
  const double b = node.type2.leading[1];
  const double h = node.type2.leading[2];
  for (double v : node.matrix) if (!std::isfinite(v)) return std::unexpected("node matrix must be finite");
  if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(h))
    return std::unexpected("sprite extents must be finite");

  const BillboardAffine3x4 nodeMatrix = fromNativeSceneMatrix(node.matrix);
  const auto toSource = compose(in.sourceTransform, compose(in.parentCamera, nodeMatrix));
  const Point origin = apply(toSource, {0, 0, 0});
  if (!finite(origin)) return std::unexpected("transformed sprite center is not finite");

  // 0x0047D158..0x0047D35F corners, then 0x0047D40B..0x0047D477: +H/2 on y of all four.
  const std::array<Point, 4> offsets{{{-a/2, -h/2, -a/2}, {-b/2, h/2, -b/2},
                                      { b/2, h/2, -b/2}, { a/2, -h/2, -a/2}}};
  std::array<Point, 4> p{};
  for (std::size_t i = 0; i < 4; ++i) {
    Point local{origin[0] + offsets[i][0], (origin[1] + offsets[i][1]) + h / 2, origin[2] + offsets[i][2]};
    if (!finite(local)) return std::unexpected("generated sprite point is not finite");
    p[i] = apply(in.projectionTransform, local);
    if (!finite(p[i])) return std::unexpected("projected sprite point is not finite");
  }
  if (p[0][2] == 0 || p[1][2] == 0) return std::unexpected("sprite projection divisor is zero");

  // 0x0047D4FB..0x0047D613 screen slots (shared with branch 0): x slots use p0.z for corners 0
  // and 3 and p1.z for 1 and 2; y slots use crossed sources.
  const std::array<double, 4> sx = {in.center[0] + p[0][0]/p[0][2], in.center[0] + p[1][0]/p[1][2],
                                    in.center[0] + p[2][0]/p[1][2], in.center[0] + p[3][0]/p[0][2]};
  const std::array<double, 4> sy = {in.center[1] - p[1][1]/p[1][2], in.center[1] - p[0][1]/p[0][2],
                                    in.center[1] - p[3][1]/p[0][2], in.center[1] - p[2][1]/p[1][2]};

  // 0x0047D771..0x0047D81B: arguments of 0x0047A565 in callee order [ebp+8..0x28].
  SceneSpriteQuad out;
  out.sourceOffset = node.sourceOffset;
  out.sourceName = node.name;
  out.sheetName = image.sheet;
  out.sourceVisibility = node.type2.visibility;
  out.blendSelector = node.type2.trailingWord;
  out.mirror = mirror;
  double a0 = (sx[0] + sx[1] + sx[2] + sx[3]) / 4.0;
  double a1 = sy[1];
  const double a2 = sx[2] - sx[1];
  const double a3 = sx[3] - sx[0];
  const double a4 = sy[1] - sy[0];
  out.args = {a0, a1, a2, a3, a4};

  // 0x0047A565: image ints X+0x30/0x34/0x38/0x3C are SpriteFrame unknown0/unknown1/width/height.
  a0 += static_cast<double>(static_cast<std::int32_t>(image.unknown0));
  a1 -= static_cast<double>(static_cast<std::int32_t>(image.unknown1));
  const double w = static_cast<double>(static_cast<std::int32_t>(image.width));
  const double hh = static_cast<double>(static_cast<std::int32_t>(image.height));
  double y0 = (a1 - hh / 2.0) - a4 / 2.0;
  y0 += hh / 4.0;  // [0x0058D53C] == 0 on the single-view route
  const double xs[4] = {a0 - a2 / 2.0 - w / 2.0, a0 - a3 / 2.0 - w / 2.0,
                        a0 + a3 / 2.0 - w / 2.0, a0 + a2 / 2.0 - w / 2.0};
  const double ys[4] = {y0, y0 + a4, y0 + a4, y0};
  // z slots stored before the call (0x0047D6F9..0x0047D76B): p1.z, p0.z, p3.z, p2.z.
  const double zs[4] = {p[1][2], p[0][2], p[3][2], p[2][2]};

  // 0x00440C50(P0,P1,P2,P3) with S10,S18,S20,S28 = the image doubles at X+0x10..0x28
  // (SpriteFrame uBottom, uTop, uLeft, uRight, in that order).
  const double S10 = image.uBottom, S18 = image.uTop, S20 = image.uLeft, S28 = image.uRight;
  const double P0 = S28, P1 = mirror ? S10 : S18, P2 = S20, P3 = mirror ? S18 : S10;
  const std::array<std::array<double, 2>, 4> uv{{{P0, P1}, {P0, P3}, {P2, P3}, {P2, P1}}};
  for (std::size_t i = 0; i < 4; ++i) {
    out.vertices[i] = {xs[i], ys[i], zs[i], uv[i][0], uv[i][1]};
    if (!std::isfinite(xs[i]) || !std::isfinite(ys[i]) || !std::isfinite(zs[i]))
      return std::unexpected("sprite vertex is not finite");
  }
  return out;
}

}  // namespace webo::app
