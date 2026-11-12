#pragma once
#include "lemmings/sprite_piece_runtime.hpp"
#include "lemmings/sprite_resource_destruction.hpp"

namespace lemmings {
struct SpriteRenderVertex {
    std::array<std::byte,24> unknown_00;
    double x_18,y_20;
    std::uint64_t coordinate_bits_28,coordinate_bits_30;
    std::array<std::byte,48> unknown_38;
};
static_assert(sizeof(SpriteRenderVertex)==0x68);
static_assert(offsetof(SpriteRenderVertex,unknown_00)==0);
static_assert(offsetof(SpriteRenderVertex,x_18)==0x18);
static_assert(offsetof(SpriteRenderVertex,y_20)==0x20);
static_assert(offsetof(SpriteRenderVertex,coordinate_bits_28)==0x28);
static_assert(offsetof(SpriteRenderVertex,coordinate_bits_30)==0x30);
static_assert(offsetof(SpriteRenderVertex,unknown_38)==0x38);
struct SpriteRenderGeometry {
    std::array<std::byte,8> unknown_00;
    std::array<SpriteRenderVertex,4> vertices_08;
    std::int32_t texture_id_1a8;
    std::array<std::byte,4> unknown_1ac;
    std::int32_t swap_endpoints_1b0;
    std::array<std::byte,4> unknown_1b4;
    std::int32_t piece_index_1b8;
    std::array<std::byte,12> unknown_1bc;
    std::int32_t clip_index_1c8,frame_index_1cc;
    std::array<std::byte,40> unknown_1d0;
    SpriteFactoryResource* resource_1f8;
};
static_assert(offsetof(SpriteRenderGeometry,unknown_00)==0);
static_assert(offsetof(SpriteRenderGeometry,vertices_08)==0x08);
static_assert(offsetof(SpriteRenderGeometry,texture_id_1a8)==0x1a8);
static_assert(offsetof(SpriteRenderGeometry,unknown_1ac)==0x1ac);
static_assert(offsetof(SpriteRenderGeometry,swap_endpoints_1b0)==0x1b0);
static_assert(offsetof(SpriteRenderGeometry,unknown_1b4)==0x1b4);
static_assert(offsetof(SpriteRenderGeometry,piece_index_1b8)==0x1b8);
static_assert(offsetof(SpriteRenderGeometry,unknown_1bc)==0x1bc);
static_assert(offsetof(SpriteRenderGeometry,clip_index_1c8)==0x1c8);
static_assert(offsetof(SpriteRenderGeometry,frame_index_1cc)==0x1cc);
static_assert(offsetof(SpriteRenderGeometry,unknown_1d0)==0x1d0);
static_assert(offsetof(SpriteRenderGeometry,resource_1f8)==0x1f8);
struct SpriteRenderGeometryHooks {
    void* context;
    void (*submit_414920)(void*,SpriteRenderGeometry&);
};
// 440C50: raw copies preserve signaling NaNs, signed zeros and all payload bits.
void set_sprite_quad_coordinates(SpriteRenderGeometry&,std::uint64_t first_x,
                                 std::uint64_t first_y,std::uint64_t second_x,std::uint64_t second_y);
// 47A190 +47AB96/440C30. Explicit globals are original58D53C and58CB38.
// delta0 => normal, delta-1 => half, any other uint32 difference => double.
// Indices/storage must be valid. Arithmetic follows masked x87 extended
// intermediates with original binary64 stores; long double must offer x87
// precision for bitwise agreement. The final renderer is an explicit boundary.
// Numeric position NaNs are equivalent by classification; their payload/sign is
// not specified. Texture-coordinate bits are always copied exactly.
void build_sprite_rectangle(SpriteRenderGeometry&,double x,double y,double width,double height,
                            std::uint32_t mode_58d53c,std::uint32_t reference_58cb38,
                            const SpriteRenderGeometryHooks&);
}
