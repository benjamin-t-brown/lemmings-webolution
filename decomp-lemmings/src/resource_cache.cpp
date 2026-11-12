#include "lemmings/resource_cache.hpp"
namespace lemmings {
void* acquire_resource(ResourceCache& cache,std::int32_t index,const ResourceCacheHooks& hooks){
    auto& entry=cache.entries_30[index];
    if(!entry.cached_80){
        entry.cached_80=hooks.find_4306f0(hooks.context,index);
        if(!entry.cached_80){
            void* result=hooks.load_430660(hooks.context,index);
            if(hooks.notify)hooks.notify(hooks.context,entry.name_00.data());
            entry.cached_80=hooks.find_4306f0(hooks.context,index);
            return result;
        }
    }
    ++entry.cached_80->references_08;
    return entry.cached_80->resource_00;
}
void release_resource(ResourceCache& cache,std::int32_t index){--cache.entries_30[index].cached_80->references_08;}
void* get_resource_balanced(ResourceCache& cache,std::int32_t index,const ResourceCacheHooks& hooks){
    auto silent=hooks;silent.notify=nullptr;
    void* result=acquire_resource(cache,index,silent);
    release_resource(cache,index);
    return result;
}
}
