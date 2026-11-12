#include "lemmings/world_entity_pass.hpp"

namespace lemmings {
WorldEntityList::WorldEntityList() {begin.next=&end;end.previous=&begin;}
void update_world_entity_collections(WorldEntityPass& world,double elapsed,const WorldEntityPassHooks& hooks) {
    auto update=[](WorldUpdateEntity* entity,double dt){return entity->operations->update_14(*entity,dt);};
    auto visit_list=[&](WorldEntityList& list,bool actors) {
        auto* next=list.begin.next;
        while(next!=&list.end) {
            auto* const node=next;
            next=node->next;
            if(update(node->entity,elapsed)==0) {
                hooks.enqueue_4090b0(hooks.context,*node->entity);
                if(actors) hooks.actor_removed_notice(hooks.context,node->entity->identifier_3a);
                hooks.unlink_list_node(hooks.context,*node,actors);
            }
        }
    };
    visit_list(world.list_1ea0,false);
    auto& transient=world.array(WorldArray::array_1eb8);
    for(std::int32_t index=0;index<transient.count;++index) {
        auto* const entity=transient.data[index];
        if(update(entity,elapsed)==0) {
            hooks.enqueue_4090b0(hooks.context,*entity);
            hooks.erase_array_index_4465d0(hooks.context,index);
            --index;
        }
    }
    visit_list(world.list_1ec8,true);
    auto visit_enabled=[&](WorldArray kind) {
        auto& array=world.array(kind);
        for(std::int32_t index=0;index<array.count;++index)
            if(array.data[index]->enabled_38!=0) update(array.data[index],elapsed);
    };
    visit_enabled(WorldArray::array_1ef0);
    auto& removable=world.array(WorldArray::array_1f00);
    for(std::int32_t index=0;index<removable.count;++index) {
        if(removable.data[index]->enabled_38!=0 && update(removable.data[index],elapsed)==0) {
            hooks.enqueue_4090b0(hooks.context,*removable.data[index]);
            hooks.erase_array_entry_446c70(hooks.context,&removable.data[index]);
            // No decrement here: original advances even if removal shifts data.
        }
    }
    visit_enabled(WorldArray::array_1f50);
    visit_enabled(WorldArray::array_1f60);
    visit_enabled(WorldArray::array_1f70);
    for(auto kind:{WorldArray::array_1f90,WorldArray::array_1f20}) {
        auto& array=world.array(kind);
        for(std::int32_t index=0;index<array.count;++index) update(array.data[index],elapsed);
    }
    auto& registry=world.array(WorldArray::array_1fa0);
    for(std::int32_t index=0;index<registry.count;++index) {
        if(registry.data[index]->enabled_38!=0 && update(registry.data[index],elapsed)==0) {
            auto* const current=registry.data[index];
            current->operations->deactivate_08(*current);
            hooks.register_43b536(hooks.context,*registry.data[index]);
        }
    }
}
} // namespace lemmings
