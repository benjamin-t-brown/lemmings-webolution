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

Real2 operand(int vertex, int field) {
  uvec4 words = bufferWord(meshVertices, vertex * 5 + field);
  return Real2(decode(words.xy), decode(words.zw));
}
)glsl";

inline constexpr std::string_view kOpaqueMeshVertex = R"glsl(
uniform int firstTriangle;
flat out int triangleIndex;
flat out ivec3 corners;
void main() {
  triangleIndex = firstTriangle + gl_InstanceID;
#ifdef WEBGL2
  corners = ivec3(bufferWord(meshIndices, triangleIndex * 2).xyz);
#else
  corners = ivec3(bufferWord(meshIndices, triangleIndex).xyz);
#endif
  vec2 a = vec2(operand(corners.x, 0));
  vec2 b = vec2(operand(corners.y, 0));
  vec2 c = vec2(operand(corners.z, 0));
  vec2 lower = clamp(floor(min(a, min(b, c))) - 1.0, vec2(0.0), viewportSize);
  vec2 upper = clamp(ceil(max(a, max(b, c))) + 1.0, vec2(0.0), viewportSize);
  vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1);
  vec2 xy = mix(lower, upper, corner) / viewportSize;
  gl_Position = vec4(2.0 * xy.x - 1.0, 1.0 - 2.0 * xy.y, 0.0, 1.0);
}
)glsl";

inline constexpr std::string_view kOpaqueMeshFragment = R"glsl(
flat in int triangleIndex;
flat in ivec3 corners;
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

Real4 diffuseAt(int corner) {
  int vertex = corners[corner];
  return Real4(operand(vertex, 3), operand(vertex, 4));
}

int wrapTap(int tap, int size) {
  return tap < 0 ? size - 1 - ((-1 - tap) % size) : tap % size;
}

ivec2 address(ivec2 tap, ivec2 size) {
  if (sampling.y == 0) return clamp(tap, ivec2(0), size - 1);
  // Signed GLSL integer remainder is undefined for a negative operand.
  return ivec2(wrapTap(tap.x, size.x), wrapTap(tap.y, size.y));
}

Real4 texel(ivec2 tap, ivec2 size) {
  // Reconstruct decoded bytes so the byte-domain filter has one final round.
  return Real4(floor(texelFetch(surfaceImage, address(tap, size), 0) * 255.0 + 0.5));
}

Real4 sampleSurface(Real2 uv) {
  ivec2 size = textureSize(surfaceImage, 0);
  if (sampling.z != 0) {
    if (sampling.y != 0) uv -= floor(uv);
    else uv = clamp(uv, Real2(0.0), Real2(1.0));
  }
  if (sampling.x != 0) return texel(ivec2(floor(uv * Real2(size))), size);
  Real2 xy = uv * Real2(size) - Real2(0.5);
  Real2 origin = floor(xy);
  ivec2 tap = ivec2(origin);
  Real2 frac = xy - origin;
  MESH_PRECISE Real4 result = Real4(0.0);
  result += texel(tap, size) * ((Real(1.0) - frac.x) * (Real(1.0) - frac.y));
  result += texel(tap + ivec2(1, 0), size) * (frac.x * (Real(1.0) - frac.y));
  result += texel(tap + ivec2(0, 1), size) * ((Real(1.0) - frac.x) * frac.y);
  result += texel(tap + ivec2(1, 1), size) * (frac.x * frac.y);
  return result;
}

Real blendFactor(int factor, Real alpha) {
  if (factor == 0) return Real(1.0);
  return factor == 1 ? alpha : Real(1.0) - alpha;
}

