#include "resource_cache_find_fixture.hpp"
#include <cassert>
#include <iostream>
#include <limits>
int main(){
    using namespace lemmings;
    ResourceCacheLookupManager manager;
    auto& list=manager.list_04;
    // Sentinel contents must not match: the end's next link is deliberately
    // invalid, so a correct lookup never follows or inspects it as a node.
    list.end.value.unknown_04=0xffffffffu;
    list.end.next_0c=reinterpret_cast<ResourceCacheLookupNode*>(std::uintptr_t(1));
    assert(find_cached_resource(manager,-1)==nullptr);
    assert(resource_cache_first(list)==&list.end);
    assert(resource_cache_is_end(list,&list.end));
    assert(!resource_cache_is_end(list,nullptr));
    assert(!resource_cache_is_end(list,&list.begin));
    ResourceCacheLookupNode first,second,third;
    first.value={nullptr,0x80000000u,0};
    second.value={reinterpret_cast<void*>(std::uintptr_t(1)),42,0xffffffffu};
    third.value={nullptr,0x80000000u,123};
    list.begin.next_0c=&first;first.next_0c=&second;second.next_0c=&third;third.next_0c=&list.end;
    assert(find_cached_resource(manager,std::numeric_limits<std::int32_t>::min())==&first.value);
    assert(find_cached_resource(manager,42)==&second.value);
    assert(find_cached_resource(manager,43)==nullptr);
    assert(first.value.references_08==0&&second.value.references_08==0xffffffffu);
    // Changing the first link changes duplicate precedence, independently of
    // node storage order and backward links.
    list.begin.next_0c=&third;third.next_0c=&first;first.next_0c=&second;second.next_0c=&list.end;
    assert(find_cached_resource(manager,std::numeric_limits<std::int32_t>::min())==&third.value);
    std::size_t comparisons=0;
    for(unsigned trial=0;trial<20000;++trial){cache_find_test::Fixture fixture(trial);const auto original=fixture.bytes();for(auto id:fixture.queries){auto* expected=fixture.expected(id);assert(find_cached_resource_node(fixture.manager.list_04,id)==expected);assert(find_cached_resource(fixture.manager,id)==(expected?&expected->value:nullptr));++comparisons;}assert(fixture.bytes()==original);}
    std::cout<<comparisons<<" resource-cache lookup cases passed\n";
}
