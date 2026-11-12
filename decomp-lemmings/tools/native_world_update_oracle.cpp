#include "../tests/world_update_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
using namespace world_test;
namespace {
using NativeUpdate=void (__attribute__((thiscall)) *)(void*,double);
std::uint8_t __attribute__((thiscall)) update(Entity* e,double dt){return active->update(*e,dt);}
void __attribute__((thiscall)) deactivate(Entity* e){active->deactivate(*e);}
void __attribute__((thiscall)) enqueue(void*,Entity* e){active->enqueue(*e);}
void __attribute__((thiscall)) unlink0(Node* node){active->unlink(0,static_cast<int>(node-active->raw_nodes[0].data()));}
void __attribute__((thiscall)) unlink1(Node* node){active->unlink(1,static_cast<int>(node-active->raw_nodes[1].data()));}
void __cdecl notice(const char*,int id,int){active->notice(id);}
void __attribute__((thiscall)) erase0(RawArray*,int index){active->erase(0,index);}
void __attribute__((thiscall)) erase2(RawArray* array,Entity** entry){active->erase(2,static_cast<int>(entry-array->data));}
void __attribute__((thiscall)) registration(void*,Entity* e){active->registration(*e);}
void jump(std::uintptr_t address,std::uintptr_t target){auto* p=reinterpret_cast<unsigned char*>(address);p[0]=0xe9;std::uint32_t delta=static_cast<std::uint32_t>(target-address-5);std::memcpy(p+1,&delta,4);}
bool load(){std::uint32_t n;std::cin.read(reinterpret_cast<char*>(&n),4);if(!std::cin||n>100)return false;while(n--){std::uint32_t a,s;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&s),4);if(a<0x400000||a+s>0x500000)return false;std::cin.read(reinterpret_cast<char*>(a),s);if(!std::cin)return false;}return true;}
}
namespace {
world_update_test::Fixture* wf=nullptr;
void receiver(void* actual,unsigned offset){if(actual!=wf->entities.raw_world.data()+offset)wf->valid=false;}
#define TICK(name,id,offset) void __attribute__((thiscall)) name(void* p,double dt){receiver(p,offset);wf->tick(id,dt);}
TICK(initial,10,0x5140) TICK(tail0,27,0x27d0) TICK(tail1,28,0x2018) TICK(tail3,32,0)
#define WORLD_OPTIONAL(name,id) void __attribute__((thiscall)) name(void* p,double dt){wf->optional(id,p,dt);}
WORLD_OPTIONAL(opt0,0x4911e9) WORLD_OPTIONAL(opt1,0x4457e0) WORLD_OPTIONAL(opt2,0x49185c) WORLD_OPTIONAL(opt3,0x49259c) WORLD_OPTIONAL(opt4,0x4457f0)
void __attribute__((thiscall)) tail2(void* p){receiver(p,8);wf->event(29);}
std::uint32_t* __attribute__((thiscall)) first_resource(void* p,int index){receiver(p,0x170);static std::uint32_t id=42;if(index!=0)wf->valid=false;wf->event(30);return &id;}
void* __attribute__((thiscall)) lookup(void* p,std::uint32_t id){if(p!=reinterpret_cast<void*>(0x58cb48))wf->valid=false;wf->event(31,id);return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(&wf->old_behavior)-0x1c);}
void* __attribute__((thiscall)) init_animation(WorldAnimationObject* a){wf->init(*a);return a;}
void __attribute__((thiscall)) config_animation(WorldAnimationObject* a,void* r,int first,int last,double dt){wf->config(*a,r,first,last,dt);}
void __attribute__((thiscall)) append_animation(void* p,WorldAnimationObject* a){receiver(p,0x538c);wf->append(*a);}
void __attribute__((thiscall)) destroy_animation(WorldAnimationObject* a){wf->destroy_animation(*a);}
void __attribute__((thiscall)) advance_animation(WorldAnimationObject* a,double dt){wf->animate(*a,dt);}
void* __cdecl allocate(unsigned size){if(size==0x50)return &wf->sequence;if(size!=0x18)wf->valid=false;return wf->entities.trial%13==1?nullptr:&wf->new_behavior;}
void* __attribute__((thiscall)) sequence_constructor(void*,int x,int y){return wf->create_sequence(x,y);}
void* __attribute__((thiscall)) behavior_constructor(void*){return wf->behavior();}
int __attribute__((thiscall)) sequence_update(void* p,void* actor,double* remaining){if(actor)wf->valid=false;return wf->sequence_update(p,*remaining);}
void __attribute__((thiscall)) sequence_leave(void* p,void* actor){if(actor)wf->valid=false;wf->leave(p);}
void* __attribute__((thiscall)) state_destructor(void* p,unsigned flags){wf->destroy(p,flags);return p;}
void __attribute__((thiscall)) sequence_position(void* p,int x,int y,unsigned flag){wf->position(p,x,y,flag);}
int __attribute__((thiscall)) query(void* p){if(p!=reinterpret_cast<void*>(0x4c6a08))wf->valid=false;return wf->query();}
void __attribute__((thiscall)) action(void* p){receiver(p,0);wf->action();}
void __attribute__((thiscall)) primary(Entity* e,void* next){wf->primary(*e,next);}
void __attribute__((thiscall)) before(void* p){receiver(p,8);wf->before();}
void __attribute__((thiscall)) auxiliary(WorldAuxiliaryRecord* a,double dt){wf->aux(*a,dt);}
}
int main(){
    static_assert(sizeof(void*)==4);
    _setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x100000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(image!=reinterpret_cast<void*>(0x400000))return 3;if(!load())return 2;
    // The complete original 4423D2 body remains unchanged. Only dependency
    // entry points are adapted; all collection and field accessors run original.
#define ADAPT(address,function) jump(address,reinterpret_cast<std::uintptr_t>(&function))
    ADAPT(0x4090b0,enqueue);ADAPT(0x445950,unlink0);ADAPT(0x408fc0,unlink1);ADAPT(0x40f860,notice);ADAPT(0x4465d0,erase0);ADAPT(0x446c70,erase2);ADAPT(0x43b536,registration);
    ADAPT(0x437bde,initial);ADAPT(0x4911e9,opt0);ADAPT(0x4457e0,opt1);ADAPT(0x49185c,opt2);ADAPT(0x49259c,opt3);ADAPT(0x4457f0,opt4);
    ADAPT(0x412e90,first_resource);ADAPT(0x414a10,lookup);ADAPT(0x447390,init_animation);ADAPT(0x47abe0,config_animation);ADAPT(0x448050,append_animation);ADAPT(0x47ac05,destroy_animation);ADAPT(0x47ac1b,advance_animation);
    ADAPT(0x49c1a8,allocate);ADAPT(0x413d40,sequence_constructor);ADAPT(0x416910,behavior_constructor);ADAPT(0x414132,sequence_position);ADAPT(0x445436,query);ADAPT(0x444a34,action);ADAPT(0x42e696,primary);
    ADAPT(0x43b419,before);ADAPT(0x41f92e,auxiliary);ADAPT(0x45af0e,tail0);ADAPT(0x44d22c,tail1);ADAPT(0x43b150,tail2);ADAPT(0x445498,tail3);
    DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    void* entity_table[6]{};entity_table[2]=reinterpret_cast<void*>(&deactivate);entity_table[5]=reinterpret_cast<void*>(&update);
    void* state_table[4]={reinterpret_cast<void*>(&state_destructor),reinterpret_cast<void*>(&sequence_update),nullptr,reinterpret_cast<void*>(&sequence_leave)};
    auto original=reinterpret_cast<NativeUpdate>(0x4423d2);
    for(int trial=0;trial<10000;++trial){world_update_test::Fixture native(trial,true,entity_table,state_table),port(trial,false);wf=&native;active=&native.entities;original(native.entities.raw_world.data(),native.entities.elapsed);port.run_port();
        if(!native.valid||!port.valid||native.trace!=port.trace||native.snapshot()!=port.snapshot()||native.entities.trace!=port.entities.trace||native.entities.snapshot()!=port.entities.snapshot()){
            std::cerr<<"failure "<<native.failure<<" "<<port.failure<<" valid "<<native.valid<<" "<<port.valid<<" collections "<<(native.entities.trace==port.entities.trace)<<" states "<<(native.entities.snapshot()==port.entities.snapshot())<<" world "<<(native.snapshot()==port.snapshot())<<" traces "<<(native.trace==port.trace)<<'\n';
            std::cerr<<"world update mismatch trial "<<trial<<" native trace "<<native.trace.size()<<" port "<<port.trace.size()<<'\n';
            for(std::size_t i=0;i<native.trace.size()&&i<port.trace.size();++i)if(native.trace[i]!=port.trace[i]){std::cerr<<"trace "<<i<<" "<<native.trace[i]<<" "<<port.trace[i]<<'\n';break;}
            auto a=native.snapshot(),b=port.snapshot();for(std::size_t i=0;i<a.size();++i)if(a[i]!=b[i]){std::cerr<<"field "<<i<<" "<<a[i]<<" "<<b[i]<<'\n';break;}return 1;}}
    VirtualFree(image,0,MEM_RELEASE);std::cout<<"native complete world update passed 10000 trace/state comparisons\n";
}