void main() {
  Real2 a = operand(corners.x, 0);
  Real2 b = operand(corners.y, 0);
  Real2 c = operand(corners.z, 0);
  Real2 point = Real2(gl_FragCoord.x, viewportSize.y - gl_FragCoord.y) +
                Real2(logicalSampleOffset - 0.5);
  Real area = cross2(b - a, c - a);
  if (abs(area) < Real(1.0e-12)) discard;
  MESH_PRECISE Real w0 = cross2(b - point, c - point) / area;
  MESH_PRECISE Real w1 = cross2(c - point, a - point) / area;
  MESH_PRECISE Real w2 = Real(1.0) - w0 - w1;
  if (min(w0, min(w1, w2)) < Real(0.0)) discard;

  Real2 za = operand(corners.x, 1);
  Real2 zb = operand(corners.y, 1);
  Real2 zc = operand(corners.z, 1);
  MESH_PRECISE Real2 zq = w0 * za + w1 * zb + w2 * zc;
#ifdef WEBGL2
  // Express affine Z as a depth plus two local offsets. Summing three
  // weighted absolute depths in binary32 can shift nearly coplanar surfaces.
  zq.x = za.x + (w1 * (zb.x - za.x) + w2 * (zc.x - za.x));
#endif
  if (!(zq.y > Real(0.0))) discard;
  ivec2 pixel = ivec2(gl_FragCoord.xy);
  if (effectMode != 0 && !(zq.x < decode(texelFetch(effectSceneDepth, pixel, 0).yz))) discard;
  if (effectMode == 2 &&
      !winsActorDepth(zq.x, particleOwner, texelFetch(previousActor, pixel, 0))) discard;
  Real2 uva = operand(corners.x, 2);
  Real2 uvb = operand(corners.y, 2);
  Real2 uvc = operand(corners.z, 2);
  MESH_PRECISE Real2 uv = (w0 * uva * za.y + w1 * uvb * zb.y + w2 * uvc * zc.y) / zq.y;
  Real4 diffuse = diffuseAt(0); // Flat is the original vertex-zero colour.
  if (sampling.w != 0) {
    MESH_PRECISE Real4 gouraud =
        (w0 * diffuse * za.y + w1 * diffuseAt(1) * zb.y + w2 * diffuseAt(2) * zc.y) / zq.y;
    diffuse = gouraud;
  }
  Real4 sampleValue = sampleSurface(uv);
  Real divisor = sampling.z != 0 ? Real(1.0) : Real(255.0);
  MESH_PRECISE Real4 shaded = sampleValue * diffuse / divisor;
  if (alphaState.x != 0) shaded.a = sampleValue.a;
  if (alphaState.y >= 0 && !(shaded.a > Real(alphaState.y))) discard;
  Real4 previous = Real4(0.0);
  Real sourceFactor = Real(1.0), destinationFactor = Real(0.0);
  Real3 sourceHighlight = highlightState.y != 0u ? sampleValue.rgb : shaded.rgb;
  if (blendFactors.x >= 0) {
    // Each blended triangle reads a copy of the preceding destination bytes.
    // Round once, after the weighted sum, including on shared triangle edges.
    previous = Real4(floor(texelFetch(destinationImage, pixel, 0) * 255.0 + 0.5));
    Real alpha = shaded.a / Real(255.0);
    sourceFactor = blendFactor(blendFactors.x, alpha);
    destinationFactor = blendFactor(blendFactors.y, alpha);
    MESH_PRECISE Real4 result = shaded * sourceFactor + previous * destinationFactor;
    shaded = result;
  }
  shaded = floor(clamp(shaded, Real4(0.0), Real4(255.0)) + Real4(0.5));
  colour = vec4(shaded / Real(255.0));
  uint triangleOwner = firstOwner + uint(triangleIndex);
#ifdef WEBGL2
  colour.a = 1.0; // The software scene target is RGB.
#else
  owner = triangleOwner;
#endif
  highlight = uvec2(0u);
  if (effectMode == 2) {
    encodedDepth = uvec4(encodeDepth(zq.x), particleOwner, 5u);
    return;
  }
  if (highlightState.z != 0u) {
    if (destinationFactor == Real(1.0) && all(equal(shaded.rgb, previous.rgb))) {
      highlight = texelFetch(previousHighlight, pixel, 0).rg;
    } else {
      uvec3 rgb = uvec3(floor(clamp(sourceHighlight * sourceFactor, Real3(0.0), Real3(255.0)) + Real3(0.5)));
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
  Real mapped = (zq.x - minimum) / extent * Real(0.5);
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
