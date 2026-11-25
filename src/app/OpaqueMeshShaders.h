#pragma once

#include <string_view>

namespace webo::app {

// An indexed rectangle only schedules fragment invocations. At logical pixel
// centres the fragment shader applies the clone's inclusive triangle edges.
// Fixed subpixel snapping and top-left ownership would change those decisions.
inline constexpr std::string_view kGlReal = R"glsl(
#ifdef MESH_FP64
#define Real double
#define Real2 dvec2
#define Real3 dvec3
#define Real4 dvec4
double decode(uvec2 value) { return packDouble2x32(value); }
uvec2 encodeDepth(double value) { return unpackDouble2x32(value); }
#else
#define Real float
#define Real2 vec2
#define Real3 vec3
#define Real4 vec4
float decode(uvec2 value) { return uintBitsToFloat(value.x); }
uvec2 encodeDepth(float value) { return uvec2(floatBitsToUint(value), 0u); }
#endif
bool winsActorDepth(Real z, uint owner, uvec4 prior) {
  Real previousZ = decode(prior.xy);
  return !isinf(z) && !isnan(z) &&
      (prior.z == 0u || z < previousZ || (z == previousZ && owner <= prior.z));
}
#ifdef WEBGL2
#define IntegerBuffer usampler2D
uvec4 bufferWord(usampler2D words, int index) {
  // ES 3.0 has no buffer textures. The linear operands occupy 256-texel rows.
  return texelFetch(words, ivec2(index & 255, index >> 8), 0);
}
#else
#define IntegerBuffer usamplerBuffer
uvec4 bufferWord(usamplerBuffer words, int index) { return texelFetch(words, index); }
#endif
)glsl";

inline constexpr std::string_view kOpaqueMeshOperands = R"glsl(
uniform IntegerBuffer meshVertices;
uniform IntegerBuffer meshIndices;
uniform vec2 viewportSize;
#ifndef WEBGL2
const int triangleStride = 3;
#else
const int triangleStride = 2;
#endif

Real2 operand(int vertex, int field) {
  uvec4 words = bufferWord(meshVertices, vertex * 5 + field);
  return Real2(decode(words.xy), decode(words.zw));
}
)glsl";

inline constexpr std::string_view kOpaqueMeshVertex = R"glsl(
uniform int firstTriangle;
#ifdef SCENE_CLIP_PROXY
uniform uvec4 depthInterval;
float cross2f(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }
float paddedEdge(vec2 edge, vec2 p, float direction) {
  return direction * cross2f(edge, p) + 2.0 * (abs(edge.x) + abs(edge.y));
}
#endif
flat out int triangleIndex;
flat out ivec3 corners;
void main() {
  triangleIndex = firstTriangle + gl_InstanceID;
  corners = ivec3(bufferWord(meshIndices, triangleIndex * triangleStride).xyz);
  vec2 a = vec2(operand(corners.x, 0));
  vec2 b = vec2(operand(corners.y, 0));
  vec2 c = vec2(operand(corners.z, 0));
  vec2 lower = clamp(floor(min(a, min(b, c))) - 1.0, vec2(0.0), viewportSize);
  vec2 upper = clamp(ceil(max(a, max(b, c))) + 1.0, vec2(0.0), viewportSize);
  vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1);
  vec2 p = mix(lower, upper, corner);
  float rasterDepth = 0.5;
#ifdef SCENE_CLIP_PROXY
  vec2 ab = b - a, bc = c - b, ca = a - c;
  float winding = cross2f(ab, c - a);
  vec2 magnitude = max(abs(a), max(abs(b), abs(c)));
  // Only use the proxy when binary32 establishes the orientation reliably.
  // The padded edges admit the exact fragment test at any input sample offset.
  if (max(magnitude.x, magnitude.y) < 8192.0 &&
      abs(winding) > 0.000002 * (abs(ab.x * ca.y) + abs(ab.y * ca.x)) + 0.01) {
    float direction = sign(winding);
    gl_ClipDistance[0] = paddedEdge(ab, p - a, direction);
    gl_ClipDistance[1] = paddedEdge(bc, p - b, direction);
    gl_ClipDistance[2] = paddedEdge(ca, p - c, direction);
  } else {
    gl_ClipDistance[0] = gl_ClipDistance[1] = gl_ClipDistance[2] = 1.0;
  }
  Real nearest = min(operand(corners.x, 1).x, min(operand(corners.y, 1).x, operand(corners.z, 1).x));
