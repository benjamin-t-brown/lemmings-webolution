#pragma once
#include "lemmings/actor_shadow.hpp"
#include <vector>

namespace shadow_test {
using namespace lemmings;
struct Effects : ActorShadowCallbacks {
    std::vector<std::array<double,3>> points{{10,5,0},{11,5,0},{12,6,0}};
    std::vector<int> calls;
    std::uint8_t has = 0;
    double left = 0, right = 0;
    void* create_point_list_4229f0() override { calls.push_back(1); return this; }
    void resize_point_list_421260(void*, std::int32_t count) override { calls.push_back(10+count); }
    void destroy_point_list_422a30(void*) override { calls.push_back(2); }
    std::uint8_t has_state_42ecc9(ShadowActor&, std::int32_t type) override { calls.push_back(100+type); return has; }
    void build_segments_459557(ActorShadow&,void*,double a,double b) override { calls.push_back(3);left=a;right=b; }
    std::int32_t point_count_459990(void*) override { calls.push_back(4);return static_cast<std::int32_t>(points.size()); }
    const std::array<double,3>& point_at_421240(void*,std::int32_t i) override { calls.push_back(20+i);return points.at(i); }
    void* lookup_resource_414a10(std::int32_t) override { calls.push_back(5);return this; }
    void activate_resource_47261f(void*) override { calls.push_back(6); }
};
}
