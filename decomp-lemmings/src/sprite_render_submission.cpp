#include "lemmings/sprite_render_submission.hpp"
#include <bit>
#include <cstring>

namespace lemmings {
namespace {
std::int32_t add(std::int32_t a,std::int32_t b) {
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(a)+static_cast<std::uint32_t>(b));
}
}
void bind_sprite_texture(SpriteTextureBindingView texture,SpriteDeviceState& state,const SpriteDeviceHooks& hooks) {
    if(state.active_texture_5cd5f4==texture.identity)return;
    state.active_texture_5cd5f4=texture.identity;
    hooks.set_texture_98(hooks.context,state.device_58cdb8,0,texture.device_texture_00);
    if(state.texture_flags_4c24e4==texture.flags_24)return;
    state.texture_flags_4c24e4=texture.flags_24;
    const auto render=[&](std::uint32_t type,std::uint32_t value){hooks.set_render_state_58(hooks.context,state.device_58cdb8,type,value);};
    const auto stage=[&](std::uint32_t type,std::uint32_t value){hooks.set_stage_state_a0(hooks.context,state.device_58cdb8,0,type,value);};
    if((texture.flags_24&8)!=0) {
        render(15,0);stage(4,2);stage(5,2);stage(1,4);stage(2,2);stage(3,0);
    } else if((texture.flags_24&16)!=0) {
        render(15,0);stage(4,4);stage(5,2);stage(6,0);stage(1,4);stage(2,2);stage(3,0);
    } else {
        stage(1,4);stage(2,2);stage(3,0);
        if((texture.flags_24&3)!=0) {
            stage(4,2);stage(5,2);render(15,1);render(24,state.alpha_reference_4c24e0);render(25,5);
        } else {
            render(15,0);
            if((texture.flags_24&4)!=0){stage(4,4);stage(5,2);stage(6,0);}
            else {stage(4,2);stage(5,2);}
        }
    }
}
void set_sprite_blend_mode(std::int32_t mode,SpriteDeviceState& state,const SpriteDeviceHooks& hooks) {
    if(state.blend_mode_4c227c==mode)return;
    state.blend_mode_4c227c=mode;
    const auto render=[&](std::uint32_t type,std::uint32_t value){hooks.set_render_state_58(hooks.context,state.device_58cdb8,type,value);};
    if(mode==0)render(27,0);
    else {
        render(27,1);
        if(mode==2){render(19,2);render(20,2);}
        else {render(19,5);render(20,mode==3?2:6);}
    }
}
void initialize_sprite_submission_vertices(SpriteVertexPointerArray& array) {
    array.count_00=0;array.capacity_04=0;array.growth_08=1;array.data_0c=nullptr;
}
void resize_sprite_submission_vertices(SpriteVertexPointerArray& array,std::int32_t count,const SpriteSubmissionHooks& hooks) {
    if(array.capacity_04<count) {
        if(array.growth_08==1)array.capacity_04=count;
        else do{array.capacity_04=add(array.capacity_04,array.growth_08);}while(array.capacity_04<=count);
        if(array.capacity_04<1) {
            if(array.data_0c)hooks.free_pointers(hooks.context,array.data_0c);
            array.data_0c=nullptr;
        } else {
            auto* data=hooks.allocate_pointers(hooks.context,static_cast<std::uint32_t>(array.capacity_04)*4);
            if(array.data_0c) {
                for(std::int32_t i=0;i<array.count_00;++i)data[i]=array.data_0c[i];
                hooks.free_pointers(hooks.context,array.data_0c);
            }
            array.data_0c=data;
        }
    }
    array.count_00=count;
}
void* get_sprite_texture_balanced(ResourceCache& cache,std::int32_t id,const ResourceCacheHooks& hooks) {
    return get_resource_balanced(cache,id,hooks);
}
void clear_sprite_submission_vertices(SpriteVertexPointerArray& array,const SpriteSubmissionHooks& hooks) {
    if(array.data_0c) {
        array.capacity_04=0;
        hooks.free_pointers(hooks.context,array.data_0c);
        array.data_0c=nullptr;
    }
    array.count_00=0;
}
void submit_sprite_geometry(SpriteRenderGeometry& model,SpriteSubmissionState& state,ResourceCache& textures,
                            const ResourceCacheHooks& cache_hooks,const SpriteSubmissionHooks& hooks) {
    void* resource=get_sprite_texture_balanced(textures,model.texture_id_1a8,cache_hooks);
    bind_sprite_texture(hooks.texture_view(hooks.context,resource),state.device,hooks.device);
    std::int32_t mode;std::memcpy(&mode,model.unknown_00.data(),4);
    set_sprite_blend_mode(mode,state.device,hooks.device);
    state.scratch_used_58d598=0;state.scratch_count_58d588=0;
    if((state.initialized_flags_5d6160&1)==0) {
        state.initialized_flags_5d6160|=1;
        initialize_sprite_submission_vertices(state.vertices_5d6170);
        hooks.register_cleanup_49cb29(hooks.context);
    }
    resize_sprite_submission_vertices(state.vertices_5d6170,4,hooks);
    state.vertices_5d6170.data_0c[0]=&model.vertices_08[0];
    state.vertices_5d6170.data_0c[1]=&model.vertices_08[3];
    state.vertices_5d6170.data_0c[2]=&model.vertices_08[2];
    state.vertices_5d6170.data_0c[3]=&model.vertices_08[1];
    hooks.clip_and_draw_46d3c1(hooks.context,state.vertices_5d6170);
}
}
