#pragma once
#include "lemmings/sprite_render_submission.hpp"
namespace lemmings {
// 46D460 packs the original device vertex format 0x1C4, stride32.
struct SpriteDeviceVertex {
    float x_00,y_04,z_08,rhw_0c;
    std::uint32_t color_10;
    std::uint32_t unknown_14; // Preserved, never assigned by this routine.
    float u_18,v_1c;
};
static_assert(sizeof(SpriteDeviceVertex)==32);
static_assert(offsetof(SpriteDeviceVertex,x_00)==0x00);
static_assert(offsetof(SpriteDeviceVertex,y_04)==0x04);
static_assert(offsetof(SpriteDeviceVertex,z_08)==0x08);
static_assert(offsetof(SpriteDeviceVertex,rhw_0c)==0x0c);
static_assert(offsetof(SpriteDeviceVertex,color_10)==0x10);
static_assert(offsetof(SpriteDeviceVertex,unknown_14)==0x14);
static_assert(offsetof(SpriteDeviceVertex,u_18)==0x18);
static_assert(offsetof(SpriteDeviceVertex,v_1c)==0x1c);
struct SpriteDeviceVertexArray {
    std::int32_t count_00,capacity_04,growth_08;
    SpriteDeviceVertex* data_0c;
};
static_assert(offsetof(SpriteDeviceVertexArray,count_00)==0);
static_assert(offsetof(SpriteDeviceVertexArray,capacity_04)==4);
static_assert(offsetof(SpriteDeviceVertexArray,growth_08)==8);
static_assert(sizeof(void*)!=4||offsetof(SpriteDeviceVertexArray,data_0c)==0x0c);
struct SpriteDrawState {
    SpriteDeviceVertexArray vertices_58d540;
    std::uint32_t initialized_flags_58d550;
    double u_offset_58d2a0,v_offset_58d2a8;
};
struct SpriteDrawHooks {
    void* context;
    SpriteDeviceVertex* (*allocate_49c1a8)(void*,std::uint32_t original_bytes);
    void (*free_49c13d)(void*,SpriteDeviceVertex*);
    void (*register_cleanup_49cb29)(void*); // Original callback46D605.
    // Original device vtable+70, return value ignored; primitive6, format1C4,
    // flags0. All arguments are captured before entering this callback.
    void (*draw_primitive_70)(void*,void* device,std::uint32_t primitive,
        std::uint32_t format,SpriteDeviceVertex*,std::uint32_t count,std::uint32_t flags);
};
void initialize_sprite_device_vertices(SpriteDeviceVertexArray&); //402C00
void resize_sprite_device_vertices(SpriteDeviceVertexArray&,std::int32_t,const SpriteDrawHooks&); //46E030
void clear_sprite_device_vertices(SpriteDeviceVertexArray&,const SpriteDrawHooks&); //46D605 chain
//46D460; source attributes are read at vertex+10,+18,+20,+28,+30,+38,+40,
//+48,+50. No color clamping: __ftol low32 values are shifted and ORed directly.
// Normal-return, successful allocation, live arrays and terminating growth.
// Floating-point comparison assumes original masked round-to-nearest mode.
void draw_sprite_polygon(SpriteVertexPointerArray&,SpriteDeviceState&,SpriteDrawState&,const SpriteDrawHooks&);
}
