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
using Point=std::array<double,3>;
template<class T>void put(void* p,unsigned offset,const T& value){std::memcpy(static_cast<char*>(p)+offset,&value,sizeof(value));}
template<class T>T get(const void* p,unsigned offset){T v;std::memcpy(&v,static_cast<const char*>(p)+offset,sizeof(v));return v;}
std::uint64_t bits(double x){return std::isnan(x)?0x7ff8000000000000ULL:std::bit_cast<std::uint64_t>(x);}
std::array<std::uint64_t,3> point_bits(const Point& p){return {bits(p[0]),bits(p[1]),bits(p[2])};}
std::array<std::uint8_t,0x3c8> raw_shadow{};
Point* source_position=nullptr;
std::vector<Point> points;
std::array<std::int32_t,16*256> cells{};
unsigned script=0;
bool valid=true;
struct Event {
    int kind,argument;
    std::array<std::uint64_t,3> position;
    std::vector<std::array<std::uint64_t,3>> entries;
    bool operator==(const Event&)const=default;
};
std::vector<Event> trace;
void record(int kind,int argument){
    Event e{kind,argument,point_bits(source_position?*source_position:get<Point>(raw_shadow.data(),0x3b0)),{}};
    for(const auto& p:points)e.entries.push_back(point_bits(p));
    trace.push_back(std::move(e));
}
void mutate_position(unsigned component,double delta){
    if(source_position)(*source_position)[component]=static_cast<double>(static_cast<long double>((*source_position)[component])+delta);
    else put(raw_shadow.data(),0x3b0+component*8,static_cast<double>(static_cast<long double>(get<double>(raw_shadow.data(),0x3b0+component*8))+delta));
}
void resize(int count){
    record(1,count);
    if(script&1)mutate_position(0,7);
    if(script&2)for(auto& value:cells)value^=1;
    points.resize(count);
}
Point& at(int index){
    record(2,index);
    if(script&4){mutate_position(1,-0.25);for(auto& value:cells)value^=0x80;}
    return points.at(index);
}
struct List:ActorShadowSegmentList {
    void resize_421260(int count)override{resize(count);}
    Point& point_at_421240(int index)override{return at(index);}
} list;
int token=0;
void __attribute__((thiscall)) resize_adapter(void* p,int count){valid&=p==&token;resize(count);}
Point* __attribute__((thiscall)) at_adapter(void* p,int index){valid&=p==&token;return &at(index);}
void jump(unsigned address,void* target){put(reinterpret_cast<void*>(address),0,std::uint8_t{0xe9});put(reinterpret_cast<void*>(address),1,static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-address-5));}
}
int main(){
    _setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xd0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);if(!std::cin||blocks>64)return 2;
    for(unsigned i=0;i<blocks;++i){
        unsigned address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0xd0000||address-0x400000>0xd0000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);if(!std::cin)return 2;
    }
    jump(0x421260,reinterpret_cast<void*>(&resize_adapter));jump(0x421240,reinterpret_cast<void*>(&at_adapter));
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Build=void(__attribute__((thiscall))*)(void*,void*,double,double);
    const auto build=reinterpret_cast<Build>(0x459557);
    CollisionGridHead grid{};grid.height=16;grid.cells.data=cells.data();
    put(reinterpret_cast<void*>(0x4c6a10),0,grid);
    unsigned cases=0,three=0,four=0,empty=0;
    auto compare=[&](Point position,double left,double right){
        const auto initial_cells=cells;
        points={{-1,-2,-3},{4,5,6},{7,8,9},{10,11,12},{13,14,15}};
        const auto initial_points=points;
        raw_shadow.fill(0xa5);put(raw_shadow.data(),0x3b0,position);auto expected_shadow=raw_shadow;
        source_position=nullptr;trace.clear();valid=true;build(raw_shadow.data(),&token,left,right);
        const auto native_trace=trace;const auto native_points=points;const auto native_cells=cells;const bool native_valid=valid;
        cells=initial_cells;points=initial_points;trace.clear();valid=true;source_position=&position;
        build_actor_shadow_segments(position,left,right,grid,list);
        put(expected_shadow.data(),0x3b0,position);
        bool same_points=points.size()==native_points.size();
        for(unsigned i=0;same_points&&i<points.size();++i)same_points=point_bits(points[i])==point_bits(native_points[i]);
        // The only possible NaNs in input position are copied or deliberately
        // mutated by adapters. Ignore their payload when checking that field.
        for(unsigned c=0;c<3;++c){const auto off=0x3b0+c*8;
            if(std::isnan(get<double>(raw_shadow.data(),off))&&std::isnan(get<double>(expected_shadow.data(),off)))
                put(expected_shadow.data(),off,get<double>(raw_shadow.data(),off));
        }
        ++cases;three+=points.size()==3;four+=points.size()==4;empty+=trace.empty();
        if(!valid||!native_valid||trace!=native_trace||!same_points||cells!=native_cells||raw_shadow!=expected_shadow){
            std::cerr<<"shadow segments mismatch case="<<cases<<" script="<<script<<" size="<<native_points.size()<<'/'<<points.size()<<'\n';
            for(unsigned i=0;i<native_trace.size()&&i<trace.size();++i)if(native_trace[i]!=trace[i]){std::cerr<<"trace event="<<i<<'\n';break;}
            for(unsigned i=0;i<native_points.size()&&i<points.size();++i)if(point_bits(native_points[i])!=point_bits(points[i])){
                std::cerr<<"point="<<i<<" native="<<native_points[i][0]<<','<<native_points[i][1]<<" source="<<points[i][0]<<','<<points[i][1]<<'\n';break;}
            return false;
        }
        return true;
    };
    // Exhaust all bit/nonzero combinations in the four tested rows, and all
    // callback mutation scripts. Use a fractional right endpoint to insert.
    const std::array<int,4> masks{0,1,2,0x80};
    for(int current:masks)for(int above:masks)for(int below:masks)for(int deep:masks)for(unsigned mode=0;mode<8;++mode){
        cells.fill(0);for(unsigned x=9;x<=11;++x){cells[4*256+x]=current;cells[5*256+x]=above;cells[3*256+x]=below;cells[2*256+x]=deep;}
        script=mode;if(!compare({10.5,4.75,0},9.25,11.75))return 1;
    }
    const double nan=std::numeric_limits<double>::quiet_NaN(),inf=std::numeric_limits<double>::infinity();
    for(double x:{-512.5,-256.0,-1.5,-0.0,0.5,255.5,256.0,511.5,4294967296.5,
                  2147483647.0,-2147483648.0,-2147483647.0,nan,inf,-inf})
        for(double y:{-inf,-3.5,-0.0,0.5,15.75,16.0,4294967296.5,nan,inf})
            for(double right:{-1.0,1.0,12.5,nan,inf}){
                cells.fill(0);script=0;if(!compare({x,y,0},-0.25,right))return 1;
            }
    std::mt19937_64 random(0x459557);
    for(unsigned i=0;i<20000;++i){
        for(auto& cell:cells)cell=std::array<int,7>{0,1,2,0x80,0x200,0x1000,-1}[random()%7];
        Point position{double(int(random()%8192)-4096)/8,double(int(random()%192)-32)/8,0.5};
        const double left=position[0]-double(random()%25)/8;
        const double right=position[0]+double(random()%25)/8;
        script=random()%8;if(!compare(position,left,right))return 1;
    }
    std::cout<<"native shadow segments passed "<<cases<<" list/grid mutation traces: "<<three<<" three-point, "<<four<<" four-point, "<<empty<<" no-iteration cases\n";
    VirtualFree(image,0,MEM_RELEASE);
}
