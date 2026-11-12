#include "resource_cache_load_fixture.hpp"
#include <cassert>
#include <iostream>
using namespace lemmings;
int main() {
    ResourceCacheNode value{reinterpret_cast<void*>(0x1234),0xdeadbeef,0xffffffff};
    assert(initialize_resource_cache_value(value)==&value);
    assert(!value.resource_00 && value.unknown_04==0xdeadbeef && value.references_08==0);
    ResourceCacheLookupNode node;
    node.value={reinterpret_cast<void*>(0x4567),123,55};
    construct_resource_cache_node(node,node.value,&node,&node);
    assert(!node.value.resource_00 && node.value.unknown_04==123 && !node.value.references_08);
    assert(node.next_0c==&node && node.previous_10==&node);
    cache_load_test::Fixture allocation_failure(67);
    assert(allocation_failure.operation==2);
    auto* old_first=allocation_failure.state.manager.list_04.begin.next_0c;
    allocation_failure.run();
    assert(!allocation_failure.state.manager.list_04.begin.next_0c);
    assert(!old_first->previous_10);
    for(unsigned t=0;t<10000;++t) {
        cache_load_test::Fixture fixture(t);
        if(fixture.operation>1)continue;
        auto* cached=find_cached_resource(fixture.state.manager,fixture.index);
        const auto previous_count=fixture.state.loaded_count;
        const auto references=cached?cached->references_08:0;
        const auto pointer=cached?cached->resource_00:nullptr;
        auto result=fixture.run();
        if(cached) {
            assert(result==pointer && cached->references_08==references+1u);
            assert(fixture.trace.empty() && fixture.state.loaded_count==previous_count);
        } else {
            assert(fixture.trace.size()==3 && fixture.trace[0]==10 && fixture.trace[2]==20);
            assert(fixture.state.manager.list_04.begin.next_0c==&fixture.nodes[7]);
            assert(fixture.nodes[7].value.unknown_04==std::bit_cast<std::uint32_t>(fixture.index));
            assert(fixture.nodes[7].value.references_08==1);
            assert(result==fixture.nodes[7].value.resource_00);
            assert(fixture.nodes[7].next_0c->previous_10==&fixture.nodes[7]);
            auto expected_count=(t&2)?previous_count^0xfedcba98:previous_count;
            if(t&32)expected_count=0xffffffff;
            assert(fixture.state.loaded_count==expected_count+1u);
        }
    }
    for(unsigned t=0;t<1000;++t) {
        cache_load_test::ComposedFixture fixture(t);
        fixture.run_combined();
        if(fixture.combined_operation<2) {
            assert(fixture.entries[fixture.index].cached_80);
            unsigned notifications=0;
            for(auto event:fixture.trace)if(event>=100 && event<256)++notifications;
            if(fixture.combined_operation==1)assert(!notifications);
        }
    }
    std::cout<<"resource cache load unit checks passed\n";
}
