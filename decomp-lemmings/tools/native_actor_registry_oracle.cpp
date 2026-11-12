#include "lemmings/actor_registry.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <array>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>
#include <new>
namespace {
using lemmings::ActorRegistry;
static_assert(sizeof(ActorRegistry)==16&&offsetof(ActorRegistry,data)==12);
struct Context{
    ActorRegistry* array=nullptr;void** input=nullptr;unsigned mutation=0;
    std::array<void*,64> old{},alternate{},replacement{};
    std::vector<std::uint32_t> events;
    unsigned identity(void** p){return p==old.data()?1:p==alternate.data()?2:p==replacement.data()?3:0;}
    void snapshot(){events.push_back(array->count);events.push_back(array->capacity);events.push_back(array->growth);events.push_back(identity(array->data));}
    void** allocate(std::uint32_t n){events.push_back(n);snapshot();
        if(mutation&1){array->count=std::max(0,array->count-1);array->data=alternate.data();*input=reinterpret_cast<void*>(0x5678);}
        return replacement.data();}
    void release(void** p){events.push_back(identity(p));snapshot();
        if(mutation&2){array->count=std::min(32,array->count+1);array->data=alternate.data();array->growth=13;*input=reinterpret_cast<void*>(0x9876);}}
}context;
void* __cdecl alloc(std::uint32_t n){return context.allocate(n);}
void __cdecl release(void* p){context.release(static_cast<void**>(p));}
void jump(std::uintptr_t a,void* f){auto p=reinterpret_cast<unsigned char*>(a);p[0]=0xe9;auto d=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(f)-a-5);std::memcpy(p+1,&d,4);}
bool load(){std::uint32_t c;std::cin.read(reinterpret_cast<char*>(&c),4);for(unsigned i=0;i<c;++i){std::uint32_t a,n;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);if(a<0x400000||a>0x4f0000||n>0x4f0000-a)return false;std::cin.read(reinterpret_cast<char*>(a),n);}return bool(std::cin);}
}
int main(){
    _setmode(_fileno(stdin),_O_BINARY);auto image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xf0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(image!=reinterpret_cast<void*>(0x400000))return 3;if(!load())return 2;
    jump(0x49c1a8,reinterpret_cast<void*>(&alloc));jump(0x49c13d,reinterpret_cast<void*>(&release));
    DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Append=void(__attribute__((thiscall))*)(void*,void**);using Register=void(__attribute__((thiscall))*)(void*,void*);using Erase=void(__attribute__((thiscall))*)(void*,int);
    alignas(ActorRegistry) std::array<unsigned char,0x1e98> world{};
    auto* a=new(world.data()+0x1e88) ActorRegistry;
    lemmings::ActorRegistryAllocation hooks{nullptr,[](void*,std::uint32_t n){return context.allocate(n);},[](void*,void** p){context.release(p);}};
    std::mt19937 rng(0x4233a0);
    for(unsigned trial=0;trial<20000;++trial){
        const auto count=static_cast<int>(rng()%31),growth=static_cast<int>(rng()%16+1);const bool grow=trial%2;
        const bool wrapper=trial%3==0;const auto input=reinterpret_cast<void*>(static_cast<std::uintptr_t>(rng()%12+1));
        Context initial;initial.mutation=trial%4;
        for(unsigned i=0;i<64;++i){initial.old[i]=reinterpret_cast<void*>(static_cast<std::uintptr_t>(rng()%12+1));initial.alternate[i]=reinterpret_cast<void*>(static_cast<std::uintptr_t>(rng()%12+1));initial.replacement[i]=reinterpret_cast<void*>(0xdeadbeef);}
        auto setup=[&](void*& value){context=initial;context.array=a;context.input=&value;*a={count,grow?count:48,growth,context.old.data()};};
        void* value=input;setup(value);
        if(wrapper)lemmings::register_world_actor(*a,value,hooks);else lemmings::append_actor_registry(*a,value,hooks);
        auto expected_events=context.events;auto expected_old=context.old;auto expected_alternate=context.alternate;auto expected_replacement=context.replacement;
        auto expected_header=*a;auto expected_data=context.identity(a->data);auto expected_value=value;
        value=input;setup(value);
        if(wrapper)reinterpret_cast<Register>(0x43b517)(world.data(),value);else reinterpret_cast<Append>(0x4233a0)(a,&value);
        if(context.events!=expected_events||context.old!=expected_old||context.alternate!=expected_alternate||context.replacement!=expected_replacement||
            a->count!=expected_header.count||a->capacity!=expected_header.capacity||a->growth!=expected_header.growth||context.identity(a->data)!=expected_data||value!=expected_value){std::cerr<<"append mismatch "<<trial<<'\n';return 1;}
        // Search/removal includes missing values, duplicates, null and negative counts.
        auto* data=a->data;auto original_header=*a;std::array<void*,64> original_data;std::copy_n(data,64,original_data.begin());
        const auto target=trial%5==0?nullptr:input;
        if(trial%101==0)a->count=-1;original_header=*a;
        lemmings::unregister_world_actor(*a,target);expected_header=*a;std::array<void*,64> removed;std::copy_n(data,64,removed.begin());
        *a=original_header;std::copy(original_data.begin(),original_data.end(),data);reinterpret_cast<Register>(0x43b536)(world.data(),target);
        if(a->count!=expected_header.count||!std::equal(removed.begin(),removed.end(),data)){std::cerr<<"remove mismatch "<<trial<<'\n';return 1;}
        if(a->count>0){const auto index=static_cast<int>(rng()%a->count);original_header=*a;std::copy_n(data,64,original_data.begin());lemmings::erase_actor_registry(*a,index);expected_header=*a;std::copy_n(data,64,removed.begin());*a=original_header;std::copy(original_data.begin(),original_data.end(),data);reinterpret_cast<Erase>(0x43bb90)(a,index);if(a->count!=expected_header.count||!std::equal(removed.begin(),removed.end(),data))return 1;}
    }
    std::cout<<"20000 registry append/wrapper/search/removal traces match original\n";
}
