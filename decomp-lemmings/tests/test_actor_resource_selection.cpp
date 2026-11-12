#include "actor_resource_selection_fixture.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace resource_selection_fixture;
    Fixture all;all.result=-7;all.run();
    const std::vector<std::string> expected{"LemmingsLo","LemmingsWaterHi","LemmingsAcidLo","EvilRatHi"};
    assert(all.events.size()==4);
    for(unsigned i=0;i<4;++i){assert(all.events[i].name==expected[i]);assert(all.ids[i]==-7);}
    Fixture disabled;disabled.enabled={0,0,0,0};disabled.quality={-1,99,-100,123456};disabled.run();
    assert(disabled.events.empty() && (disabled.ids==std::array<std::int32_t,4>{-1,-1,-1,-1}));
    Fixture mutated;mutated.enabled={0x80,1,0,1};mutated.script=7;mutated.result=0;mutated.run();
    assert(mutated.events.size()==3 && mutated.events[0].name=="LemmingsLo" && mutated.events[1].name=="LemmingsAcidHi" && mutated.events[2].name=="EvilRatHi");
    assert((mutated.ids==std::array<std::int32_t,4>{-91,-92,-93,0}));
    std::cout<<"actor resource selection units passed\n";
}
