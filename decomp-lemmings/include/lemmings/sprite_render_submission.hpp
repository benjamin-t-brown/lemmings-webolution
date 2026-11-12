#pragma once
#include "lemmings/sprite_render_geometry.hpp"
#include "lemmings/resource_cache.hpp"

namespace lemmings {
// Live fields of the texture object. Identity is the original object pointer;
// references permit the device callbacks to change fields at original rereads.
struct SpriteTextureBindingView {
    void* identity;
    void*& device_texture_00;
    std::uint32_t& flags_24;
};
struct SpriteDeviceState {
    void* device_58cdb8;
    void* active_texture_5cd5f4;
    std::uint32_t texture_flags_4c24e4;
    std::uint32_t alpha_reference_4c24e0;
    std::int32_t blend_mode_4c227c;
};
// The state aggregate above projects separate original globals, not one object.
struct SpriteDeviceHooks {
    void* context;
    // Device methods use the original vtable slot byte offsets. HRESULTs are
    // ignored by these recovered routines. Device pointer is reread each call.
    void (*set_texture_98)(void*,void* device,std::uint32_t stage,void* texture);
    void (*set_stage_state_a0)(void*,void* device,std::uint32_t stage,std::uint32_t type,std::uint32_t value);
    void (*set_render_state_58)(void*,void* device,std::uint32_t type,std::uint32_t value);
};
void bind_sprite_texture(SpriteTextureBindingView,SpriteDeviceState&,const SpriteDeviceHooks&); //47261F
void set_sprite_blend_mode(std::int32_t,SpriteDeviceState&,const SpriteDeviceHooks&); //414D80

struct SpriteVertexPointerArray {
    std::int32_t count_00,capacity_04,growth_08;
    SpriteRenderVertex** data_0c;
};
static_assert(offsetof(SpriteVertexPointerArray,count_00)==0);
static_assert(offsetof(SpriteVertexPointerArray,capacity_04)==4);
static_assert(offsetof(SpriteVertexPointerArray,growth_08)==8);
static_assert(sizeof(void*)!=4||offsetof(SpriteVertexPointerArray,data_0c)==0x0c);
struct SpriteSubmissionState {
    SpriteDeviceState device;
    // 58D578 scratch allocator: only these two words are reset by 414B70.
    std::uint32_t scratch_count_58d588;
    std::uint32_t scratch_used_58d598;
    std::uint32_t initialized_flags_5d6160;
    SpriteVertexPointerArray vertices_5d6170;
};
struct SpriteSubmissionHooks {
    void* context;
    // Pure host projection; no original side effect corresponds to this step.
    SpriteTextureBindingView (*texture_view)(void*,void* resource);
    SpriteDeviceHooks device;
    // Array allocation is plain, without cookie. original_bytes uses x86 stride4;
    // provide aligned host pointer storage for the same element count.
    SpriteRenderVertex** (*allocate_pointers)(void*,std::uint32_t original_bytes);
    void (*free_pointers)(void*,SpriteRenderVertex**);
    // Records CRT atexit registration for original callback4150A0; caller may
    // arrange cleanup with clear_sprite_submission_vertices at shutdown.
    void (*register_cleanup_49cb29)(void*);
    void (*clip_and_draw_46d3c1)(void*,SpriteVertexPointerArray&);
};
void initialize_sprite_submission_vertices(SpriteVertexPointerArray&); //414F10/414F30
void resize_sprite_submission_vertices(SpriteVertexPointerArray&,std::int32_t,const SpriteSubmissionHooks&); //414A60
void clear_sprite_submission_vertices(SpriteVertexPointerArray&,const SpriteSubmissionHooks&); //4150A0 chain
//414A10/414B90/414D20 texture specialization of the existing balanced cache API.
void* get_sprite_texture_balanced(ResourceCache&,std::int32_t,const ResourceCacheHooks&);
//414920: cache lookup, texture bind, blend state, scratch reset, lazy pointer
// array initialization, then clipping input order0,3,2,1. All hooks are required.
// Valid live cache/array storage, terminating growth and successful allocation
// are required. Callbacks may mutate state if subsequent accesses remain valid.
void submit_sprite_geometry(SpriteRenderGeometry&,SpriteSubmissionState&,ResourceCache&,
                            const ResourceCacheHooks&,const SpriteSubmissionHooks&);
}
