#include "terrain_blast_fixture.hpp"
#include <cassert>
#include <algorithm>
using namespace blast_test;
int main(){
    Fixture f;f.cells.fill(1);apply_terrain_blast(f.grid,f.position,0,0.25,f);
    assert(f.valid && f.entities>1 && f.trace.back().kind==8);
    auto rectangle=std::find_if(f.trace.begin(),f.trace.end(),[](auto& e){return e.kind==9;});
    assert(rectangle!=f.trace.end() && rectangle->values[3]==bits(4) && rectangle->values[4]==bits(4));
    assert(rectangle->values[5]==bits(11) && rectangle->values[6]==11);
    assert(f.cells[8*256+8]==0 && f.cells[0]==1);
    Fixture blocked;blocked.cells.fill(0xa03);blocked.null_entities=true;
    apply_terrain_blast(blocked.grid,blocked.position,-7,0,blocked);
    assert(blocked.entities==1 && blocked.cells[8*256+8]==0xa03);
    assert(blocked.trace.front().kind==9 && blocked.trace.back().object==-1);
}
