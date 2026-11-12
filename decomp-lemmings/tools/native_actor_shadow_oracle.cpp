#include "lemmings/actor_shadow.hpp"
#include "lemmings/actor_shadow_segments.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

namespace {
using namespace lemmings;
using Point = std::array<double,3>;
template<class T> T get(const void* p,unsigned off) { T v;std::memcpy(&v,static_cast<const char*>(p)+off,sizeof(v));return v; }
template<class T> void put(void* p,unsigned off,const T& v) { std::memcpy(static_cast<char*>(p)+off,&v,sizeof(v)); }
std::uint64_t bits(double x) { return std::isnan(x)?0x7ff8000000000000ULL:std::bit_cast<std::uint64_t>(x); }
std::array<std::uint8_t,0x3cc> native_shadow{};
std::array<std::uint8_t,0x3a8> native_actor{};
ActorShadow* source = nullptr;
ActorShadowGlobals* source_globals = nullptr;
void* native_list = nullptr;
unsigned script = 0;
std::uint8_t query_result = 0;
bool valid = true;
bool concrete_segments = false;
CollisionGridHead* source_grid = nullptr;
std::vector<Point> points;
std::vector<std::array<std::uint64_t,10>> trace;
int token = 0;
void record(unsigned call,unsigned argument=0,double a=0,double b=0) {
    trace.push_back({call,argument,bits(a),bits(b),
        source ? source->visible_3ad : get<std::uint8_t>(native_shadow.data(),0x3ad),
        static_cast<unsigned>(source ? source->intensity_3c8 : get<int>(native_shadow.data(),0x3c8)),
        static_cast<unsigned>(source ? source->mode_120 : get<int>(native_shadow.data(),0x120)),
        bits(source ? source->position_3b0[1] : get<double>(native_shadow.data(),0x3b8)),
        bits(source ? source->actor_3a8->geometry.position[2] : get<double>(native_actor.data(),0x18)),
        bits(source ? source_globals->width_4c05c8 : get<double>(reinterpret_cast<void*>(0x4c05c8),0))});
}
void add_actor(unsigned offset,double delta) {
    if(source) {
        auto& value=source->actor_3a8->geometry.position[(offset-8)/8];
        value=static_cast<double>(static_cast<long double>(value)+delta);
    } else put(native_actor.data(),offset,static_cast<double>(static_cast<long double>(get<double>(native_actor.data(),offset))+delta));
}
void resize(int count) { record(2,count);if(script&1)add_actor(0x10,0.5);if(concrete_segments)points.resize(count); }
std::uint8_t query(int type) { record(4,type);if(script&2)add_actor(0x18,0.25);return query_result; }
const Point& point(int index) {
    record(7,index);
    if(script&8)add_actor(0x18,0.125);
    if(script&16)points[0][0]=static_cast<double>(static_cast<long double>(points[0][0])+0.03125L);
    return points.at(index);
}
void activate() { record(9);if(script&4)add_actor(0x18,0.5); }
struct SegmentList : ActorShadowSegmentList {
    void resize_421260(int count) override { resize(count); }
    Point& point_at_421240(int index) override { return const_cast<Point&>(point(index)); }
} segment_list;
struct Effects : ActorShadowCallbacks {
    void* create_point_list_4229f0() override { record(1);return &token; }
    void resize_point_list_421260(void* p,int n) override { valid&=p==&token;resize(n); }
    void destroy_point_list_422a30(void* p) override { valid&=p==&token;record(3); }
    std::uint8_t has_state_42ecc9(ShadowActor&,int type) override { return query(type); }
    void build_segments_459557(ActorShadow& shadow,void* p,double a,double b) override {
        valid&=p==&token;
        if(concrete_segments)build_actor_shadow_segments(shadow.position_3b0,a,b,*source_grid,segment_list);
        else record(5,0,a,b);
    }
    int point_count_459990(void* p) override { valid&=p==&token;record(6);return static_cast<int>(points.size()); }
    const Point& point_at_421240(void* p,int i) override { valid&=p==&token;return point(i); }
    void* lookup_resource_414a10(int id) override { record(8,id);return &token; }
    void activate_resource_47261f(void* p) override { valid&=p==&token;activate(); }
} effects;
void* __attribute__((thiscall)) create_adapter(void* list) { native_list=list;record(1);return list; }
void __attribute__((thiscall)) resize_adapter(void* list,int n) { valid&=list==native_list;resize(n); }
void __attribute__((thiscall)) destroy_adapter(void* list) { valid&=list==native_list;record(3); }
std::uint8_t __attribute__((thiscall)) query_adapter(void* actor,int type) { valid&=actor==native_actor.data();return query(type); }
void __attribute__((thiscall)) build_adapter(void* shadow,void* list,double a,double b) { valid&=shadow==native_shadow.data()&&list==native_list;record(5,0,a,b); }
int __attribute__((thiscall)) count_adapter(void* list) { valid&=list==native_list;record(6);return static_cast<int>(points.size()); }
const Point* __attribute__((thiscall)) point_adapter(void* list,int index) { valid&=list==native_list;return &point(index); }
void* __attribute__((thiscall)) lookup_adapter(void* resources,int id) { valid&=resources==reinterpret_cast<void*>(0x58cb48);record(8,id);return &token; }
void __attribute__((thiscall)) activate_adapter(void* p) { valid&=p==&token;activate(); }
void jump(unsigned address,void* target) {
    put(reinterpret_cast<void*>(address),0,std::uint8_t{0xe9});
    put(reinterpret_cast<void*>(address),1,static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-address-5));
}
void write_shadow(void* target,const ActorShadow& shadow) {
    put(target,0,shadow.vertices_000);
    put(target,0x120,shadow.mode_120);
    for(unsigned g=0;g<5;++g)for(unsigned i=0;i<4;++i)
        put(target,0x128+g*0x80+0x28+i*0x18,shadow.uv_128[g][i]);
    put(target,0x3ad,shadow.visible_3ad);put(target,0x3b0,shadow.position_3b0);put(target,0x3c8,shadow.intensity_3c8);
}
}
int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xd0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned count=0;std::cin.read(reinterpret_cast<char*>(&count),4);if(!std::cin||count>100)return 2;
    for(unsigned i=0;i<count;++i) {
        unsigned address=0,length=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&length),4);
        if(address<0x400000||length>0xd0000||address-0x400000>0xd0000-length)return 2;
        std::cin.read(reinterpret_cast<char*>(address),length);if(!std::cin)return 2;
    }
    std::array<std::uint8_t,5> original_builder_entry{};
    std::memcpy(original_builder_entry.data(),reinterpret_cast<void*>(0x459557),5);
    jump(0x4229f0,reinterpret_cast<void*>(&create_adapter));jump(0x421260,reinterpret_cast<void*>(&resize_adapter));
    jump(0x422a30,reinterpret_cast<void*>(&destroy_adapter));jump(0x42ecc9,reinterpret_cast<void*>(&query_adapter));
    jump(0x459557,reinterpret_cast<void*>(&build_adapter));jump(0x459990,reinterpret_cast<void*>(&count_adapter));
    jump(0x421240,reinterpret_cast<void*>(&point_adapter));jump(0x414a10,reinterpret_cast<void*>(&lookup_adapter));
    jump(0x47261f,reinterpret_cast<void*>(&activate_adapter));
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Update=void(__attribute__((thiscall))*)(void*);
    const auto update=reinterpret_cast<Update>(0x458186);
    std::array<std::uint8_t,0x20> primary{};
    std::array<std::int32_t,32*256> cells{};
    CollisionGridHead grid{};grid.height=32;grid.cells.data=cells.data();
    source_grid=&grid;
    std::array<WorldGeometryPrefix::TripleBlock,2> rows{};
    WorldCoordinateMesh mesh{32,rows};
    put(reinterpret_cast<void*>(0x4c6a10),0,grid);
    put(reinterpret_cast<void*>(0x4c6a10),0x24,rows.data());put(reinterpret_cast<void*>(0x4c6a10),0x28,mesh.height);
    std::mt19937_64 random(0x458186);
    unsigned cases=0,visible=0,mode1=0,mode5=0;
    for(unsigned composition=0;composition<2;++composition) {
    const auto first_case=cases,first_visible=visible,first_mode1=mode1,first_mode5=mode5;
    concrete_segments=composition!=0;
    if(concrete_segments) {
        DWORD protection=0;
        if(!VirtualProtect(reinterpret_cast<void*>(0x459557),5,PAGE_EXECUTE_READWRITE,&protection))return 2;
        std::memcpy(reinterpret_cast<void*>(0x459557),original_builder_entry.data(),5);
        DWORD ignored=0;if(!VirtualProtect(reinterpret_cast<void*>(0x459557),5,protection,&ignored))return 2;
        FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(0x459557),5);
    }
    for(unsigned trial=0;trial<12000;++trial) {
        for(auto& row:rows)for(auto& p:row)for(auto& v:p)
            v=std::bit_cast<std::uint64_t>(double(int(random()%20000)-10000)/113);
        cells.fill(0);
        ShadowActor actor;ActorShadow shadow;ActorShadowGlobals globals;
        actor.geometry.position={double(random()%2048)/8,double(random()%240)/8,0.5};
        actor.geometry.direction_40=random()&1?1:-1;
        actor.geometry.direction_44=trial%11==0?-1:1;
        actor.geometry.pair_360={actor.geometry.direction_40==-1?-1.0:0.0,0};
        actor.primary_type_14=random()%40;
        actor.gadget_54=trial%13==0?&token:nullptr;
        shadow.actor_3a8=&actor;shadow.mode_120=99;shadow.visible_3ad=0x80;shadow.intensity_3c8=-123;
        shadow.position_3b0={123,456,789};
        for(auto& p:shadow.vertices_000)p={-1,-2,-3};
        for(auto& group:shadow.uv_128)for(auto& p:group)p={7,8};
        globals.width_4c05c8=std::array<double,5>{0,0.5,2.5,5,9}[random()%5];
        globals.resource_id_4c05c0=static_cast<int>(random());
        script=random()%32;query_result=trial%17==0?0x80:0;
        // Horizontal strips keep terrain queries concrete while exercising
        // grounded, nearby-airborne and hidden-distance branches.
        const int floor=static_cast<int>(actor.geometry.position[1])-int(random()%13);
        if(floor>=0)for(unsigned x=0;x<256;++x)cells[floor*256+x]=1;
        const unsigned n=1+random()%4;
        points.clear();for(unsigned i=0;i<n;++i)
            points.push_back({double(i)*0.75+double(random()%256),double(random()%240)/8,0});
        if(trial%19==0)for(auto& p:points)p[0]=5; // Zero span is preserved.
        if(trial<64) {
            // Offset the foot to a different column so its contact query
            // stays false while the raw-position distance scan sees row0.
            const double nan=std::numeric_limits<double>::quiet_NaN();
            const double inf=std::numeric_limits<double>::infinity();
            const std::array<double,16> heights{-inf,-1,-0.0,0,0.5,1,3,9,
                std::nextafter(10.0,0.0),10,std::nextafter(10.0,inf),11,31,32,inf,nan};
            actor.geometry.position={8,heights[trial%16],0.5};
            actor.geometry.direction_40=1;actor.geometry.direction_44=1;
            actor.geometry.pair_360={1,0};actor.gadget_54=nullptr;
            script=0;query_result=0;cells.fill(0);cells[8]=1;
            globals.width_4c05c8=std::array<double,4>{0,2.5,-5,9}[trial/16];
        }
        const auto original_points=points;
        native_shadow.fill(0xa5);native_actor.fill(0xa5);
        write_shadow(native_shadow.data(),shadow);put(native_shadow.data(),0x3a8,native_actor.data());
        put(native_actor.data(),8,actor.geometry.position);put(native_actor.data(),0x40,actor.geometry.direction_40);
        put(native_actor.data(),0x44,actor.geometry.direction_44);put(native_actor.data(),0x4c,actor.geometry.category_4c);
        put(native_actor.data(),0x54,actor.gadget_54);put(native_actor.data(),0x360,actor.geometry.pair_360);
        put(primary.data(),0x14,actor.primary_type_14);put(native_actor.data(),0x340,primary.data());
        put(reinterpret_cast<void*>(0x4c05c8),0,globals.width_4c05c8);put(reinterpret_cast<void*>(0x4c05c0),0,globals.resource_id_4c05c0);
        auto expected_shadow=native_shadow;auto expected_actor=native_actor;
        source=nullptr;trace.clear();valid=true;update(native_shadow.data());
        const auto original_trace=trace;const bool original_valid=valid;
        points=original_points;trace.clear();valid=true;source=&shadow;source_globals=&globals;
        update_actor_shadow(shadow,globals,grid,mesh,effects);
        write_shadow(expected_shadow.data(),shadow);put(expected_actor.data(),8,actor.geometry.position);
        ++cases;visible+=shadow.visible_3ad!=0;mode1+=shadow.mode_120==1;mode5+=shadow.mode_120==5;
        // NaNs may differ only in payload/sign between original x87 and host
        // division. Project those UV fields before comparing the full object.
        for(unsigned g=0;g<5;++g)for(unsigned i=0;i<4;++i)for(unsigned c=0;c<2;++c) {
            const auto off=0x128+g*0x80+0x28+i*0x18+c*8;
            if(std::isnan(get<double>(native_shadow.data(),off))&&std::isnan(get<double>(expected_shadow.data(),off)))
                put(expected_shadow.data(),off,get<double>(native_shadow.data(),off));
        }
        if(!valid||!original_valid||trace!=original_trace||native_shadow!=expected_shadow||native_actor!=expected_actor||
            bits(globals.width_4c05c8)!=bits(get<double>(reinterpret_cast<void*>(0x4c05c8),0))) {
            std::cerr<<"shadow mismatch composition="<<composition<<" case="<<trial<<" list="<<n<<" script="<<script<<" trace="<<original_trace.size()<<'/'<<trace.size()<<'\n';
            for(unsigned i=0;i<trace.size()&&i<original_trace.size();++i)if(trace[i]!=original_trace[i]) {
                std::cerr<<"trace event "<<i<<" original:";for(auto v:original_trace[i])std::cerr<<' '<<std::hex<<v;
                std::cerr<<" source:";for(auto v:trace[i])std::cerr<<' '<<std::hex<<v;std::cerr<<'\n';break;
            }
            for(unsigned i=0;i<native_shadow.size();++i)if(native_shadow[i]!=expected_shadow[i]) {std::cerr<<"shadow byte "<<std::hex<<i<<'\n';break;}
            return 1;
        }
    }
    std::cout<<"native shadow "<<(concrete_segments?"composed segments":"segment fixtures")<<" passed "<<cases-first_case
             <<" traces: "<<visible-first_visible<<" visible, "<<mode1-first_mode1<<" three-point meshes, "<<mode5-first_mode5<<" four-point meshes\n";
    }
    VirtualFree(image,0,MEM_RELEASE);
}
