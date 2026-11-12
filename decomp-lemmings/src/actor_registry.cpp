#include "lemmings/actor_registry.hpp"
#include <bit>
namespace lemmings {
namespace {
std::int32_t add(std::int32_t a,std::int32_t b){return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(a)+static_cast<std::uint32_t>(b));}
}
void append_actor_registry(ActorRegistry& a,void*& value,const ActorRegistryAllocation& h){
    if(a.capacity<=a.count){
        a.capacity=add(a.capacity,a.growth);
        auto* replacement=h.allocate(h.context,static_cast<std::uint32_t>(a.capacity)<<2);
        for(std::int32_t i=0;i<a.count;i=add(i,1))replacement[i]=a.data[i];
        h.release(h.context,a.data);
        a.data=replacement;
    }
    a.data[a.count]=value;
    a.count=add(a.count,1);
}
void register_world_actor(ActorRegistry& a,void* value,const ActorRegistryAllocation& h){append_actor_registry(a,value,h);}
void erase_actor_registry(ActorRegistry& a,std::int32_t index){
    for(;index<add(a.count,-1);index=add(index,1))a.data[index]=a.data[add(index,1)];
    a.count=add(a.count,-1);
}
void unregister_world_actor(ActorRegistry& a,void* value){
    std::int32_t i=0;
    while(i<a.count){if(a.data[i]==value)break;i=add(i,1);}
    if(i!=a.count)erase_actor_registry(a,i);
}
}
