#pragma once
#include "lemmings/actor_resource_selection.hpp"
#include <string>
#include <vector>

namespace resource_selection_fixture {
struct Event {
    std::string name;
    std::array<std::int32_t,4> ids;
    bool operator==(const Event&)const=default;
};
struct Fixture {
    std::array<std::uint8_t,4> enabled{1,1,1,1};
    std::array<std::int32_t,4> quality{0,1,0,1},ids{11,22,33,44};
    std::vector<Event> events;
    unsigned script=0;
    std::int32_t result=0;
    static std::int32_t register_resource(void* context,std::string_view name) {
        auto& f=*static_cast<Fixture*>(context);f.events.push_back({std::string(name),f.ids});
        if(f.script&1)for(auto& quality:f.quality)quality=1-quality;
        if(f.script&2)for(auto& enabled:f.enabled)enabled=enabled?0:0x80;
        if(f.script&4)f.ids={-91,-92,-93,-94};
        return f.result;
    }
    void run(){lemmings::select_actor_resources(enabled,quality,ids,{this,register_resource});}
};
} // namespace resource_selection_fixture