#ifdef SCENE_PROJECTED_PLANES
  Real normalized = (nearest - decode(depthInterval.xy)) * decode(depthInterval.zw);
#else
  Real normalized = (nearest - decode(depthInterval.xy)) / decode(depthInterval.zw) * Real(0.5);
#endif
  rasterDepth = clamp(float(normalized) - 0.00001, 0.0, 0.5);
#endif
  vec2 xy = p / viewportSize;
  gl_Position = vec4(2.0 * xy.x - 1.0, 1.0 - 2.0 * xy.y, 2.0 * rasterDepth - 1.0, 1.0);
}
)glsl";

inline constexpr std::string_view kOpaqueMeshFragment = R"glsl(
flat in int triangleIndex;
flat in ivec3 corners;
#ifdef SCENE_CLIP_PROXY
layout(depth_greater) out float gl_FragDepth;
#endif
uniform sampler2D surfaceImage;
uniform sampler2D destinationImage;
uniform usampler2D effectSceneDepth;
uniform usampler2D previousActor;
uniform int effectMode; // scene (0), mark (1), particle (2)
uniform uint particleOwner;
uniform ivec4 sampling; // point, wrap, tube, gouraud
uniform ivec2 alphaState; // texture alpha only, reference (-1 disables)
uniform ivec2 blendFactors; // disabled (-1), one (0), source alpha (1), inverse source alpha (2)
uniform uint firstOwner;
uniform uvec4 depthInterval;
uniform vec2 logicalSampleOffset;
layout(location = 0) out vec4 colour;
#ifdef WEBGL2
// ES 3.0 guarantees four draw buffers. Scene owner diagnostics share the
// otherwise unused fourth word of encodedDepth. Highlight stays independent.
layout(location = 1) out uvec2 highlight;
#else
layout(location = 1) out uint owner;
layout(location = 4) out uvec2 highlight;
#endif
layout(location = 2) out uvec4 encodedDepth;
layout(location = 3) out uvec4 acceptedPipeDepth;
uniform usampler2D previousHighlight;
uniform uvec4 highlightState; // packed owner, unlit sample, enabled

Real cross2(Real2 a, Real2 b) {
  MESH_PRECISE Real value = a.x * b.y - a.y * b.x;
  return value;
}

#ifdef SCENE_PROJECTED_PLANES
#define Shade float
#define Shade2 vec2
#define Shade3 vec3
#define Shade4 vec4
#else
#define Shade Real
#define Shade2 Real2
#define Shade3 Real3
#define Shade4 Real4
#endif

Shade4 diffuseAt(int corner) {
  int vertex = corners[corner];
  return Shade4(operand(vertex, 3), operand(vertex, 4));
}

int wrapTap(int tap, int size) {
  return tap < 0 ? size - 1 - ((-1 - tap) % size) : tap % size;
}

ivec2 address(ivec2 tap, ivec2 size) {
  if (sampling.y == 0) return clamp(tap, ivec2(0), size - 1);
  // Signed GLSL integer remainder is undefined for a negative operand.
  return ivec2(wrapTap(tap.x, size.x), wrapTap(tap.y, size.y));
}

Shade4 texel(ivec2 tap, ivec2 size) {
  // Reconstruct decoded bytes so the byte-domain filter has one final round.
  return Shade4(floor(texelFetch(surfaceImage, address(tap, size), 0) * 255.0 + 0.5));
}

