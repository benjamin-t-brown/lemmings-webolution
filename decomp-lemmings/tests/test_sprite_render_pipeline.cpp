#include "sprite_render_pipeline_fixture.hpp"
#include <iostream>
int main(){using namespace sprite_pipeline_test;unsigned draws=0;for(unsigned i=0;i<1000;++i){Fixture f(i);f.run();if(f.node.references_08!=7)return 1;for(const auto& e:f.trace)if(e[0]==9){if(e[2]!=6||e[3]!=0x1c4||e[6]!=0)return 1;++draws;}}if(!draws)return 1;std::cout<<"1000 composed submission/clip/draw cases passed, "<<draws<<" device draws\n";}
