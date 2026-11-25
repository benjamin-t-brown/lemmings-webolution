#pragma once

#include <string_view>

namespace webo::app {

inline constexpr std::string_view kActorFragment = R"glsl(
uniform usampler2D atlas;
uniform usampler2D sceneDepth;
uniform usampler2D pipeDepth;
uniform usampler2D previousActor;
uniform usampler2D beforeFluid;
uniform usampler2D afterFluid;
uniform sampler2D destinationImage;
uniform usampler2D brickDepth;
uniform sampler2D brickColour;
uniform IntegerBuffer rings;
uniform usampler2D collision;
uniform ivec2 viewport;
uniform int ringCount;
uniform uvec4 cameraRay[8]; // cx, cy, focalX, focalY, eye and world-to-view basis
uniform uvec4 plane[2];    // numerator, constant, dx, dy
uniform uvec4 rectangle[2]; // left, top, right, bottom
uniform uvec4 atlasUv[2];
uniform ivec4 frameBox;
uniform ivec4 drawState; // mirror U, mirror V, has brick, draw mode
uniform uvec4 actorTint; // RGB and stable owner (one-based)
uniform uvec4 effectOperands; // origin Z, additive strength
uniform bool patchFluidDepth;
uniform usampler2D previousHighlight;
uniform bool trackHighlights;
layout(location = 0) out vec4 colour;
#ifdef WEBGL2
uniform usampler2D visibility;
uniform bool requireVisibility;
uniform vec3 footprintCentre; // anchor pixel x/y and its plane denominator
uniform vec3 footprintOrigin;
uniform vec3 footprintDx;
uniform vec3 footprintDy;
layout(location = 2) out uvec4 actor;
layout(location = 1) out uvec2 highlight;
#else
layout(location = 1) out uvec4 actor; // view Z words, owner + 1, fragment kind
layout(location = 2) out uvec2 highlight;
#endif

Real2 decodePair(uvec4 words) { return Real2(decode(words.xy), decode(words.zw)); }

// std::lerp uses the weighted sum when the endpoints straddle zero.
Real lerp(Real a, Real b, Real t) {
  MESH_PRECISE Real value = (a <= Real(0) && b >= Real(0)) || (a >= Real(0) && b <= Real(0))
      ? t * b + (Real(1) - t) * a : a + t * (b - a);
  return value;
}

Real3 ringVertex(int ring, int step, int outer) {
  int i = (ring * 64 + (step & 31) * 2 + outer) * 2;
  return Real3(decodePair(bufferWord(rings, i)), decode(bufferWord(rings, i + 1).xy));
}

Real2 ringEdge(int ring, int step, int outer, Real v) {
  Real2 a = ringVertex(ring, step, outer).xz;
  Real2 b = ringVertex(ring + 1, step, outer).xz;
  return Real2(lerp(a.x, b.x, v), lerp(a.y, b.y, v));
}

int cylinderSector(Real2 p) {
  // Find the winding sector used by atan2(-x,z) without a float atan in the
  // binary64 shader. Each search stays within a half turn, so the sign of
  // the cross product gives the ordering against its midpoint ray.
  int low = p.y < Real(0) || (p.y == Real(0) && p.x < Real(0)) ? 16 : 0;
  int high = low + 16;
  while (high - low > 1) {
    int mid = (low + high) / 2;
    Real2 ray = decodePair(bufferWord(rings, ringCount * 128 + mid));
    MESH_PRECISE Real cross = ray.x * p.y - ray.y * p.x;
    if (cross >= Real(0)) low = mid;
    else high = mid;
  }
  return low;
}

bool insideTerrain(Real2 centre, Real z) {
  if (ringCount < 2) return false;
#ifdef WEBGL2
  // Evaluate near the body instead of subtracting camera-sized operands in
  // binary32. The anchor and derivative numerators were formed in binary64.
  Real2 offset = centre - footprintCentre.xy;
  Real denominator = footprintCentre.z + dot(decodePair(plane[1]), offset);
  Real3 p = footprintOrigin + (footprintDx * offset.x + footprintDy * offset.y) / denominator;
#else
  Real2 lens = decodePair(cameraRay[0]);
  Real2 focal = decodePair(cameraRay[1]);
  Real3 eye = Real3(decodePair(cameraRay[2]), decode(cameraRay[3].xy));
  Real3 mx = Real3(decode(cameraRay[3].zw), decodePair(cameraRay[4]));
  Real3 my = Real3(decodePair(cameraRay[5]), decode(cameraRay[6].xy));
  Real3 mz = Real3(decode(cameraRay[6].zw), decodePair(cameraRay[7]));
  Real sx = (centre.x - lens.x) / focal.x;
  Real sy = (lens.y - centre.y) / focal.y;
  MESH_PRECISE Real3 ray = Real3(mx.x*sx + mx.y*sy + mx.z,
      my.x*sx + my.y*sy + my.z, mz.x*sx + mz.y*sy + mz.z);
  MESH_PRECISE Real3 p = eye + z * ray;
#endif
  if (!(p.y >= Real(0) && p.y < Real(textureSize(collision, 0).y))) return false;

  int ring = 0;
  while (ring + 1 < ringCount - 1 && ringVertex(ring + 1, 0, 0).y <= p.y) ++ring;
  Real bottom = ringVertex(ring, 0, 0).y;
  Real top = ringVertex(ring + 1, 0, 0).y;
  if (!(top > bottom && p.y >= bottom && p.y <= top)) return false;
  Real v = (p.y - bottom) / (top - bottom);
  int sector = cylinderSector(Real2(p.z, -p.x));
  Real2 a = ringEdge(ring, sector, 0, v);
  Real2 b = ringEdge(ring, sector + 1, 0, v);
  MESH_PRECISE Real det = a.x*b.y - a.y*b.x;
  if (abs(det) < Real(1.0e-12)) return false;
  MESH_PRECISE Real wa = (p.x*b.y - p.z*b.x) / det;
  MESH_PRECISE Real wb = (a.x*p.z - a.y*p.x) / det;
  if (!(wa + wb > Real(0))) return false;
  Real u = clamp(wb / (wa + wb), Real(0), Real(1));
  Real2 c = ringEdge(ring, sector, 1, v);
  Real2 d = ringEdge(ring, sector + 1, 1, v);
  Real2 inner = Real2(lerp(a.x, b.x, u), lerp(a.y, b.y, u));
  Real2 delta = Real2(lerp(c.x, d.x, u), lerp(c.y, d.y, u)) - inner;
  MESH_PRECISE Real lengthSquared = delta.x*delta.x + delta.y*delta.y;
  if (!(lengthSquared > Real(0))) return false;
  MESH_PRECISE Real layer = ((p.x-inner.x)*delta.x + (p.z-inner.y)*delta.y) / lengthSquared;
  int column = (sector * 8 + int(u * Real(8))) & 255;
  uvec4 cell = texelFetch(collision, ivec2(column, int(p.y)), 0);
  Real outer = decode(cell.yz);
  return outer > Real(0) && layer >= Real(0) && layer <= outer && (cell.x & 0x401u) != 0u;
}

uvec4 sampleBody(ivec2 p) {
  Real2 lo = decodePair(rectangle[0]), hi = decodePair(rectangle[1]);
  Real2 pixel = Real2(p);
  if (any(lessThan(pixel, lo)) || any(greaterThanEqual(pixel, hi))) return uvec4(0u);
  MESH_PRECISE Real2 uv = (pixel - lo) / (hi - lo);
  if (drawState.x != 0) uv.x = Real(1) - uv.x;
  if (drawState.y != 0) uv.y = Real(1) - uv.y;
  Real2 uvLo = decodePair(atlasUv[0]), uvHi = decodePair(atlasUv[1]);
  MESH_PRECISE Real2 source = (uvLo + uv * (uvHi - uvLo)) * Real2(textureSize(atlas, 0));
  ivec2 tap = clamp(ivec2(floor(source)), frameBox.xy, frameBox.zw - 1);
  return texelFetch(atlas, tap, 0);
}

void main() {
#ifdef WEBGL2
  if (requireVisibility && texelFetch(visibility, ivec2(0), 0).r == 0u) discard;
#endif
  ivec2 texel = ivec2(gl_FragCoord.xy);
  ivec2 logical = ivec2(texel.x, viewport.y - 1 - texel.y);
  Real2 centre = Real2(logical) + Real2(0.5);
  // Draw modes supplied by SceneGpu::SpriteMode.
  int mode = drawState.w;
  bool bubble = mode == 2, originParticle = mode == 3, axe = mode == 4;
  bool mark = mode == 7 || mode == 8 || mode == 9 || mode == 10;
  bool drowning = mode == 1 || bubble || mode == 6 || mode == 8 || mode == 10;
  bool probe = mode == 6;
  Real z;
  if (bubble || originParticle) {
    z = decode(effectOperands.xy);
  } else {
    Real2 coeff = decodePair(plane[0]), slope = decodePair(plane[1]);
    MESH_PRECISE Real denom = coeff.y + slope.x * centre.x + slope.y * centre.y;
    if (!(denom > Real(0) && coeff.x > Real(0))) discard;
    z = coeff.x / denom;
  }
  if (isinf(z) || isnan(z)) discard;
  Real sceneZ = decode(texelFetch(sceneDepth, texel, 0).yz);
  // Water is transparent to these effects. Any later depth write restores
  // the completed scenery at that pixel, including gates and billboards.
  if (drowning && patchFluidDepth && sceneZ == decode(texelFetch(afterFluid, texel, 0).yz))
    sceneZ = decode(texelFetch(beforeFluid, texel, 0).yz);
  if (!(z < sceneZ)) discard;
  if (!bubble && !originParticle && !axe &&
      !(z < decode(texelFetch(pipeDepth, texel, 0).yz))) discard;

  uvec4 body = sampleBody(logical);
  uvec3 rgb;
  uint kind = mode == 0 ? 1u : mode == 1 ? 3u : bubble ? 4u : 5u;
  if (body.a > 239u && (bubble || originParticle || !insideTerrain(centre, z))) {
    rgb = (body.rgb * actorTint.rgb) / 255u;
  } else if (drawState.z != 0) {
    uvec4 brick = texelFetch(brickDepth, texel, 0);
    if (brick.x == floatBitsToUint(1.0)) discard;
    // The software clips the assembled silhouette with the body plane,
    // then submits a carried brick at its own depth to the actor layer.
    z = decode(brick.yz);
    if (isinf(z) || isnan(z)) discard;
    rgb = uvec3(floor(texelFetch(brickColour, texel, 0).rgb * 255.0 + 0.5));
    kind = 2u;
  } else {
    discard;
  }

  if (probe) {
    // The software tests whether the first body is visible to scenery
    // before applying actor depth. The GPU uses the same gate for bubbles.
    colour = vec4(0.0);
    actor = uvec4(1u);
    highlight = uvec2(0u);
    return;
  }
  uvec4 priorActor = texelFetch(previousActor, texel, 0);
  highlight = uvec2(0u);
  if (mark) {
    if (priorActor.z != actorTint.a || any(notEqual(priorActor.xy, encodeDepth(z)))) discard;
    if (mode == 9 || mode == 10) {
      colour = vec4(0.0);
      actor = uvec4(1u);
      return;
    }
    uvec3 prior = uvec3(floor(texelFetch(destinationImage, texel, 0).rgb * 255.0 + 0.5));
    rgb = min(uvec3(255u), prior + rgb);
  } else if (!winsActorDepth(z, actorTint.a, priorActor)) {
    discard;
  } else if (drowning) {
    uvec3 prior = uvec3(floor(texelFetch(destinationImage, texel, 0).rgb * 255.0 + 0.5));
    Real strength = decode(effectOperands.zw);
    MESH_PRECISE Real3 contribution = Real3(rgb) * strength;
    rgb = min(uvec3(255u), prior + uvec3(floor(contribution + Real3(0.5))));
    if (trackHighlights && all(equal(prior, rgb)))
      highlight = texelFetch(previousHighlight, texel, 0).rg;
  }
  colour = vec4(vec3(rgb) / 255.0, 1.0);
  actor = uvec4(encodeDepth(z), actorTint.a, kind);
}
)glsl";

