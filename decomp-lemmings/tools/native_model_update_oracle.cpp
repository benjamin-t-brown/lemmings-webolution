#include "lemmings/model_update.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

namespace {
constexpr std::uintptr_t image_base = 0x400000;
struct Model;
struct NativeState {
    std::array<std::uint8_t, 0x38> prefix{};
    std::uint8_t enabled = 0;
    std::array<std::uint8_t, 7> gap{};
    std::uint8_t remove_when_finished = 0;
    std::array<std::uint8_t, 3> gap2{};
    Model* model = nullptr;
    double remaining = 0;
    std::int32_t mode = 0;
};
static_assert(offsetof(NativeState, model) == 0x44);
static_assert(offsetof(NativeState, remaining) == 0x48);
static_assert(offsetof(NativeState, mode) == 0x50);
struct Event {
    int code, model;
    std::uint64_t bits;
    bool operator==(const Event&) const = default;
};
struct Model {
    void** vtable = nullptr;
    int id = 0;
    std::vector<Event>* events = nullptr;
    std::uint8_t finish = 0;
    double full = 1;
    std::uint8_t* flag = nullptr;
    void** model_field = nullptr;
    Model* replacement = nullptr;
    int mutate_flag = -1;
};
void advance(Model* model, double dt) {
    model->events->push_back({1,model->id,std::bit_cast<std::uint64_t>(dt)});
    if (model->mutate_flag >= 0) *model->flag = static_cast<std::uint8_t>(model->mutate_flag);
    if (model->replacement) *model->model_field = model->replacement;
}
std::uint8_t finished(Model* model) {
    model->events->push_back({2,model->id,0});
    return model->finish;
}
double full(Model* model) {
    model->events->push_back({3,model->id,0});
    return model->full;
}
void set(Model* model, double value) {
    model->events->push_back({4,model->id,std::bit_cast<std::uint64_t>(value)});
}
void __attribute__((thiscall)) native_advance(Model* model,double dt) { advance(model,dt); }
std::uint8_t __attribute__((thiscall)) native_finished(Model* model) { return finished(model); }
double __attribute__((thiscall)) native_full(Model* model) { return full(model); }
void __attribute__((thiscall)) native_set(Model* model,double value) { set(model,value); }
const lemmings::ModelUpdateCallbacks callbacks{
    [](void* p,double dt){advance(static_cast<Model*>(p),dt);},
    [](void* p){return finished(static_cast<Model*>(p));},
    [](void* p){return full(static_cast<Model*>(p));},
    [](void* p,double value){set(static_cast<Model*>(p),value);}
};
template<std::size_t N> bool read_at(std::uintptr_t address) {
    std::array<char,N> bytes{};
    std::cin.read(bytes.data(), N);
    if (std::cin.gcount() != N) return false;
    std::memcpy(reinterpret_cast<void*>(address),bytes.data(),N);
    return true;
}
}
int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(image_base),0xC0000,
        MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(image_base)) return 3;
    if(!read_at<104>(0x432383)||!read_at<15>(0x423470)||
       !read_at<245>(0x43416c)||!read_at<47>(0x434261)||
       !read_at<47>(0x434290)||!read_at<18>(0x41dc00)||
       !read_at<18>(0x41dc20)||!read_at<39>(0x49c5ac)||
       !read_at<8>(0x4b0338)||!read_at<8>(0x4b1210)||
       !read_at<8>(0x4b03f0)) return 2;
    DWORD old=0;
    if(!VirtualProtect(image,0xC0000,PAGE_EXECUTE_READ,&old)) return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xC0000);
    using Update=std::uint8_t(__attribute__((thiscall))*)(NativeState*,double);
    using Enable=void(__attribute__((thiscall))*)(NativeState*);
    auto normal=reinterpret_cast<Update>(0x432383);
    auto blink=reinterpret_cast<Update>(0x43416c);
    auto enable=reinterpret_cast<Enable>(0x434261);
    auto disable=reinterpret_cast<Enable>(0x434290);
    std::array<void*,20> table{};
    table[0x2c/4]=reinterpret_cast<void*>(&native_advance);
    table[0x44/4]=reinterpret_cast<void*>(&native_finished);
    table[0x4c/4]=reinterpret_cast<void*>(&native_full);
    table[0x38/4]=reinterpret_cast<void*>(&native_set);
    std::mt19937_64 rng(0x43416c);
    const double nan=std::numeric_limits<double>::quiet_NaN();
    const double inf=std::numeric_limits<double>::infinity();
    const std::array<double,18> edges{-inf,-1,-0.2,-0.0,0.0,
        std::nextafter(0.2,0.0),0.2,std::nextafter(0.2,1.0),
        0.4,0.6,0.8,1.0,2.0,1e18,1e19,inf,nan,1e-300};
    int cases=0;
    for(int trial=0;trial<20000;++trial) {
        NativeState original{};
        lemmings::ModelUpdateState recovered{};
        original.enabled=recovered.enabled=static_cast<std::uint8_t>(rng());
        original.remove_when_finished=recovered.remove_when_finished=static_cast<std::uint8_t>(trial);
        original.mode=recovered.blink_mode=trial%7-2;
        double remain=trial<static_cast<int>(edges.size()*edges.size())?
            edges[trial/edges.size()]:static_cast<double>(rng()%100000)/1000.0-10;
        double dt=trial<static_cast<int>(edges.size()*edges.size())?
            edges[trial%edges.size()]:static_cast<double>(rng()%10000)/1000.0-1;
        original.remaining=recovered.blink_remaining=remain;
        std::vector<Event> expected,observed;
        Model nm{table.data(),1,&expected,static_cast<std::uint8_t>(trial%256),0.75,
            &original.remove_when_finished,reinterpret_cast<void**>(&original.model)};
        Model sm{nullptr,1,&observed,nm.finish,nm.full,
            &recovered.remove_when_finished,&recovered.model};
        Model nm2=nm,sm2=sm;
        nm2.id=sm2.id=2;
        nm2.finish=sm2.finish=static_cast<std::uint8_t>(trial%3);
        if(trial%11==0) {nm.replacement=&nm2;sm.replacement=&sm2;}
        if(trial%5==0) nm.mutate_flag=sm.mutate_flag=trial%256;
        original.model=&nm;recovered.model=&sm;
        const bool blinking=trial%2!=0;
        if(!blinking&&trial%13==0) {original.model=nullptr;recovered.model=nullptr;}
        if(trial%17==0) {enable(&original);lemmings::enable_blinking_model(recovered);}
        if(trial%19==0) {disable(&original);lemmings::disable_blinking_model(recovered);}
        const bool nr=(blinking?blink:normal)(&original,dt)!=0;
        const bool sr=blinking?lemmings::update_blinking_model(recovered,dt,callbacks):
            lemmings::update_model_entity(recovered,dt,callbacks);
        ++cases;
        if(nr!=sr||original.enabled!=recovered.enabled||
           original.remove_when_finished!=recovered.remove_when_finished||
           original.mode!=recovered.blink_mode||
           std::bit_cast<std::uint64_t>(original.remaining)!=
               std::bit_cast<std::uint64_t>(recovered.blink_remaining)||
           expected!=observed) {
            std::cerr<<"model update mismatch case "<<trial<<" remain "<<remain<<" dt "<<dt
                     <<" mode "<<original.mode<<"/"<<recovered.blink_mode
                     <<" value "<<original.remaining<<"/"<<recovered.blink_remaining
                     <<" events "<<expected.size()<<"/"<<observed.size()<<"\n";
            return 1;
        }
    }
    std::cout<<"native model updates passed "<<cases<<" state and callback-trace comparisons\n";
    VirtualFree(image,0,MEM_RELEASE);
}
