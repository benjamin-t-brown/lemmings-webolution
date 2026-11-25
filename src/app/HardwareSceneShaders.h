#pragma once

#include <string_view>

namespace webo::app {

inline constexpr std::string_view kHardwareMeshVertex = R"glsl(
layout(location = 0) in vec4 projected; // Screen X, screen Y, view Z, reciprocal W
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 diffuse;
uniform vec2 viewportScale;
uniform vec2 depthMap; // Lower view Z and scale to window depth
out vec2 uv;
out vec4 tint;
void main() {
  float w = 1.0 / projected.w;
  vec2 xy = projected.xy * viewportScale + vec2(-1.0, 1.0);
  float z = (projected.z - depthMap.x) * depthMap.y;
  gl_Position = vec4(xy * w, (2.0 * z - 1.0) * w, w);
  uv = texcoord;
  tint = diffuse;
}
)glsl";

inline constexpr std::string_view kHardwareMeshFragment = R"glsl(
uniform sampler2D surfaceImage;
uniform sampler2D occlusionDepth;
uniform int checkOcclusion;
uniform int selectTextureAlpha;
uniform float alphaReference;
in vec2 uv;
in vec4 tint;
layout(location = 0) out vec4 colour;
layout(location = 1) out float pipeDepth;
void main() {
  if (checkOcclusion != 0 &&
      gl_FragCoord.z >= texelFetch(occlusionDepth, ivec2(gl_FragCoord.xy), 0).r) discard;
  vec4 source = texture(surfaceImage, uv);
  colour = source * tint;
  if (selectTextureAlpha != 0) colour.a = source.a;
  if (alphaReference >= 0.0 && colour.a <= alphaReference) discard;
  pipeDepth = gl_FragCoord.z;
}
)glsl";

inline constexpr std::string_view kHardwareSpriteFragment = R"glsl(
uniform sampler2D atlas;
uniform sampler2D scenery;
uniform sampler2D pipes;
uniform sampler2D beforeFluid;
uniform sampler2D afterFluid;
uniform sampler2D brickDepth;
uniform sampler2D brickColour;
uniform sampler2D lattice;
uniform usampler2D collision;
uniform sampler2D visibility;
uniform ivec4 spriteState; // Mirror U, mirror V, has attachment, depth mode
uniform ivec4 frameBox;
uniform vec4 placement;
uniform vec4 atlasUv;
uniform vec4 plane;
uniform vec4 tintStrength;
uniform vec2 depthMap;
uniform float originDepth;
uniform int height;
uniform int ringCount;
uniform int patchFluid;
uniform int requireVisibility;
uniform vec3 eye;
uniform vec3 rayOrigin;
uniform vec3 rayDx;
uniform vec3 rayDy;
out vec4 colour;

vec3 ringVertex(int ring, int step, int outer) {
  return texelFetch(lattice, ivec2((step & 31) * 2 + outer, ring), 0).xyz;
}

bool insideTerrain(vec2 centre, float z) {
  if (ringCount < 2) return false;
  vec3 p = eye + z * (rayOrigin + centre.x * rayDx + centre.y * rayDy);
  if (!(p.y >= 0.0 && p.y < float(textureSize(collision, 0).y))) return false;
  int lo = 0, hi = ringCount - 1;
  while (hi - lo > 1) {
    int mid = (lo + hi) / 2;
    if (ringVertex(mid, 0, 0).y <= p.y) lo = mid;
    else hi = mid;
  }
  float bottom = ringVertex(lo, 0, 0).y, top = ringVertex(hi, 0, 0).y;
  if (!(top > bottom && p.y >= bottom && p.y <= top)) return false;
  float v = (p.y - bottom) / (top - bottom);
  float turns = atan(-p.x, p.z) / 6.28318530718;
  int sector = min(31, int(fract(turns) * 32.0));
  vec2 a = mix(ringVertex(lo, sector, 0).xz, ringVertex(hi, sector, 0).xz, v);
  vec2 b = mix(ringVertex(lo, sector + 1, 0).xz, ringVertex(hi, sector + 1, 0).xz, v);
  float det = a.x * b.y - a.y * b.x;
  if (abs(det) < 1.0e-12) return false;
  float wa = (p.x * b.y - p.z * b.x) / det;
  float wb = (a.x * p.z - a.y * p.x) / det;
  if (!(wa + wb > 0.0)) return false;
  float u = clamp(wb / (wa + wb), 0.0, 1.0);
  vec2 c = mix(ringVertex(lo, sector, 1).xz, ringVertex(hi, sector, 1).xz, v);
  vec2 d = mix(ringVertex(lo, sector + 1, 1).xz, ringVertex(hi, sector + 1, 1).xz, v);
  vec2 inner = mix(a, b, u), delta = mix(c, d, u) - inner;
  float lengthSquared = dot(delta, delta);
  if (!(lengthSquared > 0.0)) return false;
  float layer = dot(p.xz - inner, delta) / lengthSquared;
  int column = (sector * 8 + int(u * 8.0)) & 255;
  uvec2 cell = texelFetch(collision, ivec2(column, int(p.y)), 0).rg;
  float outer = uintBitsToFloat(cell.y);
  return outer > 0.0 && layer >= 0.0 && layer <= outer && (cell.x & 0x401u) != 0u;
}

