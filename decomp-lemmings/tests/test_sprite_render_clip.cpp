#include "sprite_render_clip_fixture.hpp"
#include <iostream>
#include <memory>
namespace{void require(bool v,const char* s){if(!v){std::cerr<<s<<'\n';std::exit(1);}}}
int main(){using namespace sprite_clip_test;SpriteRenderVertex a{},b{},out{};for(unsigned i=0;i<11;++i){put(a,i,0);put(b,i,10);}put(a,2,2);put(b,2,2);a.x_18=0;a.y_20=0;b.x_18=10;b.y_20=10;std::memset(reinterpret_cast<std::byte*>(&out)+88,0x5a,16);
    interpolate_sprite_clip_vertex(out,5,a,b,false);require(out.x_18==5&&out.y_20==5&&get(out,2)==2&&get(out,5)==5,"midpoint interpolation");for(unsigned i=88;i<104;++i)require(reinterpret_cast<unsigned char*>(&out)[i]==0x5a,"explicit indeterminate tail projection");
    auto inside=std::make_unique<Fixture>(1);inside->run();require(inside->draws==1&&inside->state.scratch_58d578.used_20==8,"inside polygon uses original pointers");
    auto outside=std::make_unique<Fixture>(2);outside->run();require(outside->draws==0&&outside->state.temporary_58d558.count_00==0,"maximum Y early exit");
    auto nan=std::make_unique<Fixture>(1);nan->vertices[0].y_20=NAN;auto h=nan->hooks();clip_sprite_plane(nan->input,nan->state.temporary_58d558,nan->state.scratch_58d578,8,SpriteClipPlane::maximum_y,h);require(nan->state.temporary_58d558.count_00==2,"maximum plane treats unordered as inside");
    clip_sprite_plane(nan->state.temporary_58d558,nan->input,nan->state.scratch_58d578,-8,SpriteClipPlane::minimum_y,h);require(nan->input.count_00==3,"minimum plane treats unordered as outside and generates intersections");
    std::cout<<"sprite clipping interpolation, early exits, NaN planes and unknown tail passed\n";
}
