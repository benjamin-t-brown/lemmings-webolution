#include "lemmings/actor_shadow_segments.hpp"
#include <cassert>
#include <limits>
#include <vector>

using namespace lemmings;
namespace {
struct List : ActorShadowSegmentList {
    std::vector<std::array<double,3>> points;
    std::vector<int> calls;
    void resize_421260(int size) override { calls.push_back(size);points.resize(size); }
    std::array<double,3>& point_at_421240(int index) override { calls.push_back(-1-index);return points.at(index); }
};
}
int main() {
    std::array<std::int32_t,8*256> cells{};
    CollisionGridHead grid{};grid.height=8;grid.cells.data=cells.data();
    List list;
    build_actor_shadow_segments({10.5,4.75,9},9.25,11.75,grid,list);
    assert((list.points==std::vector<std::array<double,3>>{{10,4,0},{11,4,0},{12,4,0},{12,4,0}}));
    assert((list.calls==std::vector<int>{1,-1,2,-2,3,-3,4,-4}));
    // Any nonzero value below is solid for this branch, even bit0x80 alone.
    cells.fill(0);for(int x=9;x<=11;++x)cells[3*256+x]=0x80;
    list.calls.clear();build_actor_shadow_segments({10.5,4.75,9},9.25,11.0,grid,list);
    assert((list.points==std::vector<std::array<double,3>>{{9.25,4,0},{10,4,0},{11,4,0}}));
    // Solid current/above preserves the previous selected pair until last.
    for(int x=9;x<=11;++x)cells[4*256+x]=cells[5*256+x]=1;
    build_actor_shadow_segments({10.5,4.75,9},9.25,11,grid,list);
    assert((list.points==std::vector<std::array<double,3>>{{9.25,4,0},{9.25,4,0},{11,4,0}}));
    cells.fill(0);for(int x=9;x<=11;++x)cells[2*256+x]=2;
    build_actor_shadow_segments({10.5,4.75,9},9.25,11,grid,list);
    assert((list.points==std::vector<std::array<double,3>>{{9.25,3,0},{9.25,3,0},{11,3,0}}));
    const auto previous=list.points;list.calls.clear();
    build_actor_shadow_segments({2147483647.0,4,0},0,1,grid,list);
    assert(list.calls.empty() && list.points==previous); // Wrapped first > last.
    cells.fill(0);build_actor_shadow_segments({0,4,0},-1,1,grid,list);
    assert(list.points[0][0]==256); // No final wrap of the selected endpoint.
    build_actor_shadow_segments({10,4,0},9,std::numeric_limits<double>::quiet_NaN(),grid,list);
    assert(list.points.size()==4); // Unordered right endpoint inserts a point.
}
