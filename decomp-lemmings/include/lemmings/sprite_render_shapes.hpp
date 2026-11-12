#pragma once
#include "lemmings/sprite_render_submission.hpp"
namespace lemmings {
// Separate global/manager fields projected for portable source composition.
struct SpriteShapeState {std::uint32_t mode_58d53c,reference_58cb38;std::uint8_t filtering_supported_58d121;};
struct SpriteShapeHooks {SpriteRenderGeometryHooks geometry;SpriteDeviceHooks device;};
//470F29; manager+38 aliases global device58CDB8, manager+3A1 is byte58D121.
void set_sprite_filtering(bool enabled,std::uint8_t supported,SpriteDeviceState&,const SpriteDeviceHooks&);
//47A565. Width0 belongs to vertices0/3, width1 to1/2. Only mode==0 adds
// quarter of native piece height to top Y; mode does not scale this shape.
void build_sprite_trapezoid(SpriteRenderGeometry&,double x,double y,double width0,double width1,double height,
    std::uint32_t mode_58d53c,const SpriteRenderGeometryHooks&);
//47A75D. Orientation is model+1B4's first byte; remaining bytes stay unknown.
// Initial mode0 disables filtering before piece lookup; filtering is enabled
// after submission unconditionally. State and callbacks are live.
void build_sprite_anchored_rectangle(SpriteRenderGeometry&,double x,double y,
    SpriteShapeState&,SpriteDeviceState&,const SpriteShapeHooks&);
}