inline constexpr std::string_view kHatchMaskFragment = R"glsl(
flat in ivec3 corners;
uniform usampler2D sceneDepth;
layout(location = 0) out uint visible;
void main() {
  Real2 a = operand(corners.x, 0);
  Real2 b = operand(corners.y, 0);
  Real2 c = operand(corners.z, 0);
  Real2 p = Real2(gl_FragCoord.x, viewportSize.y - gl_FragCoord.y);
  if (any(lessThan(p, floor(min(a, min(b, c))))) ||
      any(greaterThanEqual(p, ceil(max(a, max(b, c)))))) discard;
  MESH_PRECISE Real area = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
  if (abs(area) < Real(1.0e-9)) discard;
  MESH_PRECISE Real wb = ((p.x-a.x)*(c.y-a.y) - (p.y-a.y)*(c.x-a.x)) / area;
  MESH_PRECISE Real wc = ((b.x-a.x)*(p.y-a.y) - (b.y-a.y)*(p.x-a.x)) / area;
  MESH_PRECISE Real wa = Real(1) - wb - wc;
  if (min(wa, min(wb, wc)) < Real(0)) discard;
  MESH_PRECISE Real z = wa * operand(corners.x, 1).x +
                       wb * operand(corners.y, 1).x + wc * operand(corners.z, 1).x;
  if (abs(z - decode(texelFetch(sceneDepth, ivec2(gl_FragCoord.xy), 0).yz)) > Real(0.5)) discard;
  visible = 1u;
}
)glsl";

inline constexpr std::string_view kHatchReduceFragment = R"glsl(
uniform usampler2D sourceMask;
uniform ivec2 viewport;
uniform ivec2 range;
uniform int reduceRows;
layout(location = 0) out uvec4 extent;
void main() {
  extent = uvec4(0u);
  if (reduceRows == 0) {
    int row = int(gl_FragCoord.y);
    uint y = uint(viewport.y - 1 - row);
    for (int x = range.x; x < range.y; ++x) {
      if (texelFetch(sourceMask, ivec2(x, row), 0).r == 0u) continue;
      // Encode minima in reverse so componentwise maxima reduce all edges.
      extent = max(extent, uvec4(uint(viewport.x - x), uint(viewport.y) - y, uint(x+1), y+1u));
    }
  } else {
    for (int y = range.x; y < range.y; ++y)
      extent = max(extent, texelFetch(sourceMask, ivec2(0, y), 0));
  }
}
)glsl";

}  // namespace webo::app
