#include "resource_registration_fixture.hpp"
#include <cstdlib>
#include <iostream>

namespace {
void require(bool value,const char* message){if(!value){std::cerr<<message<<'\n';std::exit(1);}}
}
int main() {
    using namespace registration_test;
    ResourceCacheEntry entry;
    entry.name_00.fill('x');entry.cached_80=reinterpret_cast<ResourceCacheNode*>(0x1234);entry.unknown_84=0xa1b2c3ff;
    initialize_resource_entry(entry);
    require(entry.name_00[0]==0&&entry.name_00[1]=='x'&&entry.cached_80==reinterpret_cast<ResourceCacheNode*>(0x1234)&&entry.unknown_84==0xa1b2c300,"default constructor must preserve uninitialized fields");
    initialize_named_resource_entry(entry,"ab");
    require(std::string(entry.name_00.data())=="ab"&&entry.name_00[3]=='x'&&!entry.cached_80&&entry.unknown_84==0xa1b2c301,"named constructor partial writes");
    ResourceCacheEntry other;other.name_00[0]='z';other.cached_80=reinterpret_cast<ResourceCacheNode*>(0x5678);other.unknown_84=0xdeadbe80;
    assign_resource_entry(entry,other);
    require(entry.name_00[0]=='z'&&entry.name_00[1]==0&&entry.name_00[2]==0&&entry.name_00[3]=='x'&&entry.cached_80==other.cached_80&&entry.unknown_84==0xa1b2c380,"assignment must copy only low active byte");
    // An existing name does not prevent a new registration; only active flags select a slot.
    Fixture reuse(0);reuse.cache.entries_30=reuse.old_block.entries.data();reuse.state.count_30=3;reuse.state.capacity_34=3;
    initialize_named_resource_entry(reuse.old_block.entries[0],"same");
    reuse.old_block.entries[1].unknown_84=0xabcdef00;
    auto hooks=reuse.hooks();require(register_resource_slot(reuse.state,"same",hooks)==1,"first inactive slot selected");
    require(reuse.trace.empty()&&reuse.state.count_30==3&&reuse.old_block.entries[1].unknown_84==0xabcdef01,"slot reuse must not allocate or change count");
    // The allocation hook changes count/storage and capacity after the new capacity is captured.
    Fixture captured(10);captured.cache.entries_30=captured.old_block.entries.data();captured.state.count_30=2;captured.state.capacity_34=2;captured.state.growth_38=3;
    for(int i=0;i<2;++i)captured.old_block.entries[i].unknown_84|=1;
    hooks=captured.hooks();require(register_resource_slot(captured.state,"new",hooks)==2,"returned index must precede callbacks");
    require(captured.new_block.cookie==5&&captured.state.capacity_34==-4&&captured.state.count_30==3,"captured capacity cookie and live metadata");
    require(std::string(captured.new_block.entries[2].name_00.data())=="new","name must be captured before allocation");
    Fixture release(6);release.cache.entries_30=release.old_block.entries.data();release.state.count_30=2;release.state.capacity_34=2;release.state.growth_38=3;
    const std::string expected=release.old_block.entries[0].name_00.data();
    hooks=release.hooks();append_resource_entry(release.state,release.source,hooks);
    require(std::string(release.new_block.entries[0].name_00.data())==expected,"copy old contents before release");
    require(release.cache.entries_30==release.new_block.entries.data()&&release.state.count_30==4&&release.state.capacity_34==21,"append uses release-mutated live count and replaces data");
    require(std::string(release.new_block.entries[3].name_00.data())=="#","append source remains live across release");
    std::cout<<"resource registration constructor, reuse, growth and callback ordering tests passed\n";
}
