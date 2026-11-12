#include "../tests/actor_forced_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <random>
#include <limits>
namespace {
using forced_test::Triple;
forced_test::Trace trace;
std::array<void*,22> table{};
bool valid=true;
void* __cdecl alloc(std::uint32_t n){
    if(n==0x148)return trace.effect.data();
    if(n==0x48)return trace.null_entity?nullptr:trace.entity.data();
    valid=false;return nullptr;
}
long double __cdecl random_adapter(){return trace.random_404080();}
void* __attribute__((thiscall)) construct(void* self){valid &= self==trace.create_effect_416ed0();auto p=table.data();std::memcpy(self,&p,4);return self;}
void __attribute__((thiscall)) count(void* self,int n){trace.set_count_417150(self,n);}
Triple* __attribute__((thiscall)) world(void*,Triple* result,const Triple* p){*result=trace.world_position_43d56c(*p);return result;}
void __attribute__((thiscall)) direction(void* self,const Triple* p){trace.set_direction_54(self,*p);}
void __attribute__((thiscall)) position(void* self,const Triple* p){trace.set_position_08(self,*p);}
void __attribute__((thiscall)) pair(void* self,double a,double b){trace.set_pair_4171f0(self,a,b);}
void __attribute__((thiscall)) mode(void* self,int n){trace.set_mode_417230(self,n);}
void __attribute__((thiscall)) advance(void* self,double d){trace.advance_effect_2c(self,d);}
Triple* __attribute__((thiscall)) sample(void*,std::uint32_t x,int y){static Triple p;p=trace.sample_triple_403f40(x,y);return &p;}
void* __attribute__((thiscall)) child(void* self,int n){return trace.child_4018b0(static_cast<unsigned char*>(self)-0xb0,n);}
void __attribute__((thiscall)) child_triple(void* self,const Triple* p){trace.set_child_triple_403820(static_cast<unsigned char*>(self)-0xb0,*p);}
void* __attribute__((thiscall)) entity(void* self,const Triple* p,void* e,std::uint32_t f){valid &= self==trace.create_entity_432080(*p,e,static_cast<std::uint8_t>(f));return self;}
void __attribute__((thiscall)) insert(void*,void** p){trace.insert_entity_401890(*p);}
void jump(std::uintptr_t a,void* f){auto p=reinterpret_cast<unsigned char*>(a);p[0]=0xe9;auto d=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(f)-a-5);std::memcpy(p+1,&d,4);}
bool load(){std::uint32_t count;std::cin.read(reinterpret_cast<char*>(&count),4);for(unsigned i=0;i<count;++i){std::uint32_t a,n;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);if(a<0x400000||a>0x4f0000||n>0x4f0000-a)return false;std::cin.read(reinterpret_cast<char*>(a),n);}return bool(std::cin);}
template<class T,class B>void put(B& b,unsigned n,T v){std::memcpy(b.data()+n,&v,sizeof v);}
template<class T,class B>T get(const B& b,unsigned n){T v;std::memcpy(&v,b.data()+n,sizeof v);return v;}
}
int main(){
    _setmode(_fileno(stdin),_O_BINARY);
    auto image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xf0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;if(!load())return 2;
    std::array<unsigned char,18> original_random;
    std::memcpy(original_random.data(),reinterpret_cast<void*>(0x404080),18);
    jump(0x49c1a8,reinterpret_cast<void*>(&alloc));jump(0x404080,reinterpret_cast<void*>(&random_adapter));
    jump(0x416ed0,reinterpret_cast<void*>(&construct));jump(0x417150,reinterpret_cast<void*>(&count));
    jump(0x43d56c,reinterpret_cast<void*>(&world));jump(0x4171f0,reinterpret_cast<void*>(&pair));
    jump(0x417230,reinterpret_cast<void*>(&mode));jump(0x403f40,reinterpret_cast<void*>(&sample));
    jump(0x4018b0,reinterpret_cast<void*>(&child));jump(0x403820,reinterpret_cast<void*>(&child_triple));
    jump(0x432080,reinterpret_cast<void*>(&entity));jump(0x401890,reinterpret_cast<void*>(&insert));
    table[2]=reinterpret_cast<void*>(&position);table[11]=reinterpret_cast<void*>(&advance);table[21]=reinterpret_cast<void*>(&direction);
    DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Update=std::uint32_t(__attribute__((thiscall))*)(void*,void*,double*);
    using Constructor=void*(__attribute__((thiscall))*)(void*);
    using Empty=void(__attribute__((thiscall))*)(void*,void*);
    std::mt19937_64 rng(0x416c49);
    const double edges[]={0.,-0.,1.,-1.,127.999,-128.001,2147483648.,-4294967297.,9223372036854775808.,
        -9223372036854775808.,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity()};
    for(unsigned trial=0;trial<40000;++trial){
        if(trial==20000){
            DWORD ignored;VirtualProtect(reinterpret_cast<void*>(0x404080),18,PAGE_EXECUTE_READWRITE,&ignored);
            std::memcpy(reinterpret_cast<void*>(0x404080),original_random.data(),18);
            VirtualProtect(reinterpret_cast<void*>(0x404080),18,PAGE_EXECUTE_READ,&ignored);
            FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(0x404080),18);
            trace.concrete_random=true;
        }
        lemmings::TerminalActor actor;
        for(unsigned i=0;i<3;++i)actor.position_08[i]=trial<39?edges[(trial+i)%13]:static_cast<double>(static_cast<int>(rng()%100000)-50000)/32;
        actor.category_4c=static_cast<std::int32_t>(rng());double dt=trial<13?edges[trial]:double(rng()%4096)/8192;
        alignas(8) std::array<unsigned char,0x3a0> raw;raw.fill(0xa5);put(raw,8,actor.position_08);put(raw,0x4c,actor.category_4c);
        auto before=raw;double native_dt=dt;
        trace.mutate=trial%16;trace.null_entity=trial%7==0;
        const auto random_seed=static_cast<std::uint32_t>(rng());
        trace.random_state.value=random_seed;
        std::memcpy(reinterpret_cast<void*>(0x4c4570),&random_seed,4);
        trace.randoms={static_cast<long double>(static_cast<int>(rng()%65535)-32767)/32767.0L,
                      static_cast<long double>(static_cast<int>(rng()%65535)-32767)/32767.0L};
        trace.position=&actor.position_08;trace.category=&actor.category_4c;trace.remaining=&dt;trace.reset();
        auto expected_result=lemmings::update_actor_forced(actor,dt,trace);auto expected=trace.events;
        const auto expected_random_seed=trace.random_state.value;
        auto expected_effect_category=get<std::int32_t>(trace.effect,0xd8);
        trace.reset();trace.position=raw.data()+8;trace.category=raw.data()+0x4c;trace.remaining=&native_dt;
        alignas(8) std::array<unsigned char,0x18> state;state.fill(0xa5);
        reinterpret_cast<Constructor>(0x416bf0)(state.data());
        if(get<double>(state,8)!=0||state[0x10]!=0||state[0x11]!=0||get<std::int32_t>(state,0x14)!=0x25)return 1;
        const auto state_before=state;
        reinterpret_cast<Empty>(0x416c3c)(state.data(),raw.data());
        auto got=reinterpret_cast<Update>(0x416c49)(state.data(),raw.data(),&native_dt);
        reinterpret_cast<Empty>(0x401320)(state.data(),raw.data());
        put(before,8,actor.position_08);put(before,0x4c,actor.category_4c);
        if(!valid||got!=expected_result||trace.events!=expected||raw!=before||state!=state_before||
           (trace.concrete_random&&*reinterpret_cast<std::uint32_t*>(0x4c4570)!=expected_random_seed)||
           std::bit_cast<std::uint64_t>(dt)!=std::bit_cast<std::uint64_t>(native_dt)||get<std::int32_t>(trace.effect,0xd8)!=expected_effect_category){
            std::cerr<<"forced mismatch "<<trial<<" events "<<trace.events.size()<<'/'<<expected.size()<<'\n';
            for(unsigned i=0;i<std::min(trace.events.size(),expected.size());++i)if(trace.events[i]!=expected[i]){std::cerr<<"first "<<i<<' '<<trace.events[i]<<' '<<expected[i]<<'\n';break;}return 1;
        }
    }
    std::cout<<"40000 forced traces match original core (20000 with original PRNG chain)\n";
}
