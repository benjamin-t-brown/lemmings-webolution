#include "../tests/terrain_blast_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cstring>
#include <iostream>
#include <random>

namespace {
using namespace blast_test;
Fixture* active;
std::array<std::uintptr_t,24> vtable;
void* __cdecl allocate(unsigned size){
    if(size==0x748)return active->create_debris_40c960(active->position);
    if(size==0x158)return active->create_particles_402870();
    if(size==0x48)return active->create_entity_432080(active->position,active->last_effect,1);
    active->valid=false;return nullptr;
}
void* __attribute__((thiscall)) debris_ctor(void* p,const Point*){return p;}
void* __attribute__((thiscall)) particle_ctor(void* p){return p;}
void* __attribute__((thiscall)) entity_ctor(void* p,const Point*,void*,int){return p;}
void __attribute__((thiscall)) set_time(void* p,double value){active->set_time_40c920(p,value);}
void __attribute__((thiscall)) reset(void* p,int value){active->reset_3c(p,value);}
void __attribute__((thiscall)) debris_count(void* p,int value){active->set_debris_count_40dbf0(p,value);}
Point* __attribute__((thiscall)) world_position(void* world,Point* output,const Point* input){active->valid &= world==reinterpret_cast<void*>(0x4c6a10);*output=active->world_position_43d56c(*input);return output;}
void __attribute__((thiscall)) direction(void* p,const Point* value){active->set_direction_54(p,*value);}
void __attribute__((thiscall)) insert(void* list,void** p){active->valid &= list==reinterpret_cast<void*>(0x4c88a8);active->insert_entity_401890(*p);}
void __attribute__((thiscall)) rectangle(void* world,unsigned x,int y,unsigned ex,int ey){active->valid &= world==reinterpret_cast<void*>(0x4c6a10);active->notify_rectangle_43b0a1(x,y,ex,ey);}
void __attribute__((thiscall)) color(void* p,const Point* value){active->set_color_43fa30(static_cast<unsigned char*>(p)-0xb0,*value);}
void __attribute__((thiscall)) particle_count(void* p,int value){active->set_particle_count_402ce0(p,value);}
void __attribute__((thiscall)) position(void* p,const Point* value){active->set_position_08(p,*value);}
void __attribute__((thiscall)) pair_value(void* p,double a,double b){active->set_pair_402d80(p,a,b);}
void __attribute__((thiscall)) flag(void* p,int value){active->set_flag_403e10(p,value);}
void __attribute__((thiscall)) mode(void* p,int value){active->set_mode_402dc0(p,value);}
void __attribute__((thiscall)) advance(void* p,double dt){active->advance_2c(p,dt);}
template<class T>void put(void* p,unsigned offset,const T& value){std::memcpy(static_cast<unsigned char*>(p)+offset,&value,sizeof(value));}
void jump(unsigned address,void* target){put(reinterpret_cast<void*>(address),0,std::uint8_t{0xe9});put(reinterpret_cast<void*>(address),1,static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-address-5));}
}
int main(){
    _setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xd0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);if(!std::cin||blocks>64)return 2;
    for(unsigned i=0;i<blocks;++i){unsigned address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0xd0000||address-0x400000>0xd0000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);if(!std::cin)return 2;}
    jump(0x49c1a8,reinterpret_cast<void*>(&allocate));jump(0x40c960,reinterpret_cast<void*>(&debris_ctor));jump(0x402870,reinterpret_cast<void*>(&particle_ctor));
    jump(0x432080,reinterpret_cast<void*>(&entity_ctor));jump(0x40c920,reinterpret_cast<void*>(&set_time));jump(0x40dbf0,reinterpret_cast<void*>(&debris_count));
    jump(0x43d56c,reinterpret_cast<void*>(&world_position));jump(0x401890,reinterpret_cast<void*>(&insert));jump(0x43b0a1,reinterpret_cast<void*>(&rectangle));
    jump(0x43fa30,reinterpret_cast<void*>(&color));jump(0x402ce0,reinterpret_cast<void*>(&particle_count));jump(0x402d80,reinterpret_cast<void*>(&pair_value));
    jump(0x403e10,reinterpret_cast<void*>(&flag));jump(0x402dc0,reinterpret_cast<void*>(&mode));
    vtable[0x08/4]=reinterpret_cast<std::uintptr_t>(&position);vtable[0x2c/4]=reinterpret_cast<std::uintptr_t>(&advance);
    vtable[0x3c/4]=reinterpret_cast<std::uintptr_t>(&reset);vtable[0x54/4]=reinterpret_cast<std::uintptr_t>(&direction);
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Blast=void(__cdecl*)(Point*,int,double);auto blast=reinterpret_cast<Blast>(0x43f510);
    std::mt19937 random(0x43f510);unsigned total_sites=0;
    const std::array<int,10> values{0,1,2,3,0x80,0x200,0x201,0x800,0x801,0x1001};
    for(unsigned test=0;test<4096;++test){
        Fixture f;active=&f;for(auto& token:f.tokens)token.vtable=reinterpret_cast<std::uintptr_t>(vtable.data());
        f.script=test%128;f.null_entities=test%5==0;f.grid.height=1+random()%16;
        f.position={double(static_cast<int>(random()%1600)-700)/2,
                    double(static_cast<int>(random()%100)-20)/4,double(random()%20)/8};
        if(test%11==0){
            const std::array<double,10> boundaries{std::nextafter(3.5,0.0),3.5,std::nextafter(3.5,4.0),
                std::nextafter(4.5,0.0),4.5,std::nextafter(4.5,5.0),
                std::nextafter(259.5,0.0),259.5,std::nextafter(259.5,300.0),-256.5};
            f.position[0]=boundaries[(test/11)%boundaries.size()];
            f.position[1]=boundaries[(test/17)%6];
        }
        for(auto& cell:f.cells)cell=test%4==0?1:values[random()%values.size()];
        const int mode_value=test%3==0?0:(test%3==1?1:-8);
        const double elapsed=(static_cast<int>(random()%100)-20)/1024.0;
        auto initial=f;put(reinterpret_cast<void*>(0x4c6a10),0,f.grid);
        blast(&f.position,mode_value,elapsed);
        const auto native_trace=f.trace;const auto native_cells=f.cells;const auto native_position=f.position;const auto native_count=f.next;const bool valid=f.valid;
        total_sites+=f.entities-1;f=initial;f.grid.cells.data=f.cells.data();
        apply_terrain_blast(f.grid,f.position,mode_value,elapsed,f);
        bool same_position=true;for(unsigned i=0;i<3;++i)same_position &= bits(native_position[i])==bits(f.position[i]);
        if(!valid||!f.valid||native_trace!=f.trace||native_cells!=f.cells||!same_position||native_count!=f.next){
            std::cerr<<"terrain blast mismatch case="<<test<<" script="<<f.script<<" trace sizes="<<native_trace.size()<<'/'<<f.trace.size()<<'\n';
            for(unsigned i=0;i<native_trace.size()&&i<f.trace.size();++i)if(native_trace[i]!=f.trace[i]){
                std::cerr<<"event="<<i<<" kinds="<<native_trace[i].kind<<'/'<<f.trace[i].kind<<" hashes="<<native_trace[i].grid_hash<<'/'<<f.trace[i].grid_hash<<'\n';
                for(unsigned j=0;j<7;++j)if(native_trace[i].values[j]!=f.trace[i].values[j])std::cerr<<"value="<<j<<" bits="<<std::hex<<native_trace[i].values[j]<<'/'<<f.trace[i].values[j]<<'\n';
                break;}
            return 1;
        }
    }
    std::cout<<"terrain blast: 4096 full routine traces, "<<total_sites<<" debris sites passed\n";
}
