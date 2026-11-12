#pragma once
#include "lemmings/sprite_render_submission.hpp"
namespace lemmings {
enum class SpriteClipPlane { maximum_y,minimum_y,maximum_x,minimum_x };
struct SpriteClipScratch {
    std::array<std::uint32_t,3> unknown_00;
    SpriteRenderVertex* vertices_0c;
    SpriteVertexPointerArray available_10;
    std::uint32_t used_20;
};
static_assert(offsetof(SpriteClipScratch,unknown_00)==0);
static_assert(sizeof(void*)!=4||offsetof(SpriteClipScratch,vertices_0c)==0x0c);
static_assert(sizeof(void*)!=4||offsetof(SpriteClipScratch,available_10)==0x10);
static_assert(sizeof(void*)!=4||offsetof(SpriteClipScratch,used_20)==0x20);
struct SpriteClipState {
    std::int32_t minimum_x_58d280,maximum_x_58d284,maximum_y_58d288,minimum_y_58d28c;
    SpriteVertexPointerArray temporary_58d558;
    SpriteClipScratch scratch_58d578;
};
struct SpriteClipHooks {
    void* context;
    SpriteRenderVertex** (*allocate_49c1a8)(void*,std::uint32_t original_bytes);
    void (*free_49c13d)(void*,SpriteRenderVertex**);
    void (*draw_46d460)(void*,SpriteVertexPointerArray&);
};
//46D910/46DB80: calculate the defined prefix through+57. Original copies16
// indeterminate stack bytes at+58..+67 as well. This projection preserves that
// explicit unknown destination tail; it makes no original-tail value claim.
void interpolate_sprite_clip_vertex(SpriteRenderVertex& destination,double plane,
    const SpriteRenderVertex& first,const SpriteRenderVertex& second,bool x_axis);
//46DF90: pop an available pointer or return contiguous pool[used++] without
// allocation or bounds checks. Pool capacity is a caller precondition.
SpriteRenderVertex* take_sprite_clip_vertex(SpriteClipScratch&);
//46DED0: append a live pointer slot, preserving reads after allocator callbacks.
void append_sprite_clip_pointer(SpriteVertexPointerArray&,SpriteRenderVertex* const&,
    const SpriteClipHooks&);
//46CCE2/46CE60/46CFDE/46D15C. Positive input count required. Input pointer storage
// must remain alive throughout callbacks. Output is distinct and cleared first.
void clip_sprite_plane(SpriteVertexPointerArray& input,SpriteVertexPointerArray& output,
    SpriteClipScratch&,double plane,SpriteClipPlane,const SpriteClipHooks&);
//46D3C1: maximumY, minimumY, maximumX, minimumX then draw; returns early after
// each empty result. Bounds are live signed integer globals converted to double.
// Normal-return, sufficient pool/array storage, successful allocations and
// masked x87 arithmetic are required. Empty initial input is outside the domain.
void clip_and_draw_sprite_polygon(SpriteVertexPointerArray&,SpriteClipState&,const SpriteClipHooks&);
}
