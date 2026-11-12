#include "resource_manager_construction_fixture.hpp"
#include <iostream>

namespace {
void require(bool value,const char* message){if(!value){std::cerr<<message<<'\n';std::exit(1);}}
}
int main() {
    using namespace manager_construction_test;
    Fixture sprite(0);const auto begin_id=sprite.manager.list_04.begin.value.unknown_04,end_id=sprite.manager.list_04.end.value.unknown_04;
    sprite.run();auto bytes=sprite.serialize(0x10000);
    require(Fixture::read(bytes.data())==0x4b1ba8,"sprite vtable token");
    require(sprite.loaded_count==0&&sprite.registration.count_30==0&&sprite.registration.capacity_34==0&&sprite.registration.growth_38==1&&!sprite.cache.entries_30,"sprite manager counters and array");
    auto& list=sprite.manager.list_04;
    require(!list.begin.value.resource_00&&!list.end.value.resource_00&&!list.begin.value.references_08&&!list.end.value.references_08,"sentinel payload reset");
    require(list.begin.value.unknown_04==begin_id&&list.end.value.unknown_04==end_id,"sentinel IDs preserved");
    require(list.begin.next_0c==&list.end&&!list.begin.previous_10&&!list.end.next_0c&&list.end.previous_10==&list.begin,"empty sentinel topology");
    Fixture named(1);named.run();require(Fixture::read(named.serialize(0).data())==0x4b1bc0,"named vtable token");
    Fixture base(2);base.run();bytes=base.serialize(0x10000);
    require(Fixture::read(bytes.data())==0x4b1bd8&&std::memcmp(bytes.data()+0x30,base.initial.data()+0x30,16)==0,"base must preserve named vector fields");
    Fixture node(4);node.run();bytes=node.serialize(0x10000);
    for(unsigned i=0;i<64;++i)if(!((i>=4&&i<8)||(i>=12&&i<16)))require(bytes[i]==node.initial[i],"node helper must preserve ID, links, all other fields");
    Fixture vector(5);vector.run();bytes=vector.serialize(0x10000);
    require(std::memcmp(bytes.data(),vector.initial.data(),0x30)==0,"vector helper preserves manager and list");
    std::cout<<"resource manager constructor topology, preserved IDs and scope tests passed\n";
}
