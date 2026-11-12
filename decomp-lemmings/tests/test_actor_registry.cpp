#include "lemmings/actor_registry.hpp"
#include <array>
#include <cassert>
using namespace lemmings;
int main(){
    int one,two,three;
    std::array<void*,8> old{&one,&two},replacement{};
    ActorRegistry a{2,2,4,old.data()};void* value=&three;
    struct Context{ActorRegistry* a;void** replacement;void** input;void* changed;bool released=false;}c{&a,replacement.data(),&value,&one};
    ActorRegistryAllocation h{&c,[](void* p,std::uint32_t n){auto& c=*static_cast<Context*>(p);assert(n==24);assert(c.a->capacity==6);*c.input=c.changed;return c.replacement;},[](void* p,void**){static_cast<Context*>(p)->released=true;}};
    append_actor_registry(a,value,h);assert(c.released&&a.data==replacement.data()&&a.count==3&&a.data[2]==&one);
    unregister_world_actor(a,&one);assert(a.count==2&&a.data[0]==&two&&a.data[1]==&one&&a.data[2]==&one);
    unregister_world_actor(a,&three);assert(a.count==2);
    register_world_actor(a,&three,h);assert(a.count==3&&a.data[2]==&three);
    erase_actor_registry(a,2);assert(a.count==2&&a.data[2]==&three);
}
