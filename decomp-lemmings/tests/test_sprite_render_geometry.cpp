#include "sprite_render_geometry_fixture.hpp"
#include <iostream>
#include <cstdlib>
namespace {void require(bool x,const char* m){if(!x){std::cerr<<m<<'\n';std::exit(1);}}}
int main() {
    using namespace sprite_geometry_test;
    Fixture f(0);RuntimeSpritePiece piece{};piece.values_30={2,4,6,8};piece.double_bits_10={1,2,3,4};piece.texture_id_40=17;
    std::memcpy(f.pieces[0][0][0].opaque_00.data(),&piece,sizeof(piece));f.arguments={100,200,20,40};f.mode=f.reference;f.run();
    auto& v=f.model.vertices_08;
    require(v[0].x_18==89&&v[1].x_18==89&&v[2].x_18==109&&v[3].x_18==109,"rectangle horizontal coordinates");
    require(v[0].y_20==172&&v[3].y_20==172&&v[1].y_20==212&&v[2].y_20==212,"rectangle vertical coordinates");
    require(v[0].coordinate_bits_28==4&&v[0].coordinate_bits_30==2&&v[2].coordinate_bits_28==3&&v[2].coordinate_bits_30==1,"coordinate endpoint mapping");
    require(f.model.texture_id_1a8==17&&f.calls==1,"bind texture before one submit");
    f.model.swap_endpoints_1b0=-1;f.mode=f.reference-1;f.run();
    require(v[0].coordinate_bits_30==1&&v[1].coordinate_bits_30==2&&v[0].x_18==94.5&&v[2].x_18==104.5,"half scale and endpoint swap");
    piece.values_30={0x80000001u,0,0,0};std::memcpy(f.pieces[0][0][0].opaque_00.data(),&piece,sizeof(piece));
    f.mode=f.reference+7;f.arguments={0,0,0,0};f.run();require(v[0].x_18==2,"double offset wraps in 32bits before signed conversion");
    std::cout<<"sprite rectangle coordinates, scale, endpoint swap and wrapped offsets passed\n";
}
