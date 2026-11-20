#pragma once

#include "assets/SceneNode.h"
#include "assets/Sprite.h"

#include <array>
#include <expected>
#include <string>

namespace webo::app {

// Row-major 3x4 affine transform: rows are x, y, z; column 3 is translation.
struct BillboardAffine3x4 {
  std::array<double, 12> values{};
};

struct BillboardSourceInputs {
  // Applied in this order: node matrix, parent/camera, source transform,
  // projection transform. The first three locate the source node; projection
  // transforms generated local billboard points.
  BillboardAffine3x4 parentCamera;
  BillboardAffine3x4 sourceTransform;
  BillboardAffine3x4 projectionTransform;
  std::array<double, 2> center{};
};

struct BillboardVertex {
  double x = 0, y = 0, z = 0;
  double u = 0, v = 0;
};

struct SceneBillboardSource {
  std::size_t sourceOffset = 0;
  std::string sourceName;
  std::string textureName;
  std::uint32_t blendSelector = 0;
  std::uint8_t sourceVisibility = 0;
  std::array<std::array<double, 3>, 4> generatedPoints{};
  // Native storage slots, before any screen clipping or raster work.
  std::array<BillboardVertex, 4> slots{};
  std::array<unsigned, 4> adapterOrder{0, 3, 2, 1};
};

// Produces float64 preclip geometry only. No visibility/cull/pass/state decision.
std::expected<SceneBillboardSource, std::string> makeSceneBillboardSource(
    const SceneNode& node, const BillboardSourceInputs& inputs);

// Per-node temporal state: sub-object +0x1C0 accumulator, +0x1CC temporal index, +0x1D0 mode
// (constructor 0x004680F0: 1), +0x200 finished, +0x201 loop (constructor: 1), node +0x350
// first-draw latch (cleared by the reader 0x0047B461).
struct SpriteTemporalState {
  std::uint32_t temporal = 0;
  double accumulator = 0.0;
  bool firstDrawDone = false;
  std::uint8_t mode = 1;
  bool loop = true;
  bool finished = false;
};

// First draw (0x0047D391..0x0047D3F8) latches ftol(double(count-1) * (rand()/32767.0)).
// CrtRand::unit() supplies unitRandom (0x004040A0). Returns true if this call sets the latch.
bool latchSpriteFirstDrawFrame(SpriteTemporalState& state, std::uint32_t temporalCount,
                               double unitRandom);

// Forward mode 1 at 0x00468833 uses per-node dt (0x004764C4) and double
// rate at Sprite+0x10. Threshold 0.06 (0x4B1940), epsilon 1e-8 (0x4B1928).
// A completed strip wraps only when looping is enabled.
void advanceSpriteTemporal(SpriteTemporalState& state, double dt, double rate,
                           std::uint32_t temporalCount);

// Branch-1 quad: the four sub-object vertices (sub+0x08, +0x70, +0xD8, +0x140) in native slot
// order with screen x/y/z and u/v, plus the 0x0047A565 argument doubles a0..a4 (before the
// image offsets are applied). The quad is a separate type from SceneBillboardSource.
struct SceneSpriteQuad {
  std::size_t sourceOffset = 0;
  std::string sourceName;
  std::string sheetName;     // image sheet as stored in the SPR, e.g. "Sheets\\_Fire0"
  std::uint32_t blendSelector = 0;
  std::uint8_t sourceVisibility = 0;
  bool mirror = false;
  std::array<double, 5> args{};  // a0..a4 as passed to 0x0047A565
  std::array<BillboardVertex, 4> vertices{};
  std::array<unsigned, 4> adapterOrder{0, 3, 2, 1};
};

// 0x0047D379..0x0047D81B and 0x0047A565 reuse branch-0 corners with +H/2
// on Y, then fill the projected rectangle from image offsets, size and UVs.
// [0x0058D53C] is treated as zero, selecting the +h/4 shift.
std::expected<SceneSpriteQuad, std::string> makeSceneSpriteQuad(
    const SceneNode& node, const BillboardSourceInputs& inputs, const SpriteFrame& image,
    bool mirror);

}  // namespace webo::app
