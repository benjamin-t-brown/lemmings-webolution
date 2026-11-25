#pragma once

#include <string_view>

namespace webo::app {

inline constexpr std::string_view kCompositorFragment = R"glsl(
uniform sampler2D sourceImage;
uniform sampler2D destinationImage;
uniform highp usampler2D sceneDepth;
uniform highp isampler2D iconTaps;
uniform highp usampler2D sceneHighlights;
uniform highp usampler2D highlightOwners;
uniform ivec4 destinationRect; // Logical origin and size before clipping
uniform ivec4 sourceRect;      // Top-down origin and size
uniform ivec4 state;           // Blend, sample, logical height, depth test
uniform ivec4 ink;             // RGB and constant alpha
uniform int captionScale;
uniform uvec2 labelDepth;
out vec4 colour;

// All additions, rounding and divisions take place in the byte domain.
ivec4 bytes(vec4 value) { return ivec4(floor(value * 255.0 + 0.5)); }

void main() {
  ivec2 pixel = ivec2(gl_FragCoord.xy);
  ivec2 logical = ivec2(pixel.x, state.z - 1 - pixel.y);
  ivec2 offset = logical - destinationRect.xy;
  int blend = state.x, sampleMode = state.y;
  if (state.w != 0) {
    uvec2 z = texelFetch(sceneDepth, pixel, 0).yz;
    // Labels carry positive view Z. Reject negative scene depth before
    // comparing the ordered IEEE words, including the positive clear sentinel.
    uint signWord = state.w == 2 ? z.y : z.x;
    if ((signWord & 0x80000000u) != 0u ||
        !(labelDepth.y < z.y || (labelDepth.y == z.y && labelDepth.x < z.x))) discard;
  }

  ivec4 src = ink;
  if (sampleMode == 1 || sampleMode == 2 || sampleMode == 3) {
    ivec2 size = textureSize(sourceImage, 0);
    ivec2 tap;
    if (sampleMode == 1) {
      tap = sourceRect.xy + offset * sourceRect.zw / destinationRect.zw;
    } else if (sampleMode == 2) {
      tap = ivec2(texelFetch(iconTaps, ivec2(offset.x, 0), 0).r,
                  texelFetch(iconTaps, ivec2(offset.y, 0), 0).g);
    } else {
      tap = ivec2(logical.x, pixel.y);
    }
    if (any(lessThan(tap, ivec2(0))) || any(greaterThanEqual(tap, size))) discard;
    src = bytes(texelFetch(sourceImage, tap, 0));
  } else if (sampleMode == 4) {
    int numerator = offset.x * sourceRect.z;
    int width = destinationRect.z;
    ivec2 tap = ivec2(numerator / width, offset.y);
    int rem = numerator % width;
    ivec4 a = bytes(texelFetch(sourceImage, tap, 0));
    tap.x = min(tap.x + 1, sourceRect.z - 1);
    ivec4 b = bytes(texelFetch(sourceImage, tap, 0));
    src = (a * (width - rem) + b * rem) / width;
  } else if (sampleMode == 5) {
    uvec2 mark = texelFetch(sceneHighlights, pixel, 0).rg;
    if (mark.x == 0u) discard;
    bool selected = false;
    for (int i = 0; i < textureSize(highlightOwners, 0).x; ++i)
      selected = selected || texelFetch(highlightOwners, ivec2(i, 0), 0).r == mark.x;
    if (!selected) discard;
    src.rgb = ivec3(mark.y & 255u, (mark.y >> 8) & 255u, (mark.y >> 16) & 255u);
  }

  if (blend == 0) {
    colour = vec4(src) / 255.0;
    return;
  }
  ivec4 dst = bytes(texelFetch(destinationImage, pixel, 0));
  if (blend == 1) {
    dst.rgb = src.rgb;
  } else if (blend == 2 || blend == 3) {
    dst.rgb = (src.rgb * src.a + dst.rgb * (255 - src.a)) / 255;
    if (blend == 2) dst.a = max(dst.a, src.a);
  } else if (blend == 4 || blend == 5) {
    ivec3 rgb = blend == 4 ? src.rgb : ink.rgb;
    dst.rgb = (rgb * src.a + dst.rgb * (255 - src.a) + 127) / 255;
  } else if (blend == 6 || blend == 7) {
    ivec3 add = blend == 6 ? src.rgb : (src.rgb * src.a + 127) / 255;
    dst.rgb = min(ivec3(255), dst.rgb + add);
  } else {
    int scale = captionScale;
    dst.rgb = (src.rgb * src.a + dst.rgb * (scale - src.a) + scale / 2) / scale;
  }
  colour = vec4(dst) / 255.0;
}
)glsl";

}  // namespace webo::app
