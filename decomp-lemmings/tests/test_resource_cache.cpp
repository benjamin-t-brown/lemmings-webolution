#include "resource_cache_fixture.hpp"
#include <cassert>
int main(){using namespace cache_test;
    Fixture cached(0,false);auto before=cached.nodes[0].references_08;assert(cached.run()==cached.nodes[0].resource_00);assert(cached.nodes[0].references_08==before+1&&cached.trace.empty());
    Fixture release(1,false);release.nodes[0].references_08=0;release.run();assert(release.nodes[0].references_08==0xffffffffu);
    Fixture loaded(6,false);auto count=loaded.nodes[1].references_08;assert(loaded.run()==reinterpret_cast<void*>(std::uintptr_t(0x12340007)));assert(loaded.nodes[1].references_08==count);
    Fixture balanced(8,false);balanced.run();for(auto event:balanced.trace)assert(event<300);
    for(unsigned i=0;i<20000;++i){Fixture f(i,false);f.run();}
}