Shade4 sampleSurface(Real2 uv) {
  ivec2 size = textureSize(surfaceImage, 0);
  if (sampling.z != 0) {
    if (sampling.y != 0) uv -= floor(uv);
    else uv = clamp(uv, Real2(0.0), Real2(1.0));
  }
  if (sampling.x != 0) return texel(ivec2(floor(uv * Real2(size))), size);
  Real2 xy = uv * Real2(size) - Real2(0.5);
  Real2 origin = floor(xy);
  ivec2 tap = ivec2(origin);
  Shade2 frac = Shade2(xy - origin);
  MESH_PRECISE Shade4 result = Shade4(0.0);
  result += texel(tap, size) * ((Shade(1.0) - frac.x) * (Shade(1.0) - frac.y));
  result += texel(tap + ivec2(1, 0), size) * (frac.x * (Shade(1.0) - frac.y));
  result += texel(tap + ivec2(0, 1), size) * ((Shade(1.0) - frac.x) * frac.y);
  result += texel(tap + ivec2(1, 1), size) * (frac.x * frac.y);
  return result;
}

#ifdef SCENE_PROJECTED_PLANES
Real exactAlpha(Real2 uv, Real w0, Real w1, Real w2, Real qa, Real qb, Real qc, Real q) {
  ivec2 size = textureSize(surfaceImage, 0);
  if (sampling.z != 0) {
    if (sampling.y != 0) uv -= floor(uv);
    else uv = clamp(uv, Real2(0.0), Real2(1.0));
  }
  MESH_PRECISE Real alpha;
  if (sampling.x != 0) {
    alpha = Real(texel(ivec2(floor(uv * Real2(size))), size).a);
  } else {
    Real2 xy = uv * Real2(size) - Real2(0.5);
    Real2 origin = floor(xy), f = xy - origin;
    ivec2 tap = ivec2(origin);
    alpha = Real(texel(tap, size).a) * ((Real(1.0) - f.x) * (Real(1.0) - f.y));
    alpha += Real(texel(tap + ivec2(1, 0), size).a) * (f.x * (Real(1.0) - f.y));
    alpha += Real(texel(tap + ivec2(0, 1), size).a) * ((Real(1.0) - f.x) * f.y);
    alpha += Real(texel(tap + ivec2(1, 1), size).a) * (f.x * f.y);
  }
  if (alphaState.x != 0) return alpha;
  MESH_PRECISE Real lightAlpha = operand(corners.x, 4).y;
  if (sampling.w != 0)
    lightAlpha = (w0 * lightAlpha * qa + w1 * operand(corners.y, 4).y * qb +
                  w2 * operand(corners.z, 4).y * qc) / q;
  return alpha * lightAlpha / (sampling.z != 0 ? Real(1.0) : Real(255.0));
}
#endif

Shade blendFactor(int factor, Shade alpha) {
  if (factor == 0) return Shade(1.0);
  return factor == 1 ? alpha : Shade(1.0) - alpha;
}

#ifdef SCENE_PROJECTED_PLANES
// The CPU determines whether the screen-space plane is well conditioned.
// Projected weights only decide coverage away from the inclusive edges.
uniform bool useProjectedWeights;

vec3 projectedWeights(Real2 a, Real2 point, vec4 slopes) {
  vec2 delta = vec2(point - a);
  MESH_PRECISE float w0 = 1.0 + dot(slopes.xy, delta);
  MESH_PRECISE float w1 = dot(slopes.zw, delta);
  return vec3(w0, w1, 1.0 - w0 - w1);
}
#endif

