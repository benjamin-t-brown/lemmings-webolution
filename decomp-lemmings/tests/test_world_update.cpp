#include "world_update_fixture.hpp"
#include <iostream>
int main(){
    using namespace world_update_test;
    Fixture disabled(0,false);disabled.run_port();
    assert(disabled.w.status_00==1&&disabled.w.elapsed_5120==123&&disabled.trace.empty());
    // The x87 C0-only gate includes NaN, excludes both signed zeros, and
    // schedules at most one actor even for a very negative countdown.
    for(double timer:{-100.0,-0.0,0.0,1.0,std::numeric_limits<double>::quiet_NaN()}){
        Fixture f(9,false);f.entities.mode=0;f.entities.elapsed=0;
        f.w.sequence_enabled_5138=1;f.w.enabled_5139=1;
        f.w.sequence_count_5308=65535;f.w.sequence_done_50d3=1;
        f.w.sequence_ready_53a0=1;f.w.sequence_539c=nullptr;f.w.sequence_timer_5128=timer;
        // Install one valid actor independently of the generated list length.
        auto& list=f.entities.world.list_1ec8;auto& node=f.entities.nodes[1][0];
        node={&f.entities.entities[32].port,&list.end,&list.begin};list.begin.next=&node;list.end.previous=&node;
        f.w.sequence_cursor_5304=&node;f.run_port();
        const bool entered=!(timer>=0.0);
        assert(f.w.sequence_count_5308==(entered?0:65535));
        assert(f.w.sequence_cursor_5304==(entered?&list.begin:&node));
        int transitions=0;for(auto value:f.trace)if(value>>32==24)++transitions;
        assert(transitions==(entered?1:0));
    }
    Fixture initialized(2,false);initialized.run_port();
    assert(initialized.w.animations_538c.count==2);
    std::vector<int> setup;
    for(auto event:initialized.trace){auto kind=event>>32;if(kind==12)setup.push_back(static_cast<int>(event));}
    assert((setup==std::vector<int>{0xf0,0xf8}));
    for(int i=1;i<10000;++i){Fixture f(i,false);f.run_port();assert(f.valid&&f.entities.elapsed_matches);}
    std::cout<<"world update unit fixtures passed\n";
}
