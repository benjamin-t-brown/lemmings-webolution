#include "sprite_render_shapes_fixture.hpp"
#include <iostream>
namespace{void require(bool v,const char* s){if(!v){std::cerr<<s<<'\n';std::exit(1);}}}
int main(){using namespace sprite_shapes_test;Fixture f(4);RuntimeSpritePiece p{};p.values_30={2,4,6,8};p.double_bits_10={1,2,3,4};p.texture_id_40=17;std::memcpy(f.pieces[f.model.clip_index_1c8][f.model.frame_index_1cc][f.model.piece_index_1b8].opaque_00.data(),&p,sizeof(p));f.arguments={100,200,20,40};f.state.mode_58d53c=1;f.run(false);auto& v=f.model.vertices_08;require(v[0].x_18==89&&v[1].x_18==79&&v[2].x_18==119&&v[3].x_18==109,"trapezoid unequal widths");require(v[0].y_20==187&&v[1].y_20==197,"trapezoid height");
    f.state.mode_58d53c=0;f.run(false);require(v[0].y_20==189,"mode0 quarter-height offset");
    f.state.mode_58d53c=f.state.reference_58cb38=1;f.model.unknown_1b4[0]=std::byte{};f.run(true);require(v[0].x_18==98&&v[2].x_18==104&&v[0].y_20==188&&v[1].y_20==196,"anchored native rectangle");require(v[0].coordinate_bits_28==3,"zero orientation reverses coordinate X");require(f.events.back()[1]==17&&f.device.alpha_reference_4c24e0==160,"filter enable after submission");
    std::cout<<"sprite trapezoid, anchored orientation and filtering passed\n";
}
