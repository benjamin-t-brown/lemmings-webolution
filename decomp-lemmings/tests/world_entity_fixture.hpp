#pragma once
#include "lemmings/world_entity_pass.hpp"
#include <array>
#include <cassert>
#include <cstring>
#include <limits>
#include <random>
#include <vector>

namespace world_test {
using namespace lemmings;
struct Entity {
    std::array<unsigned char,0x348> raw{};
    WorldUpdateEntity port;
    int id=0;
    unsigned char result=0;
};
struct Node {Entity* entity; Node* next; Node* previous;};
struct RawArray {int count; int unknown[2]; Entity** data;};
struct Fixture;
inline Fixture* active=nullptr;
inline std::uint8_t port_update(WorldUpdateEntity&,double);
inline void port_deactivate(WorldUpdateEntity&);
inline const WorldEntityOperations operations{port_update,port_deactivate};
inline constexpr int offsets[9]={0x1eb8,0x1ef0,0x1f00,0x1f50,0x1f60,0x1f70,0x1f90,0x1f20,0x1fa0};
struct Fixture {
    bool native;
    bool elapsed_matches=true;
    int trial, mode, last_collection=0, last_index=0;
    double elapsed;
    WorldEntityPass world;
    alignas(void*) std::array<unsigned char,0x5400> raw_world{};
    std::array<Entity,176> entities;
    std::array<std::array<WorldUpdateEntity*,6>,9> slots{},alternate{};
    std::array<std::array<Entity*,6>,9> raw_slots{},raw_alternate{};
    std::array<std::array<WorldEntityNode,6>,2> nodes{};
    std::array<std::array<Node,6>,2> raw_nodes{};
    std::vector<std::uint64_t> trace;
    void* observer_context=nullptr;
    void (*observer)(void*,std::uint64_t)=nullptr;
    std::array<bool,9> grown{};
    static int collection(int array) {return array==0?1:array+2;}
    static int array_index(int c) {return c==1?0:c-2;}
    RawArray& raw_array(int a) {return *reinterpret_cast<RawArray*>(raw_world.data()+offsets[a]);}
    Node* raw_end(int list) {return reinterpret_cast<Node*>(raw_world.data()+(list?0x1ec8:0x1ea0)+12);}
    WorldEntityList& list(int l) {return l?world.list_1ec8:world.list_1ea0;}
    int count(int a) {return native?raw_array(a).count:world.arrays[a].count;}
    void set_count(int a,int n) {if(native)raw_array(a).count=n;else world.arrays[a].count=n;}
    Entity& entity(int c,int i) {return entities[c*16+i];}
    Entity* get(int c,int i) {
        if(c==0||c==2) {int l=c/2;return native?raw_nodes[l][i].entity:static_cast<Entity*>(nodes[l][i].entity->context);}
        int a=array_index(c);return native?raw_array(a).data[i]:static_cast<Entity*>(world.arrays[a].data[i]->context);
    }
    void set(int c,int i,Entity* e) {
        if(c==0||c==2){int l=c/2;if(native)raw_nodes[l][i].entity=e;else nodes[l][i].entity=&e->port;}
        else {int a=array_index(c);if(native)raw_array(a).data[i]=e;else world.arrays[a].data[i]=&e->port;}
    }
    Fixture(int t,bool n,void** vtable=nullptr):native(n),trial(t),mode(t%8) {
        double times[]={0.0,-0.0,-2.0,0.125,std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()};
        elapsed=times[t%7];std::mt19937 random(0x4423d2u+static_cast<unsigned>(t));
        for(int id=0;id<176;++id){auto& e=entities[id];e.id=id;e.port={&operations,&e,static_cast<std::uint8_t>(random()%3==0?0:random()|1),static_cast<std::int16_t>(id-90)};e.result=static_cast<unsigned char>(random()%3==0?0:random()|1);std::memcpy(e.raw.data(),&vtable,sizeof(vtable));e.raw[0x38]=e.port.enabled_38;std::memcpy(e.raw.data()+0x3a,&e.port.identifier_3a,2);}
        for(int a=0;a<9;++a){int c=collection(a);for(int i=0;i<6;++i){slots[a][i]=&entity(c,i).port;raw_slots[a][i]=&entity(c,i);}int count=t<7?t-1:static_cast<int>(random()%6);if(mode==6)count=3;world.arrays[a]={count,slots[a].data()};if(native)raw_array(a)={count,{0,0},raw_slots[a].data()};}
        for(int l=0;l<2;++l){int ncount=t<7?(t==0?0:t-1):static_cast<int>(random()%6);auto& head=list(l);head.begin.next=ncount?&nodes[l][0]:&head.end;head.end.previous=ncount?&nodes[l][ncount-1]:&head.begin;
            if(native){Node* first=ncount?&raw_nodes[l][0]:raw_end(l);std::memcpy(raw_world.data()+(l?0x1ec8:0x1ea0)+4,&first,sizeof(first));}
            for(int i=0;i<ncount;++i){nodes[l][i]={&entity(l*2,i).port,i+1<ncount?&nodes[l][i+1]:&head.end,i?&nodes[l][i-1]:&head.begin};raw_nodes[l][i]={&entity(l*2,i),i+1<ncount?&raw_nodes[l][i+1]:raw_end(l),i?&raw_nodes[l][i-1]:nullptr};}
        }
    }
    void event(unsigned kind,int id) {auto value=(std::uint64_t(kind)<<32)|static_cast<std::uint32_t>(id);trace.push_back(value);if(observer)observer(observer_context,value);}
    unsigned char update(Entity& e,double dt) {
        std::uint64_t bits,expected;std::memcpy(&bits,&dt,8);std::memcpy(&expected,&elapsed,8);if(bits!=expected)elapsed_matches=false;event(1,e.id);
        last_collection=e.id/16;last_index=0; while(last_index<6 && get(last_collection,last_index)!=&e)++last_index;assert(last_index<6);
        if(mode==1 && e.id%16<6)set(last_collection,last_index,&entity(last_collection,e.id%16+8));
        if(mode==4&&(last_collection==0||last_collection==2)){int l=last_collection/2;if(native)raw_nodes[l][last_index].next=raw_end(l);else nodes[l][last_index].next=&list(l).end;}
        if(mode==5&&last_collection!=0&&last_collection!=2)set_count(array_index(last_collection),last_index+1);
        if(mode==6&&last_index==0&&last_collection!=0&&last_collection!=2){int a=array_index(last_collection);if(!grown[a]){grown[a]=true;set_count(a,4);}}
        return e.result;
    }
    void enqueue(Entity& e){event(2,e.id);if(mode==2)set(last_collection,last_index,&entity(last_collection,9));
        if(mode==7&&last_collection==4){int a=2;if(native){for(int i=0;i<6;++i)raw_alternate[a][i]=raw_array(a).data[i];raw_array(a).data=raw_alternate[a].data();}else{for(int i=0;i<6;++i)alternate[a][i]=world.arrays[a].data[i];world.arrays[a].data=alternate[a].data();}}
    }
    void unlink(int l,int i){event(3,l*16+i);}
    void notice(int id){event(4,id);}
    void erase(int a,int i){event(a==0?5:6,i);assert(i>=0&&i<count(a));for(int j=i;j<count(a)-1;++j)set(collection(a),j,get(collection(a),j+1));set_count(a,count(a)-1);}
    void deactivate(Entity& e){event(7,e.id);if(mode==3)set(last_collection,last_index,&entity(last_collection,9));}
    void registration(Entity& e){event(8,e.id);}
    std::vector<int> snapshot(){std::vector<int> out;for(int a=0;a<9;++a){out.push_back(count(a));for(int i=0;i<count(a);++i)out.push_back(get(collection(a),i)->id);}return out;}
    WorldEntityPassHooks hooks();
    void run_port();
};
inline std::uint8_t port_update(WorldUpdateEntity& e,double dt){return active->update(*static_cast<Entity*>(e.context),dt);}
inline void port_deactivate(WorldUpdateEntity& e){active->deactivate(*static_cast<Entity*>(e.context));}
inline WorldEntityPassHooks Fixture::hooks(){return WorldEntityPassHooks{this,
    [](void* p,WorldUpdateEntity& e){static_cast<Fixture*>(p)->enqueue(*static_cast<Entity*>(e.context));},
    [](void* p,WorldEntityNode& node,bool actors){auto& f=*static_cast<Fixture*>(p);int l=actors?1:0;f.unlink(l,static_cast<int>(&node-f.nodes[l].data()));},
    [](void* p,std::int16_t id){static_cast<Fixture*>(p)->notice(id);},
    [](void* p,std::int32_t i){static_cast<Fixture*>(p)->erase(0,i);},
    [](void* p,WorldUpdateEntity** entry){auto& f=*static_cast<Fixture*>(p);f.erase(2,static_cast<int>(entry-f.world.arrays[2].data));},
    [](void* p,WorldUpdateEntity& e){static_cast<Fixture*>(p)->registration(*static_cast<Entity*>(e.context));}};
}
inline void Fixture::run_port(){active=this;auto callbacks=hooks();
    update_world_entity_collections(world,elapsed,callbacks);
}
} // namespace world_test
