#include "sprite_render_draw_fixture.hpp"
#include <iostream>
namespace{void require(bool v,const char* s){if(!v){std::cerr<<s<<'\n';std::exit(1);}}}
int main(){using namespace sprite_draw_test;Fixture f(1);f.input.count_00=1;auto& v=f.vertices[0];
    Fixture::put(v,0x18,12.25);Fixture::put(v,0x20,-8.5);Fixture::put(v,0x10,2);Fixture::put(v,0x28,0.25);Fixture::put(v,0x30,0.75);
    Fixture::put(v,0x38,18.9);Fixture::put(v,0x40,52.2);Fixture::put(v,0x48,86.1);Fixture::put(v,0x50,120.9);f.state.u_offset_58d2a0=0.125;f.state.v_offset_58d2a8=-0.25;f.run();auto& out=f.state.vertices_58d540.data_0c[0];
    require(out.x_00==12.25f&&out.y_04==-8.5f&&out.z_08==2&&out.rhw_0c==0.125f,"position/rhw packing");require(out.color_10==0x78123456,"truncated ARGB packing");require(out.u_18==0.375f&&out.v_1c==0.5f&&out.unknown_14==0x5a5a5a5a,"UV offsets and untouched word");
    Fixture::put(v,0x48,-1);f.run();require(f.state.vertices_58d540.data_0c[0].color_10==0xffffffff,"colors are not clamped");
    Fixture empty(0);empty.run();require(empty.trace.back()[0]==4&&empty.trace.back()[1]==0,"zero count still submits");
    clear_sprite_device_vertices(f.state.vertices_58d540,f.hooks());require(!f.state.vertices_58d540.data_0c&&f.state.vertices_58d540.count_00==0,"packed array cleanup");
    std::cout<<"sprite device packing, ARGB truncation, UV offsets and empty draw passed\n";
}
