#pragma once
#include "world_entity_fixture.hpp"
#include "lemmings/world_update.hpp"
#include <cmath>
namespace world_update_test {
using namespace lemmings;
struct State {void** vtable=nullptr;int id=0;};
struct Fixture {
    world_test::Fixture entities;
    WorldUpdateState w;
    std::array<WorldAnimationObject,4> animations{};
    std::array<WorldTimedRecord,4> records{};
    std::array<WorldAuxiliaryRecord,4> auxiliary{};
    State sequence,old_behavior,new_behavior;
    std::vector<std::uint64_t> trace;
    int constructors=0,updates=0;
    bool valid=true; int failure=0;
    Fixture(int trial,bool native,void** entity_table=nullptr,void** state_table=nullptr):entities(trial,native,entity_table){
        entities.observer_context=this;
        entities.observer=[](void* context,std::uint64_t event){static_cast<Fixture*>(context)->trace.push_back(event+(std::uint64_t{100}<<32));};
        sequence={state_table,1};old_behavior={state_table,2};new_behavior={state_table,3};
        w.enabled_5139=trial%13!=0;w.sequence_enabled_5138=trial%7!=0;
        w.optional_50b8=trial%2?&sequence:nullptr;w.optional_50c0=trial%3?&old_behavior:nullptr;w.optional_50b4=trial%5?&new_behavior:nullptr;w.optional_50bc=trial%7?&sequence:nullptr;w.optional_50c8=trial%11?&sequence:nullptr;
        w.animations_538c={trial%19==0?-1:trial%2?2:0,animations.data()};w.records_5100={trial%17==0?-1:trial%5,records.data()};w.auxiliary_1fd0={trial%23==0?-1:trial%5,auxiliary.data()};w.status_00=99;w.value_5098=99;w.elapsed_5120=123;
        w.sequence_count_5308=trial%3==0?0:trial%11==0?65535:1;
        w.sequence_done_50d3=trial%2;w.sequence_ready_53a0=trial%2;
        w.sequence_539c=trial%3==1?&sequence:nullptr;
        w.sequence_cursor_5304=entities.world.list_1ec8.end.previous;
        double timers[]={-1.0,0.0,-0.0,0.01,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity()};w.sequence_timer_5128=timers[trial%7];
        for(int i=0;i<4;++i){records[i].paused_168=(trial+i)%3==0;records[i].remaining_170=timers[(trial+i)%7];int id=i;std::memcpy(animations[i].unknown.data(),&id,4);}
        if(trial>=64){std::mt19937_64 random(0x4423d2ull+trial);auto value=[&]{std::uint64_t bits=random();double v;std::memcpy(&v,&bits,8);return std::isnan(v)?std::numeric_limits<double>::quiet_NaN():v;};entities.elapsed=value();w.sequence_timer_5128=value();for(auto& r:records)r.remaining_170=value();}
        for(auto& e:entities.entities)e.port.primary_behavior_340=trial%3?&old_behavior:nullptr;
        if(native){auto* begin=reinterpret_cast<world_test::Node*>(entities.raw_world.data()+0x1ec8);auto* tail=raw_node(w.sequence_cursor_5304);std::memcpy(entities.raw_world.data()+0x1edc,&tail,4);entities.raw_nodes[1][0].previous=begin;sync(false);}
    }
    bool native()const{return entities.native;}
    world_test::Node* raw_node(WorldEntityNode* p){auto& l=entities.world.list_1ec8;if(p==&l.begin)return reinterpret_cast<world_test::Node*>(entities.raw_world.data()+0x1ec8);if(p==&l.end)return entities.raw_end(1);return &entities.raw_nodes[1][p-entities.nodes[1].data()];}
    WorldEntityNode* port_node(world_test::Node* p){if(p==reinterpret_cast<world_test::Node*>(entities.raw_world.data()+0x1ec8))return &entities.world.list_1ec8.begin;if(p==entities.raw_end(1))return &entities.world.list_1ec8.end;return &entities.nodes[1][p-entities.raw_nodes[1].data()];}
    template<class T>void field(bool read,int offset,T& value){if(read)std::memcpy(&value,entities.raw_world.data()+offset,sizeof(T));else std::memcpy(entities.raw_world.data()+offset,&value,sizeof(T));}
    void sync(bool read){if(!native())return;
        field(read,0,w.status_00);field(read,0x50b8,w.optional_50b8);field(read,0x50c0,w.optional_50c0);field(read,0x50b4,w.optional_50b4);field(read,0x50bc,w.optional_50bc);field(read,0x50c8,w.optional_50c8);field(read,0x50d3,w.sequence_done_50d3);field(read,0x5098,w.value_5098);field(read,0x5120,w.elapsed_5120);field(read,0x5128,w.sequence_timer_5128);field(read,0x5138,w.sequence_enabled_5138);field(read,0x5139,w.enabled_5139);field(read,0x5308,w.sequence_count_5308);field(read,0x539c,w.sequence_539c);field(read,0x53a0,w.sequence_ready_53a0);
        auto* cursor=read?nullptr:raw_node(w.sequence_cursor_5304);field(read,0x5304,cursor);if(read)w.sequence_cursor_5304=port_node(cursor);
        field(read,0x538c,w.animations_538c.count);field(read,0x5398,w.animations_538c.data);field(read,0x5100,w.records_5100.count);field(read,0x510c,w.records_5100.data);field(read,0x1fd0,w.auxiliary_1fd0.count);field(read,0x1fdc,w.auxiliary_1fd0.data);
        for(auto& e:entities.entities){if(read)std::memcpy(&e.port.primary_behavior_340,e.raw.data()+0x340,4);else std::memcpy(e.raw.data()+0x340,&e.port.primary_behavior_340,4);}
    }
    void event(unsigned kind,int value=0){trace.push_back((std::uint64_t(kind)<<32)|static_cast<std::uint32_t>(value));}
    void time(double dt){std::uint64_t a,b;std::memcpy(&a,&dt,8);std::memcpy(&b,&entities.elapsed,8);if(a!=b){valid=false;failure=1;}}
    void tick(unsigned kind,double dt){sync(true);time(dt);event(kind);if(kind==10&&entities.trial%4==0)w.optional_50b8=&sequence;if(kind==10&&entities.trial%6==0)w.elapsed_5120=987;sync(false);}
    void optional(unsigned addr,void* obj,double dt){sync(true);time(dt);event(addr,static_cast<State*>(obj)->id);sync(false);}
    void init(WorldAnimationObject& a){sync(true);std::memset(a.unknown.data(),0,40);int id=constructors++;std::memcpy(a.unknown.data(),&id,4);event(11,id);sync(false);}
    int animation_id(WorldAnimationObject& a){int id;std::memcpy(&id,a.unknown.data(),4);return id;}
    void config(WorldAnimationObject& a,void* payload,int first,int last,double rate){sync(true);event(12,first);if(payload!=&old_behavior||last!=first+7||bits(rate)!=0x3faeb851eb851eb8ull){valid=false;failure=2;}int id=(first-0xf0)/8;std::memcpy(a.unknown.data(),&id,4);sync(false);}
    void append(WorldAnimationObject& a){sync(true);event(13,animation_id(a));w.animations_538c.data[w.animations_538c.count++]=a;sync(false);}
    void destroy_animation(WorldAnimationObject& a){event(14,animation_id(a));}
    void animate(WorldAnimationObject& a,double dt){sync(true);time(dt);event(15,animation_id(a));sync(false);}
    void* create_sequence(int x,int y){sync(true);if(x!=5||y!=5){valid=false;failure=3;}event(16);sync(false);return &sequence;}
    int sequence_update(void* s,double& dt){sync(true);event(17,static_cast<State*>(s)->id);trace.push_back(bits(dt));++updates;if(entities.trial%4==0)w.sequence_timer_5128=-1;dt=456;sync(false);return entities.trial%3==0?1:entities.trial%3==1?2:0;}
    void position(void* s,int x,int y,unsigned flag){sync(true);event(18,static_cast<State*>(s)->id);if(x!=320||y!=240||flag!=1){valid=false;failure=4;}if(entities.trial%11==2)w.sequence_539c=&old_behavior;sync(false);}
    void leave(void* s){sync(true);event(19,static_cast<State*>(s)->id);if(entities.trial%5==0)w.sequence_539c=nullptr;sync(false);}
    void destroy(void* s,unsigned flag){sync(true);event(20,static_cast<State*>(s)->id);if(flag!=1){valid=false;failure=5;}sync(false);}
    int query(){sync(true);event(21);sync(false);return entities.trial%2?0x100:0;}
    void action(){sync(true);event(22);w.value_5098=555;sync(false);}
    void* behavior(){sync(true);void* answer=entities.trial%13==1?nullptr:&new_behavior;if(answer)event(23);sync(false);return answer;}
    void primary(world_test::Entity& e,void* state){sync(true);event(24,e.id);if(entities.trial%4!=1)e.port.primary_behavior_340=state;if(entities.trial%6==0)w.sequence_timer_5128=-2;sync(false);}
    void before(){sync(true);event(25);if(entities.trial%4==0)w.auxiliary_1fd0.count=2;sync(false);}
    void aux(WorldAuxiliaryRecord& a,double dt){sync(true);time(dt);event(26,static_cast<int>(&a-auxiliary.data()));if(entities.trial%5==0)w.auxiliary_1fd0.count=1;sync(false);}
    WorldUpdateHooks hooks(){return {this,
        [](void* p,double d){static_cast<Fixture*>(p)->tick(10,d);},
        [](void* p,std::uint32_t a,void* o,double d){static_cast<Fixture*>(p)->optional(a,o,d);},
        [](void* p){static_cast<Fixture*>(p)->event(30);return 42u;},
        [](void* p,std::uint32_t id)->void*{auto& f=*static_cast<Fixture*>(p);f.event(31,id);return &f.old_behavior;},
        [](void* p,WorldAnimationObject& a){static_cast<Fixture*>(p)->init(a);},
        [](void* p,WorldAnimationObject& a,void* r,int b,int e,double d){static_cast<Fixture*>(p)->config(a,r,b,e,d);},
        [](void* p,WorldAnimationObject& a){static_cast<Fixture*>(p)->append(a);},
        [](void* p,WorldAnimationObject& a){static_cast<Fixture*>(p)->destroy_animation(a);},
        [](void* p,WorldAnimationObject& a,double d){static_cast<Fixture*>(p)->animate(a,d);},
        [](void* p,int x,int y){return static_cast<Fixture*>(p)->create_sequence(x,y);},
        [](void* p,void* s,double& d){return static_cast<Fixture*>(p)->sequence_update(s,d);},
        [](void* p,void* s,int x,int y,std::uint8_t f){static_cast<Fixture*>(p)->position(s,x,y,f);},
        [](void* p,void* s){static_cast<Fixture*>(p)->leave(s);},
        [](void* p,void* s,unsigned f){static_cast<Fixture*>(p)->destroy(s,f);},
        [](void* p){return static_cast<Fixture*>(p)->query();},
        [](void* p){static_cast<Fixture*>(p)->action();},
        [](void* p){return static_cast<Fixture*>(p)->behavior();},
        [](void* p,WorldUpdateEntity& e,void* s){static_cast<Fixture*>(p)->primary(*static_cast<world_test::Entity*>(e.context),s);},
        [](void* p){static_cast<Fixture*>(p)->before();},
        [](void* p,WorldAuxiliaryRecord& a,double d){static_cast<Fixture*>(p)->aux(a,d);},
        [](void* p,double d){static_cast<Fixture*>(p)->tick(27,d);},
        [](void* p,double d){static_cast<Fixture*>(p)->tick(28,d);},
        [](void* p){static_cast<Fixture*>(p)->event(29);},
        [](void* p,double d){static_cast<Fixture*>(p)->tick(32,d);}};}
    void run_port(){world_test::active=&entities;auto h=hooks();auto c=entities.hooks();update_world(w,entities.world,entities.elapsed,h,c);}
    static std::uint64_t bits(double d){if(std::isnan(d))return 0x7ff8000000000000ull;std::uint64_t v;std::memcpy(&v,&d,8);return v;}
    std::vector<std::uint64_t> snapshot(){sync(true);std::vector<std::uint64_t> v={static_cast<unsigned>(w.status_00),w.value_5098,bits(w.elapsed_5120),bits(w.sequence_timer_5128),w.sequence_count_5308,w.sequence_done_50d3,w.sequence_ready_53a0,w.sequence_539c?static_cast<std::uint64_t>(static_cast<State*>(w.sequence_539c)->id):0u,
            static_cast<std::uint32_t>(w.animations_538c.count),static_cast<std::uint32_t>(w.records_5100.count),static_cast<std::uint32_t>(w.auxiliary_1fd0.count),
            w.sequence_cursor_5304==&entities.world.list_1ec8.begin?0xffffu:w.sequence_cursor_5304==&entities.world.list_1ec8.end?0xfffeu:static_cast<std::uint64_t>(w.sequence_cursor_5304-entities.nodes[1].data())};for(auto& r:records)v.push_back(bits(r.remaining_170));for(auto& e:entities.entities)v.push_back(e.port.primary_behavior_340?static_cast<State*>(e.port.primary_behavior_340)->id:0);return v;}
};
}
