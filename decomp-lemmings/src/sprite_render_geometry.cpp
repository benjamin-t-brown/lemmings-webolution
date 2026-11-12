#include "lemmings/sprite_render_geometry.hpp"
#include <bit>
#include <cstring>

namespace lemmings {
namespace {
double rounded(long double value){volatile double result=static_cast<double>(value);return result;}
std::int32_t signed_word(std::uint32_t value){return std::bit_cast<std::int32_t>(value);}
}
void set_sprite_quad_coordinates(SpriteRenderGeometry& model,std::uint64_t x0,std::uint64_t y0,
                                 std::uint64_t x1,std::uint64_t y1) {
    auto& v=model.vertices_08;
    v[1].coordinate_bits_28=x0;v[0].coordinate_bits_28=x0;
    v[3].coordinate_bits_28=x1;v[2].coordinate_bits_28=x1;
    v[3].coordinate_bits_30=y0;v[0].coordinate_bits_30=y0;
    v[2].coordinate_bits_30=y1;v[1].coordinate_bits_30=y1;
}
void build_sprite_rectangle(SpriteRenderGeometry& model,double x,double y,double width,double height,
                            std::uint32_t mode,std::uint32_t reference,const SpriteRenderGeometryHooks& hooks) {
    auto& clip=static_cast<SpriteDestructionClip*>(model.resource_1f8->clips_00.data_0c)[model.clip_index_1c8];
    auto& frame=static_cast<SpriteDestructionFrame*>(clip.frames_00.data_0c)[model.frame_index_1cc];
    const auto& storage=static_cast<SpriteDestructionPiece*>(frame.pieces_00.data_0c)[model.piece_index_1b8];
    RuntimeSpritePiece piece;std::memcpy(&piece,storage.opaque_00.data(),sizeof(piece));
    set_sprite_quad_coordinates(model,piece.double_bits_10[3],
        piece.double_bits_10[model.swap_endpoints_1b0==0?1:0],piece.double_bits_10[2],
        piece.double_bits_10[model.swap_endpoints_1b0==0?0:1]);
    const auto delta=mode-reference;
    const auto ox=signed_word(piece.values_30[0]),oy=signed_word(piece.values_30[1]);
    const auto correction_x=signed_word(piece.values_30[2]),correction_y=signed_word(piece.values_30[3]);
    long double half_scale,full_scale;
    if(delta==0) {
        x=rounded(static_cast<long double>(ox)+x);y=rounded(static_cast<long double>(y)-oy);
        half_scale=0.5L;full_scale=1;
    } else if(delta==0xffffffffu) {
        x=rounded(static_cast<long double>(ox)/2+x);y=rounded(static_cast<long double>(y)-static_cast<long double>(oy)/2);
        half_scale=0.25L;full_scale=0.5L;
    } else {
        x=rounded(static_cast<long double>(signed_word(piece.values_30[0]<<1))+x);
        y=rounded(static_cast<long double>(y)-signed_word(piece.values_30[1]<<1));
        half_scale=1;full_scale=2;
    }
    auto& v=model.vertices_08;
    v[1].x_18=rounded((static_cast<long double>(x)-static_cast<long double>(width)*half_scale)-static_cast<long double>(correction_x)*half_scale);
    v[0].x_18=v[1].x_18;
    v[3].x_18=rounded(static_cast<long double>(width)*full_scale+v[0].x_18);
    v[2].x_18=v[3].x_18;
    v[3].y_20=rounded((static_cast<long double>(y)-static_cast<long double>(correction_y)*half_scale)-static_cast<long double>(height)*half_scale);
    v[0].y_20=v[3].y_20;
    v[2].y_20=rounded(static_cast<long double>(height)*full_scale+v[0].y_20);
    v[1].y_20=v[2].y_20;
    model.texture_id_1a8=piece.texture_id_40;
    hooks.submit_414920(hooks.context,model);
}
}
