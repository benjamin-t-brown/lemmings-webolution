#include "lemmings/sprite_render_shapes.hpp"
#include <bit>
#include <cstring>
namespace lemmings {
namespace {
double rounded(long double d){volatile double v=static_cast<double>(d);return v;}
std::int32_t signed_word(std::uint32_t v){return std::bit_cast<std::int32_t>(v);}
RuntimeSpritePiece piece_at(const SpriteRenderGeometry& m){
    auto& clip=static_cast<SpriteDestructionClip*>(m.resource_1f8->clips_00.data_0c)[m.clip_index_1c8];
    auto& frame=static_cast<SpriteDestructionFrame*>(clip.frames_00.data_0c)[m.frame_index_1cc];
    RuntimeSpritePiece result;std::memcpy(&result,static_cast<SpriteDestructionPiece*>(frame.pieces_00.data_0c)[m.piece_index_1b8].opaque_00.data(),sizeof(result));return result;
}
void coordinates(SpriteRenderGeometry& m,const RuntimeSpritePiece& p,bool reverse_x){set_sprite_quad_coordinates(m,p.double_bits_10[reverse_x?2:3],p.double_bits_10[m.swap_endpoints_1b0==0?1:0],p.double_bits_10[reverse_x?3:2],p.double_bits_10[m.swap_endpoints_1b0==0?0:1]);}
void submit(SpriteRenderGeometry& m,const RuntimeSpritePiece& p,const SpriteRenderGeometryHooks& h){m.texture_id_1a8=p.texture_id_40;h.submit_414920(h.context,m);}
}
void set_sprite_filtering(bool enabled,std::uint8_t supported,SpriteDeviceState& d,const SpriteDeviceHooks& h){
    std::uint32_t mode=1;d.alpha_reference_4c24e0=239;
    if(enabled&&supported){mode=2;d.alpha_reference_4c24e0=160;}
    h.set_stage_state_a0(h.context,d.device_58cdb8,0,16,mode);
    h.set_stage_state_a0(h.context,d.device_58cdb8,0,17,mode);
}
void build_sprite_trapezoid(SpriteRenderGeometry& m,double x,double y,double width0,double width1,double height,std::uint32_t mode,const SpriteRenderGeometryHooks& h){
    const auto p=piece_at(m);coordinates(m,p,false);x=rounded(static_cast<long double>(signed_word(p.values_30[0]))+x);y=rounded(static_cast<long double>(y)-signed_word(p.values_30[1]));
    const long double correction=signed_word(p.values_30[2]),native_height=signed_word(p.values_30[3]);auto& v=m.vertices_08;
    v[0].x_18=rounded((static_cast<long double>(x)-static_cast<long double>(width0)/2)-correction/2);
    v[1].x_18=rounded((static_cast<long double>(x)-static_cast<long double>(width1)/2)-correction/2);
    v[2].x_18=rounded((static_cast<long double>(width1)/2+x)-correction/2);
    v[3].x_18=rounded((static_cast<long double>(width0)/2+x)-correction/2);
    v[0].y_20=rounded((static_cast<long double>(y)-native_height/2)-static_cast<long double>(height)/2);
    if(mode==0)v[0].y_20=rounded(native_height/4+v[0].y_20);
    v[3].y_20=v[0].y_20;v[2].y_20=rounded(static_cast<long double>(height)+v[0].y_20);v[1].y_20=v[2].y_20;submit(m,p,h);
}
void build_sprite_anchored_rectangle(SpriteRenderGeometry& m,double x,double y,SpriteShapeState& s,SpriteDeviceState& d,const SpriteShapeHooks& h){
    if(s.mode_58d53c==0)set_sprite_filtering(false,s.filtering_supported_58d121,d,h.device);
    const auto p=piece_at(m);const auto delta=s.mode_58d53c-s.reference_58cb38;
    y=rounded(static_cast<long double>(y)-signed_word(p.values_30[1]));double height=signed_word(p.values_30[3]);
    const bool flipped=m.unknown_1b4[0]!=std::byte{};coordinates(m,p,!flipped);auto& v=m.vertices_08;
    if(flipped){
        if(delta==0){x=rounded(static_cast<long double>(signed_word(p.values_30[0]))+x);v[1].x_18=rounded(static_cast<long double>(x)-signed_word(p.values_30[2]));}
        else if(delta==0xffffffffu){y=rounded(static_cast<long double>(signed_word(p.values_30[1]))/2+y);x=rounded(static_cast<long double>(signed_word(p.values_30[0]))/2+x);height=rounded(static_cast<long double>(height)/2);v[1].x_18=rounded(static_cast<long double>(x)-static_cast<long double>(signed_word(p.values_30[2]))/2);}
        else{y=rounded(static_cast<long double>(y)-signed_word(p.values_30[1]));x=rounded(static_cast<long double>(signed_word(p.values_30[0]<<1))+x);height=rounded(static_cast<long double>(height)+height);v[1].x_18=rounded(static_cast<long double>(x)-signed_word(p.values_30[2]<<1));}
        v[0].x_18=v[1].x_18;v[3].x_18=x;v[2].x_18=x;
    }else{
        if(delta==0){x=rounded(static_cast<long double>(x)-signed_word(p.values_30[0]));v[3].x_18=rounded(static_cast<long double>(signed_word(p.values_30[2]))+x);}
        else if(delta==0xffffffffu){x=rounded(static_cast<long double>(x)-static_cast<long double>(signed_word(p.values_30[0]))/2);height=rounded(static_cast<long double>(height)/2);v[3].x_18=rounded(static_cast<long double>(signed_word(p.values_30[2]))/2+x);}
        else{x=rounded(static_cast<long double>(x)-signed_word(p.values_30[0]<<1));height=rounded(static_cast<long double>(height)+height);v[3].x_18=rounded(static_cast<long double>(signed_word(p.values_30[2]<<1))+x);}
        v[2].x_18=v[3].x_18;v[1].x_18=x;v[0].x_18=x;
    }
    v[3].y_20=rounded(static_cast<long double>(y)-height);v[0].y_20=v[3].y_20;v[2].y_20=y;v[1].y_20=y;
    submit(m,p,h.geometry);set_sprite_filtering(true,s.filtering_supported_58d121,d,h.device);
}
}