void main() {
  Real2 a = operand(corners.x, 0);
  Real2 point = Real2(gl_FragCoord.x, viewportSize.y - gl_FragCoord.y) +
                Real2(logicalSampleOffset - 0.5);
  Real2 za = operand(corners.x, 1);
  Real2 zb = operand(corners.y, 1);
  Real2 zc = operand(corners.z, 1);
  MESH_PRECISE Real2 zq;
  MESH_PRECISE Real w0, w1, w2;
#ifdef SCENE_PROJECTED_PLANES
  uvec4 indexWords = bufferWord(meshIndices, triangleIndex * triangleStride);
  float edgeGuard = useProjectedWeights && effectMode == 0 ? uintBitsToFloat(indexWords.w) : 0.0;
  vec4 slopes = uintBitsToFloat(bufferWord(meshIndices, triangleIndex * triangleStride + 1));
  vec3 weights = projectedWeights(a, point, slopes);
  float edgeWeight = min(weights.x, min(weights.y, weights.z));
  if (edgeGuard > 0.0 && edgeWeight < -edgeGuard) discard;
  bool projected = edgeGuard > 0.0 && edgeWeight > edgeGuard;
  if (projected) {
    w0 = Real(weights.x);
    w1 = Real(weights.y);
    w2 = Real(weights.z);
    uvec4 gradient = bufferWord(meshIndices, triangleIndex * triangleStride + 2);
    Real2 delta = point - a;
    zq.x = za.x + (decode(gradient.xy) * delta.x + decode(gradient.zw) * delta.y);
    zq.y = Real(dot(weights, vec3(za.y, zb.y, zc.y)));
  } else
#endif
  {
    Real2 b = operand(corners.y, 0);
    Real2 c = operand(corners.z, 0);
    Real area = cross2(b - a, c - a);
    if (abs(area) < Real(1.0e-12)) discard;
    w0 = cross2(b - point, c - point) / area;
    w1 = cross2(c - point, a - point) / area;
    w2 = Real(1.0) - w0 - w1;
    if (min(w0, min(w1, w2)) < Real(0.0)) discard;
    zq = w0 * za + w1 * zb + w2 * zc;
  }
#ifdef WEBGL2
  // GPU float affine interpolation uses local depth offsets.
  zq.x = za.x + (w1 * (zb.x - za.x) + w2 * (zc.x - za.x));
#endif
  if (!(zq.y > Real(0.0))) discard;
  ivec2 pixel = ivec2(gl_FragCoord.xy);
  if (effectMode != 0 && !(zq.x < decode(texelFetch(effectSceneDepth, pixel, 0).yz))) discard;
  if (effectMode == 2 &&
      !winsActorDepth(zq.x, particleOwner, texelFetch(previousActor, pixel, 0))) discard;
  Real2 uv;
  Shade4 diffuse = diffuseAt(0);
#ifdef SCENE_PROJECTED_PLANES
  if (projected) {
    vec3 weightsRhw = vec3(w0, w1, w2) * vec3(za.y, zb.y, zc.y) / float(zq.y);
    vec2 uva = vec2(operand(corners.x, 2));
    vec2 uvb = vec2(operand(corners.y, 2));
    vec2 uvc = vec2(operand(corners.z, 2));
    MESH_PRECISE vec2 coordinates = weightsRhw.x * uva + weightsRhw.y * uvb + weightsRhw.z * uvc;
    uv = Real2(coordinates);
    if (sampling.w != 0)
      diffuse = weightsRhw.x * diffuse + weightsRhw.y * diffuseAt(1) + weightsRhw.z * diffuseAt(2);
  } else
#endif
  {
    Real2 uva = operand(corners.x, 2);
    Real2 uvb = operand(corners.y, 2);
    Real2 uvc = operand(corners.z, 2);
    MESH_PRECISE Real2 interpolated = (w0 * uva * za.y + w1 * uvb * zb.y + w2 * uvc * zc.y) / zq.y;
    uv = interpolated;
    if (sampling.w != 0) {
      MESH_PRECISE Real4 gouraud =
          (w0 * Real4(diffuse) * za.y + w1 * Real4(diffuseAt(1)) * zb.y +
           w2 * Real4(diffuseAt(2)) * zc.y) / zq.y;
      diffuse = Shade4(gouraud);
    }
  }
  Shade4 sampleValue = sampleSurface(uv);
  Shade divisor = sampling.z != 0 ? Shade(1.0) : Shade(255.0);
  MESH_PRECISE Shade4 shaded = sampleValue * diffuse / divisor;
  if (alphaState.x != 0) shaded.a = sampleValue.a;
  if (alphaState.y >= 0) {
#ifdef SCENE_PROJECTED_PLANES
    // The strict cutout uses the original interpolation and filter in
    // binary64, including exactly-on-reference texels.
    if (!(exactAlpha(uv, w0, w1, w2, za.y, zb.y, zc.y, zq.y) > Real(alphaState.y))) discard;
#else
    if (!(shaded.a > Shade(alphaState.y))) discard;
#endif
  }
  Shade4 previous = Shade4(0.0);
  Shade sourceFactor = Shade(1.0), destinationFactor = Shade(0.0);
  Shade3 sourceHighlight = highlightState.y != 0u ? sampleValue.rgb : shaded.rgb;
  if (blendFactors.x >= 0) {
    // Each blended triangle reads a copy of the preceding destination bytes.
    // Round once, after the weighted sum, including on shared triangle edges.
    previous = Shade4(floor(texelFetch(destinationImage, pixel, 0) * 255.0 + 0.5));
    Shade alpha = shaded.a / Shade(255.0);
    sourceFactor = blendFactor(blendFactors.x, alpha);
    destinationFactor = blendFactor(blendFactors.y, alpha);
    MESH_PRECISE Shade4 result = shaded * sourceFactor + previous * destinationFactor;
    shaded = result;
  }
#ifndef WEBGL2
  if (blendFactors.x == -2 && blendFactors.y == 1)
    shaded.rgb *= shaded.a / Shade(255.0);
#endif
  shaded = floor(clamp(shaded, Shade4(0.0), Shade4(255.0)) + Shade4(0.5));
  colour = vec4(shaded / Shade(255.0));
  uint triangleOwner = firstOwner + uint(triangleIndex);
#ifdef WEBGL2
  colour.a = 1.0; // The software scene target is RGB.
#else
  owner = triangleOwner;
#endif
  highlight = uvec2(0u);
  if (effectMode == 2) {
    encodedDepth = uvec4(encodeDepth(zq.x), particleOwner, 5u);
    gl_FragDepth = gl_FragCoord.z;
    return;
  }
  if (highlightState.z != 0u) {
    if (destinationFactor == Shade(1.0) && all(equal(shaded.rgb, previous.rgb))) {
      highlight = texelFetch(previousHighlight, pixel, 0).rg;
    } else {
      uvec3 rgb = uvec3(floor(clamp(sourceHighlight * sourceFactor, Shade3(0.0), Shade3(255.0)) + Shade3(0.5)));
      highlight = uvec2(highlightState.x, rgb.r | (rgb.g << 8) | (rgb.b << 16));
    }
  }
#ifdef WEBGL2
  // Form the depth-plane coefficients before quantizing them to highp.
  // Evaluate around the first corner to avoid rounding an absolute view Z
  // to binary32 and then rounding it again in the comparison interval.
  uvec4 depthPlane = bufferWord(meshIndices, triangleIndex * 2 + 1);
  if (depthPlane.w == 0u) discard;
  vec3 mappedPlane = uintBitsToFloat(depthPlane.xyz);
  Real mapped = mappedPlane.x + dot(mappedPlane.yz, point - a);
#else
  Real minimum = decode(depthInterval.xy);
  Real extent = decode(depthInterval.zw);
#ifdef SCENE_PROJECTED_PLANES
  Real mapped = (zq.x - minimum) * extent;
#else
  Real mapped = (zq.x - minimum) / extent * Real(0.5);
#endif
#endif
  gl_FragDepth = float(clamp(mapped, Real(0.0), Real(0.5)));
  // R keeps the normalized comparison code, with a distinct out-of-range
  // diagnostic value. G/B retain the winning view Z for actors and input.
  encodedDepth = uvec4(floatBitsToUint(mapped >= Real(0.0) && mapped <= Real(0.5)
                                      ? gl_FragDepth : 2.0), encodeDepth(zq.x), 0u);
  acceptedPipeDepth = encodedDepth;
#ifdef WEBGL2
  encodedDepth.w = triangleOwner;
#endif
}
)glsl";

inline constexpr std::string_view kBlueSkyFragment = R"glsl(
uniform sampler2D sceneColour;
uniform usampler2D sceneDepth;
uniform ivec4 fillColour;
layout(location = 0) out vec4 colour;
void main() {
  ivec2 pixel = ivec2(gl_FragCoord.xy);
  colour = texelFetch(sceneColour, pixel, 0);
  bool untouched = texelFetch(sceneDepth, pixel, 0).r == floatBitsToUint(1.0);
  if (untouched && all(equal(colour.rgb, vec3(0.0))))
    colour.rgb = vec3(fillColour.rgb) / 255.0;
}
)glsl";

}  // namespace webo::app
