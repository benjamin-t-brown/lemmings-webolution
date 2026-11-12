#include "sprite_render_submission_fixture.hpp"
#include <iostream>
namespace {void require(bool b,const char* m){if(!b){std::cerr<<m<<'\n';std::exit(1);}}}
int main(){using namespace sprite_submission_test;
    Fixture f(0);f.run();require(f.trace.front()[0]==1&&f.trace.back()[0]==7,"device binding precedes submission");
    require(f.state.vertices_5d6170.data_0c[0]==&f.model.vertices_08[0]&&f.state.vertices_5d6170.data_0c[1]==&f.model.vertices_08[3]&&f.state.vertices_5d6170.data_0c[2]==&f.model.vertices_08[2]&&f.state.vertices_5d6170.data_0c[3]==&f.model.vertices_08[1],"quad pointer order");
    require(f.node.references_08==7&&f.state.initialized_flags_5d6160==0xa5&&f.state.scratch_count_58d588==0&&f.state.scratch_used_58d598==0,"balanced references, flags and scratch reset");
    Fixture cached(32);cached.run();for(auto e:cached.trace)require(e[0]!=1&&e[0]!=2,"identity cache skips texture and stages");
    for(unsigned flags=0;flags<32;++flags){Fixture b(flags);bind_sprite_texture({&b.texture,b.texture.device,b.texture.flags},b.state.device,b.hooks().device);require(b.trace.front()[0]==1,"bind texture first");if(flags&8)require(b.trace.size()==7,"bit8 precedence");else if(flags&16)require(b.trace.size()==8,"bit16 sequence");else if(flags&3)require(b.trace.size()==9,"alpha test sequence");}
    Fixture clean(0);clear_sprite_submission_vertices(clean.state.vertices_5d6170,clean.hooks());require(clean.trace[0][8]==0&&clean.trace[0][7]==2&&clean.state.vertices_5d6170.count_00==0&&!clean.state.vertices_5d6170.data_0c,"clear callback order");
    clean.state.vertices_5d6170.capacity_04=9;clear_sprite_submission_vertices(clean.state.vertices_5d6170,clean.hooks());require(clean.state.vertices_5d6170.capacity_04==9,"null data retains capacity");
    std::cout<<"sprite submission state, texture priorities, quad order and cleanup passed\n";
}
