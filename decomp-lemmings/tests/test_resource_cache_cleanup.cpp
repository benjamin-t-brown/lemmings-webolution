#include "resource_cache_cleanup_fixture.hpp"
#include <cassert>
using namespace cleanup_test;
int main(){
    Fixture empty(0,0,0);empty.run();assert(empty.trace.empty()&&empty.manager.loaded_count==0);
    Fixture evict(3,0,0);evict.nodes[0].value.resource_00=nullptr;
    evict.run();assert(evict.trace.size()==2&&evict.trace[0].kind==1&&evict.trace[0].argument==0);
    assert(evict.trace[1].kind==3&&evict.manager.loaded_count==0xffffffffu);
    assert(evict.manager.lookup.list_04.begin.next_0c==&evict.nodes[1]);
    Fixture unload(4,0,1);unload.run();
    for(auto* n=unload.manager.lookup.list_04.begin.next_0c;n!=&unload.manager.lookup.list_04.end;n=n->next_0c)assert(!n->value.resource_00);
    Fixture flags(4,0,5);flags.entries_a[0].unknown_84=0x12345680;flags.entries_a[0].cached_80=nullptr;
    flags.run();assert(flags.entries_a[0].unknown_84==0x12345600);
    Fixture refresh(4,0,4);refresh.run();
    for(int i=0;i<refresh.manager.entry_count;++i)assert(refresh.manager.cache.entries_30[i].cached_80==find_cached_resource(refresh.manager.lookup,i));
    Fixture unlink(2,0,6);auto saved=unlink.nodes[0];unlink.run();
    assert(unlink.manager.loaded_count==2&&unlink.nodes[0].next_0c==saved.next_0c&&unlink.nodes[0].previous_10==saved.previous_10);
    assert(unlink.nodes[1].previous_10==&unlink.manager.lookup.list_04.begin);
}
