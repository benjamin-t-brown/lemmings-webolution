#include "world_entity_fixture.hpp"
#include <iostream>
int main(){
    using namespace world_test;
    // All entities return false and are enabled: array 1EB8 visits all shifted
    // entries; array 1F00 skips the entry shifted into each removed slot.
    Fixture f(99,false);f.mode=0;
    for(auto& e:f.entities){e.result=0;e.port.enabled_38=1;}
    for(auto& a:f.world.arrays)a.count=5;
    f.world.list_1ea0.begin.next=&f.world.list_1ea0.end;
    f.world.list_1ec8.begin.next=&f.world.list_1ec8.end;
    f.run_port();
    std::vector<int> transient,removable,registry;
    for(auto value:f.trace)if(value>>32==1){int id=static_cast<int>(value);if(id/16==1)transient.push_back(id%16);if(id/16==4)removable.push_back(id%16);if(id/16==10)registry.push_back(id%16);}
    assert((transient==std::vector<int>{0,1,2,3,4}));
    assert((removable==std::vector<int>{0,2,4}));
    assert((registry==std::vector<int>{0,1,2,3,4}));
    assert(f.world.arrays[0].count==0&&f.world.arrays[2].count==2);
    // Nonzero update results prevent disposal, while unconditional arrays
    // still update disabled entities. Negative counts perform zero accesses.
    Fixture empty(0,false);empty.run_port();assert(empty.trace.empty());
    Fixture enabled(99,false);enabled.mode=0;for(auto& e:enabled.entities){e.result=0x80;e.port.enabled_38=0;}
    for(auto& a:enabled.world.arrays)a.count=1;
    enabled.world.list_1ea0.begin.next=&enabled.world.list_1ea0.end;enabled.world.list_1ec8.begin.next=&enabled.world.list_1ec8.end;
    enabled.run_port();assert(enabled.trace.size()==3);assert((enabled.trace[0]&0xffffffff)==16);assert((enabled.trace[1]&0xffffffff)==128);assert((enabled.trace[2]&0xffffffff)==144);
    // Updates may replace the live slot. Transient disposal keeps the old
    // pointer, while removable/registry collections reread the live slot.
    Fixture replaced(99,false);replaced.mode=1;
    for(auto& e:replaced.entities){e.result=0;e.port.enabled_38=1;}
    for(auto& a:replaced.world.arrays)a.count=0;
    for(int a:{0,2,8})replaced.world.arrays[a].count=1;
    replaced.world.list_1ea0.begin.next=&replaced.world.list_1ea0.end;
    replaced.world.list_1ec8.begin.next=&replaced.world.list_1ec8.end;
    replaced.run_port();
    auto event=[](unsigned kind,int id){return (std::uint64_t(kind)<<32)|static_cast<std::uint32_t>(id);};
    assert((replaced.trace==std::vector<std::uint64_t>{event(1,16),event(2,16),event(5,0),event(1,64),event(2,72),event(6,0),event(1,160),event(7,168),event(8,168)}));
    // The successor is cached before invoking update.
    Fixture linked(99,false);linked.mode=4;for(auto& a:linked.world.arrays)a.count=0;
    for(auto& e:linked.entities)e.result=1;
    linked.world.list_1ec8.begin.next=&linked.world.list_1ec8.end;
    linked.world.list_1ea0.begin.next=&linked.nodes[0][0];
    linked.nodes[0][0]={&linked.entities[0].port,&linked.nodes[0][1],&linked.world.list_1ea0.begin};
    linked.nodes[0][1]={&linked.entities[1].port,&linked.world.list_1ea0.end,&linked.nodes[0][0]};
    linked.run_port();assert((linked.trace==std::vector<std::uint64_t>{event(1,0),event(1,1)}));
    for(int trial=0;trial<10000;++trial){Fixture sample(trial,false);sample.run_port();}
    std::cout<<"world entity collection unit cases passed\n";
}
