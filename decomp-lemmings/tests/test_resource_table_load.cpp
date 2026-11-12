#include "resource_table_load_fixture.hpp"
#include "resource_registration_fixture.hpp"
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {
void require(bool v,const char* text){if(!v){std::cerr<<text<<'\n';std::exit(1);}}

using namespace lemmings;
using Entries = std::array<ResourceCacheEntry,24>;

// Expected writes follow the retained byte stores and string-copy contract.
// These helpers never invoke entry initialization/assignment or either loader.
void name_bytes(ResourceCacheEntry& entry,std::string_view name) {
    require(name.size()<entry.name_00.size(),"bounded expected name");
    std::copy(name.begin(),name.end(),entry.name_00.begin());
    entry.name_00[name.size()]='\0';
}
void low_byte(ResourceCacheEntry& entry,std::uint8_t value) {
    entry.unknown_84=(entry.unknown_84&0xffffff00u)|value;
}
void require_entries(const Entries& actual,const Entries& expected,const char* text) {
    for(std::size_t i=0;i<actual.size();++i) {
        require(actual[i].name_00==expected[i].name_00,text);
        require(actual[i].cached_80==expected[i].cached_80,text);
        require(actual[i].unknown_84==expected[i].unknown_84,text);
    }
}
void append_word(std::vector<std::uint8_t>& bytes,std::uint32_t word) {
    for(unsigned i=0;i<4;++i)bytes.push_back(static_cast<std::uint8_t>(word>>(8*i)));
}

struct ExpectedTable {
    table_load_test::Fixture table{0};
    Entries initial_new,after_cleanup,at_release,final_entries;
    const std::string long_name=std::string(127,'z');
    bool growing;

    explicit ExpectedTable(bool grow):growing(grow) {
        auto& old=table.old_block.entries;
        auto& fresh=table.new_block.entries;
        // Explicit dirty destinations make suffix/high-byte and untouched-slot
        // assertions meaningful, without inspecting host structure padding.
        for(unsigned i=0;i<24;++i) {
            old[i].name_00.fill('o');old[i].name_00[127]=0;
            old[i].cached_80=&table.nodes[2].value;
            old[i].unknown_84=0xa1000000u+(i<<8);
            fresh[i].name_00.fill('n');fresh[i].name_00[127]=0;
            fresh[i].cached_80=&table.nodes[3].value;
            fresh[i].unknown_84=0xb2000066u+(i<<8);
        }
        const std::array<std::string_view,8> old_names{
            "Keep","drop","Also","stale","repeat","null","unused","tail"};
        const std::array<std::uint8_t,8> active{0x80,0x7f,0xfe,0x22,0,0x99,0,0};
        const std::array<ResourceCacheNode*,8> cached{
            &table.nodes[0].value,&table.nodes[1].value,&table.nodes[2].value,
            &table.nodes[3].value,&table.nodes[0].value,nullptr,
            &table.nodes[2].value,&table.nodes[3].value};
        for(unsigned i=0;i<8;++i) {
            name_bytes(old[i],old_names[i]);old[i].cached_80=cached[i];low_byte(old[i],active[i]);
        }
        table.nodes[0].value.references_08=1;
        table.nodes[1].value.references_08=0;
        table.nodes[2].value.references_08=2;
        table.nodes[3].value.references_08=0;
        table.registration.count_30=8;
        table.registration.capacity_34=grow?8:16;
        table.registration.growth_38=3;
        initial_new=fresh;after_cleanup=old;
        for(unsigned i:{1u,3u,5u})low_byte(after_cleanup[i],0);

        // Incoming 8 + surviving loaded 2 requests 10 entries. With growth 3,
        // capacity 8 steps to 11; the subsequent duplicate shrink keeps 11.
        at_release=initial_new;
        for(unsigned i=0;i<11;++i) {
            at_release[i].name_00[0]=0;low_byte(at_release[i],0);
        }
        const std::array<std::uint8_t,8> copied_active{0x80,0,0xfe,0,0,0,0,0};
        for(unsigned i=0;i<8;++i) {
            name_bytes(at_release[i],old_names[i]);
            at_release[i].cached_80=cached[i];low_byte(at_release[i],copied_active[i]);
        }
        final_entries=grow?at_release:after_cleanup;
        // Duplicates: retained Keep, earlier incoming fresh, retained Also.
        // keep differs by case; inactive repeat does not count as a duplicate.
        // The first holes, in order, are 1,3,4,5,6.
        const std::array<unsigned,5> slots{1,3,4,5,6};
        const std::array<std::string_view,5> names{"fresh","keep","","repeat",long_name};
        for(unsigned i=0;i<slots.size();++i) {
            auto& entry=final_entries[slots[i]];
            name_bytes(entry,names[i]);entry.cached_80=nullptr;low_byte(entry,1);
        }

        table.bytes.clear();append_word(table.bytes,8);
        const std::array<std::string_view,8> incoming{
            "Keep","fresh","fresh","keep","","repeat",long_name,"Also"};
        for(std::string_view name:incoming) {
            append_word(table.bytes,static_cast<std::uint32_t>(name.size()));
            table.bytes.insert(table.bytes.end(),name.begin(),name.end());
        }
        table.bytes.insert(table.bytes.end(),{0xde,0xad,0xbe,0xef});
        table.stream={table.bytes};
    }

    ResourceEntryAllocation allocate(std::uint32_t bytes) {
        require(growing,"spare capacity must not allocate");
        require_entries(table.old_block.entries,after_cleanup,"cleanup before allocation");
        require_entries(table.new_block.entries,initial_new,"allocation precedes initialization");
        require(table.new_block.cookie==24,"cookie is not written before allocation returns");
        return table.allocate(bytes);
    }
    void deallocate(ResourceCacheEntry* entries) {
        require(growing&&entries==table.old_block.entries.data(),"release old allocation");
        require_entries(table.old_block.entries,after_cleanup,"copy leaves old storage untouched");
        require_entries(table.new_block.entries,at_release,"partial initialization and copy before release");
        require(table.new_block.cookie==11,"captured capacity written to cookie before release");
        require(table.cache.entries_30==entries,"publish new storage after release");
        table.deallocate(entries);
    }
    void run() {
        ResourceRegistrationHooks allocation{this,
            [](void* p,std::uint32_t n){return static_cast<ExpectedTable*>(p)->allocate(n);},
            [](void* p,ResourceCacheEntry* e){static_cast<ExpectedTable*>(p)->deallocate(e);}};
        load_resource_name_table(table.manager.lookup,table.manager.loaded,
            table.registration,table.stream,allocation,table.cleanup_hooks());
        require(table.manager.loaded==2,"loaded count tracks surviving nodes, not incoming names");
        require(table.registration.count_30==7,"8 incoming + 2 loaded - 3 duplicates");
        require(table.registration.capacity_34==(growing?11:16),"final shrink retains capacity");
        require(table.registration.growth_38==3,"growth increment preserved");
        require(table.cache.entries_30==(growing?table.new_block.entries.data():table.old_block.entries.data()),
                "expected final storage");
        require_entries(table.old_block.entries,growing?after_cleanup:final_entries,
                        "all old name bytes, cached pointers and flag bytes");
        require_entries(table.new_block.entries,growing?final_entries:initial_new,
                        "all new name bytes, cached pointers and flag bytes");
        require(table.old_block.cookie==24&&table.new_block.cookie==(growing?11u:24u),"array cookies");
        require(table.stream.cursor==191&&table.bytes.size()==195,"exact count and eight string reads");
        const std::array<std::uint8_t,4> trailing{0xde,0xad,0xbe,0xef};
        require(std::equal(trailing.begin(),trailing.end(),table.stream.bytes.begin()+191),
                "trailing input remains unread");

        // Fixture trace records event, loaded, logical, capacity, backing ID,
        // cursor, then event-specific resource/node/allocation data.
        const std::uint64_t capacity=growing?8:16;
        std::vector<std::uint64_t> expected_trace{
            3,4,8,capacity,1,0,0,            // destroy null resource before any read
            4,4,8,capacity,1,0,4,5,3,        // unlink before release; count still 4
            3,3,8,capacity,1,0,0x200c,
            4,3,8,capacity,1,0,6,2,5};
        if(growing) {
            const std::vector<std::uint64_t> growth_trace{
                1,2,8,11,1,4,1500,          // 11 * original stride 0x88 + cookie 4
                2,2,8,11,1,4,1,11};         // copied storage visible before publish
            expected_trace.insert(expected_trace.end(),growth_trace.begin(),growth_trace.end());
        }
        require(table.trace==expected_trace,"expected cleanup/allocation/copy/free order and metadata");
        auto& list=table.manager.lookup.list_04;
        require(list.begin.next_0c==&table.nodes[0]&&table.nodes[0].next_0c==&table.nodes[2]&&
                table.nodes[2].previous_10==&table.nodes[0]&&table.nodes[2].next_0c==&list.end&&
                list.end.previous_10==&table.nodes[2],"only referenced list nodes survive");
    }
};
}
int main() {
    using namespace lemmings;
    // Successful allocation, bounded names and valid storage only. In
    // particular, do not execute zero/negative-step nonterminating growth.
    ExpectedTable spare(false);spare.run();
    ExpectedTable growing(true);growing.run();
    registration_test::Fixture exact(0);exact.state.count_30=2;exact.state.capacity_34=2;exact.state.growth_38=1;
    auto hooks=exact.hooks();resize_resource_entries(exact.state,5,hooks);
    require(exact.new_block.cookie==5&&exact.state.capacity_34==5&&exact.state.count_30==5,"growth1 uses exact request");
    registration_test::Fixture stepped(0);stepped.state.count_30=2;stepped.state.capacity_34=2;stepped.state.growth_38=3;
    hooks=stepped.hooks();resize_resource_entries(stepped.state,5,hooks);
    require(stepped.new_block.cookie==8&&stepped.state.capacity_34==8,"stepped growth exceeds equal request");
    auto* storage=stepped.cache.entries_30;auto before=stepped.trace.size();resize_resource_entries(stepped.state,-1,hooks);
    require(stepped.cache.entries_30==storage&&stepped.state.capacity_34==8&&stepped.state.count_30==-1&&stepped.trace.size()==before,"shrinking preserves storage and capacity");
    registration_test::Fixture changed(10);changed.state.count_30=2;changed.state.capacity_34=2;changed.state.growth_38=3;
    hooks=changed.hooks();resize_resource_entries(changed.state,5,hooks);
    require(changed.new_block.cookie==8&&changed.state.capacity_34==-4&&changed.state.count_30==5,"captured cookie, live callback capacity, requested final count");
    table_load_test::Fixture table(0);table.run();
    require(table.manager.loaded==0&&table.registration.count_30==0&&table.stream.cursor==4,"empty incoming table evicts all unreferenced nodes before read");
    require(table.trace.size()>0&&table.trace[0]==3&&table.trace[5]==0,"destroy observes stream before count is read");
    // Repeatability only; the independent expected results are above.
    for(unsigned i=0;i<4096;++i) {table_load_test::Fixture a(i),b(i);a.run();b.run();require(a.equals(b)&&a.trace==b.trace,"deterministic resource table fixture");}
    std::cout<<"resource table retention, deduplication, resizing and callback ordering tests passed\n";
}