vec4 sampleBody(vec2 logical) {
  if (any(lessThan(logical, placement.xy)) || any(greaterThanEqual(logical, placement.zw)))
    return vec4(0.0);
  vec2 uv = (logical - placement.xy) / (placement.zw - placement.xy);
  if (spriteState.x != 0) uv.x = 1.0 - uv.x;
  if (spriteState.y != 0) uv.y = 1.0 - uv.y;
  vec2 source = (atlasUv.xy + uv * (atlasUv.zw - atlasUv.xy)) * vec2(textureSize(atlas, 0));
  return texelFetch(atlas, clamp(ivec2(floor(source)), frameBox.xy, frameBox.zw - 1), 0);
}

void main() {
  ivec2 pixel = ivec2(gl_FragCoord.xy);
  vec2 logical = vec2(pixel.x, height - 1 - pixel.y);
  vec2 centre = logical + 0.5;
  vec4 body = sampleBody(logical);
  if (body.a <= 239.0 / 255.0 && spriteState.z == 0) discard;
  int mode = spriteState.w;
  if (requireVisibility != 0 && texelFetch(visibility, ivec2(0), 0).r == 0.0) discard;
  bool origin = mode == 2 || mode == 3;
  float denominator = plane.y + dot(plane.zw, centre);
  if (!origin && !(denominator > 0.0 && plane.x > 0.0)) discard;
  float z = origin ? originDepth : plane.x / denominator;
  float depth = (z - depthMap.x) * depthMap.y;
  float scene = texelFetch(scenery, pixel, 0).r;
  if (patchFluid != 0 && (mode == 1 || mode == 2 || mode == 5) &&
      scene == texelFetch(afterFluid, pixel, 0).r)
    scene = texelFetch(beforeFluid, pixel, 0).r;
  if (depth >= scene) discard;
  if (!origin && mode != 4 && depth >= texelFetch(pipes, pixel, 0).r) discard;
  if (body.a > 239.0 / 255.0 && (origin || !insideTerrain(centre, z))) {
    // The sprite's lighting is an integer multiply before additive effects.
    vec3 rgb = floor((body.rgb * 255.0 * tintStrength.rgb + 0.001) / 255.0);
    colour = vec4(floor(rgb * tintStrength.a + 0.5) / 255.0, 1.0);
  } else if (spriteState.z != 0) {
    depth = texelFetch(brickDepth, pixel, 0).r;
    if (depth >= 1.0) discard;
    colour = vec4(texelFetch(brickColour, pixel, 0).rgb, 1.0);
  } else {
    discard;
  }
  if (mode == 5) colour = vec4(1.0);
  gl_FragDepth = depth;
}
)glsl";

inline constexpr std::string_view kHardwareVisibilityFragment = R"glsl(
uniform sampler2D coverage;
uniform ivec4 bounds;
out float visible;
void main() {
  visible = 0.0;
  for (int y = bounds.y; y < bounds.w; ++y) {
    for (int x = bounds.x; x < bounds.z; ++x) {
      if (texelFetch(coverage, ivec2(x, y), 0).r == 0.0) continue;
      visible = 1.0;
      return;
    }
  }
}
)glsl";

inline constexpr std::string_view kHardwareSkyFragment = R"glsl(
uniform sampler2D sceneColour;
uniform sampler2D sceneDepth;
out vec4 colour;
void main() {
  ivec2 pixel = ivec2(gl_FragCoord.xy);
  colour = texelFetch(sceneColour, pixel, 0);
  if (texelFetch(sceneDepth, pixel, 0).r == 1.0 && all(equal(colour.rgb, vec3(0.0))))
    colour.rgb = vec3(4.0, 8.0, 24.0) / 255.0;
}
)glsl";

}  // namespace webo::app
