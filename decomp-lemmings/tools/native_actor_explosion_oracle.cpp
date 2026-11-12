#include "../tests/actor_explosion_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <random>
#include <limits>
namespace {
using explosion_test::Triple;
explosion_test::Trace t;
std::array<void*,22> table{};bool valid=true;
void* __cdecl allocation(std::uint32_t n){if(n==0x128)return t.effect.data();if(n==0x48)return t.null_entity?nullptr:t.entity.data();valid=false;return nullptr;}
void* construct(void* self,lemmings::TerminalEffectKind k){valid &= self==t.create_effect(k);auto v=table.data();std::memcpy(self,&v,4);return self;}
void* __attribute__((thiscall)) first(void* self){return construct(self,lemmings::TerminalEffectKind::effect_4018e0);}
void* __attribute__((thiscall)) alternate(void* self){return construct(self,lemmings::TerminalEffectKind::effect_4072a0);}
void __attribute__((thiscall)) event(void*,std::int32_t id,std::int32_t e,std::uint32_t f){lemmings::TerminalActor a;t.event_44535b(a,static_cast<std::int16_t>(id),e,static_cast<std::uint8_t>(f));}
void __cdecl nearby(Triple* p,double r){t.nearby_effect_41dd86(*p,r);}
void __attribute__((thiscall)) notice(void*,std::int32_t n,Triple* p){t.world_notice_43b31e(n,*p);}
void __cdecl blast(Triple* p,std::int32_t n,double d){t.terrain_blast_43f510(*p,n,d);}
void __attribute__((thiscall)) count(void* self,int n){t.set_count(self,n);}
void __attribute__((thiscall)) position(void* self,const Triple* p){t.set_position_08(self,*p);}
void __attribute__((thiscall)) pair(void* self,double a,double b){t.set_pair(self,a,b);}
void __attribute__((thiscall)) mode(void* self,int n){t.set_mode(self,n);}
Triple* __attribute__((thiscall)) world(void*,Triple* out,const Triple* p){*out=t.world_position_43d56c(*p);return out;}
void __attribute__((thiscall)) direction(void* self,const Triple* p){t.set_direction_54(self,*p);}
void __attribute__((thiscall)) advance(void* self,double d){t.advance_effect_2c(self,d);}
Triple* __attribute__((thiscall)) sample(void*,std::uint32_t x,int y){static Triple p;p=t.sample_triple_403f40(x,y);return &p;}
int __attribute__((thiscall)) child_count(void* self){return t.child_count_4018d0(static_cast<unsigned char*>(self)-0xb0);}
void* __attribute__((thiscall)) child(void* self,int n){return t.child_4018b0(static_cast<unsigned char*>(self)-0xb0,n);}
void __attribute__((thiscall)) child_triple(void* self,const Triple* p){t.set_child_triple_403820(static_cast<unsigned char*>(self)-0xb0,*p);}
void* __attribute__((thiscall)) entity(void* self,const Triple* p,void* e,std::uint32_t f){valid &= self==t.create_entity_432080(*p,e,static_cast<std::uint8_t>(f));return self;}
void __attribute__((thiscall)) insert(void*,void** p){t.insert_entity_401890(*p);}
void jump(std::uintptr_t a,void* f){auto p=reinterpret_cast<unsigned char*>(a);p[0]=0xe9;auto d=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(f)-a-5);std::memcpy(p+1,&d,4);}
bool load(){std::uint32_t c;std::cin.read(reinterpret_cast<char*>(&c),4);for(unsigned i=0;i<c;++i){std::uint32_t a,n;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);if(a<0x400000||a>0x4f0000||n>0x4f0000-a)return false;std::cin.read(reinterpret_cast<char*>(a),n);}return bool(std::cin);}
template<class T,class B>void put(B& b,unsigned n,T v){std::memcpy(b.data()+n,&v,sizeof v);}
template<class T,class B>T get(B& b,unsigned n){T v;std::memcpy(&v,b.data()+n,sizeof v);return v;}
}
int main(){
    _setmode(_fileno(stdin),_O_BINARY);auto image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xf0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(image!=reinterpret_cast<void*>(0x400000))return 3;if(!load())return 2;
    jump(0x49c1a8,reinterpret_cast<void*>(&allocation));jump(0x4018e0,reinterpret_cast<void*>(&first));jump(0x4072a0,reinterpret_cast<void*>(&alternate));jump(0x44535b,reinterpret_cast<void*>(&event));
    jump(0x41dd86,reinterpret_cast<void*>(&nearby));jump(0x43b31e,reinterpret_cast<void*>(&notice));jump(0x43f510,reinterpret_cast<void*>(&blast));
    for(auto a:{0x401bb0,0x407560})jump(a,reinterpret_cast<void*>(&count));for(auto a:{0x401ef0,0x4076d0})jump(a,reinterpret_cast<void*>(&pair));for(auto a:{0x401f50,0x407730})jump(a,reinterpret_cast<void*>(&mode));
    jump(0x43d56c,reinterpret_cast<void*>(&world));jump(0x403f40,reinterpret_cast<void*>(&sample));jump(0x4018d0,reinterpret_cast<void*>(&child_count));jump(0x4018b0,reinterpret_cast<void*>(&child));jump(0x403820,reinterpret_cast<void*>(&child_triple));jump(0x432080,reinterpret_cast<void*>(&entity));jump(0x401890,reinterpret_cast<void*>(&insert));
    table[2]=reinterpret_cast<void*>(&position);table[11]=reinterpret_cast<void*>(&advance);table[21]=reinterpret_cast<void*>(&direction);
    DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Constructor=void*(__attribute__((thiscall))*)(void*);using Enter=void(__attribute__((thiscall))*)(void*,void*);using Update=std::uint32_t(__attribute__((thiscall))*)(void*,void*,double*);
    std::mt19937_64 rng(0x408227);const double edges[]={0.,-0.,-1.,128.5,2147483648.,-4294967297.,9223372036854775808.,-9223372036854775808.,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity()};
    for(unsigned trial=0;trial<20000;++trial){
        lemmings::ExplosionActor a;for(unsigned i=0;i<3;++i)a.position_08[i]=trial<33?edges[(trial+i)%11]:double(static_cast<int>(rng()%32000)-16000)/32;
        a.category_4c=static_cast<int>(rng()%5);a.identifier_3a=static_cast<std::int16_t>(rng());a.pair_360={double(static_cast<int>(rng()%17)-8)/4,double(static_cast<int>(rng()%17)-8)/4};
        double dt=trial<11?edges[trial]:double(rng()%1024)/2048,native_dt=dt;
        lemmings::ActorTerminalState port_state{0};
        const unsigned alias_kind=trial%5,alias_axis=(trial/5)%3;
        double* port_remaining=alias_kind==0?&a.position_08[alias_axis]:alias_kind==1?&port_state.elapsed_08:alias_kind==2?&a.pair_360[alias_axis%2]:&dt;
        alignas(8) std::array<unsigned char,0x370> raw;raw.fill(0xa5);put(raw,8,a.position_08);put(raw,0x3a,a.identifier_3a);put(raw,0x4c,a.category_4c);put(raw,0x360,a.pair_360);auto expected_raw=raw;
        auto setup=[&](){t.events.clear();t.child_total=trial%10==0?-1:trial%8;t.mutation=trial%19;t.null_entity=trial%7==0;};
        setup();t.position=&a.position_08;t.category=&a.category_4c;t.remaining=port_remaining;
        const std::int32_t event_value=static_cast<std::int32_t>(rng());std::memcpy(reinterpret_cast<void*>(0x4e32c0),&event_value,4);
        lemmings::enter_actor_explosion(a,event_value,t);const auto expected_result=lemmings::update_actor_explosion(a,*port_remaining,t);auto expected_events=t.events;
        put(expected_raw,8,a.position_08);put(expected_raw,0x4c,a.category_4c);put(expected_raw,0x360,a.pair_360);
        setup();t.position=raw.data()+8;t.category=raw.data()+0x4c;
        alignas(8) std::array<unsigned char,0x18> state;state.fill(0xa5);reinterpret_cast<Constructor>(0x4081b0)(state.data());
        if(get<double>(state,8)!=0||state[0x10]!=0||state[0x11]!=0||get<int>(state,0x14)!=2)return 1;const auto state_before=state;
        double* native_remaining=alias_kind==0?reinterpret_cast<double*>(raw.data()+8+alias_axis*8):alias_kind==1?reinterpret_cast<double*>(state.data()+8):alias_kind==2?reinterpret_cast<double*>(raw.data()+0x360+(alias_axis%2)*8):&native_dt;
        t.remaining=native_remaining;
        reinterpret_cast<Enter>(0x4081fc)(state.data(),raw.data());const auto got=reinterpret_cast<Update>(0x408227)(state.data(),raw.data(),native_remaining);reinterpret_cast<Enter>(0x401320)(state.data(),raw.data());
        if(!valid||got!=expected_result||t.events!=expected_events||raw!=expected_raw||state!=state_before||std::bit_cast<std::uint64_t>(*port_remaining)!=std::bit_cast<std::uint64_t>(*native_remaining)){
            std::cerr<<"explosion mismatch "<<trial<<" events "<<t.events.size()<<'/'<<expected_events.size()<<'\n';for(unsigned i=0;i<std::min(t.events.size(),expected_events.size());++i)if(t.events[i]!=expected_events[i]){std::cerr<<"first "<<i<<' '<<t.events[i]<<' '<<expected_events[i]<<'\n';break;}return 1;
        }
    }
    std::cout<<"20000 explosion constructor/entry/update traces match original core\n";
}
