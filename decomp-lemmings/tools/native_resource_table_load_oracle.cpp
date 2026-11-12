#include "../tests/resource_table_load_fixture.hpp"
#include "../tests/resource_registration_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
namespace {
table_load_test::Fixture* table=nullptr;
registration_test::Fixture* resize=nullptr;
bool valid=true;
void* __cdecl allocate(std::uint32_t size) {return table?table->allocate(size).cookie:resize->allocate(size).cookie;}
void __cdecl release(void* p) {
    if(table) {
        for(auto& node:table->nodes)if(p==&node){table->release(&node);return;}
        table->deallocate(reinterpret_cast<lemmings::ResourceCacheEntry*>(static_cast<unsigned char*>(p)+4));
    } else resize->deallocate(reinterpret_cast<lemmings::ResourceCacheEntry*>(static_cast<unsigned char*>(p)+4));
}
void __attribute__((thiscall)) destroy(void* manager,void* resource) {valid&=manager==&table->manager.lookup;table->destroy(resource);}
// Named read_stream rather than read: <io.h> declares a global read, which
// makes the address-of expression below ambiguous.
std::uint32_t __attribute__((thiscall)) read_stream(lemmings::LevelByteStream* stream,std::uint32_t count,char* destination) {
    if(stream->cursor>stream->bytes.size()||count>stream->bytes.size()-stream->cursor){valid=false;return 0;}
    std::memcpy(destination,stream->bytes.data()+stream->cursor,count);stream->cursor+=count;return count;
}
void jump(std::uintptr_t a,void* target){auto* code=reinterpret_cast<unsigned char*>(a);code[0]=0xe9;auto displacement=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-a-5);std::memcpy(code+1,&displacement,4);}
}
int main() {
    using namespace lemmings;
    static_assert(sizeof(void*)==4);static_assert(sizeof(ResourceCacheEntry)==0x88);
    static_assert(offsetof(table_load_test::Manager,loaded)==0x2c);static_assert(offsetof(table_load_test::Manager,entries)==0x3c);
    static_assert(offsetof(table_load_test::Block,entries)==4);
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xc0000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);
    while(blocks--){std::uint32_t a=0,n=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);
        if(a<0x400000||n>0xc0000||a>0x4c0000-n)return 2;std::cin.read(reinterpret_cast<char*>(a),n);}
    if(!std::cin)return 2;
    jump(0x49c1a8,reinterpret_cast<void*>(&allocate));jump(0x49c13d,reinterpret_cast<void*>(&release));jump(0x47eef7,reinterpret_cast<void*>(&read_stream));
    std::array<std::uintptr_t,5> vtable{};vtable[4]=reinterpret_cast<std::uintptr_t>(&destroy);
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Resize=void(__attribute__((thiscall))*)(void*,std::int32_t);
    for(unsigned i=0;i<32768;++i)for(unsigned variant=0;variant<2;++variant) {
        registration_test::Fixture original(i,true),source(i);resize=&original;
        auto requested=static_cast<std::int32_t>((i/32)%15)-2;
        reinterpret_cast<Resize>(variant?0x449bf0:0x449940)(original.raw_manager.data()+0x30,requested);
        resize_resource_entries(source.state,requested,source.hooks());
        if(original.trace!=source.trace||!original.equals(source)) {std::cerr<<"resize mismatch "<<i<<" variant "<<variant<<" request "<<requested<<'\n';return 1;}
    }
    using Load=void(__attribute__((thiscall))*)(void*,void*);
    for(unsigned i=0;i<8192;++i)for(unsigned variant=0;variant<2;++variant) {
        table_load_test::Fixture original(i,true),source(i);table=&original;
        auto* vt=vtable.data();std::memcpy(original.manager.lookup.unknown_00.data(),&vt,4);
        reinterpret_cast<Load>(variant?0x445eb0:0x445c10)(&original.manager,&original.stream);
        source.run();
        if(!valid||original.trace!=source.trace||!original.equals(source)) {
            std::cerr<<"table mismatch "<<i<<" variant "<<variant<<" trace sizes "<<original.trace.size()<<'/'<<source.trace.size()<<'\n';
            for(unsigned j=0;j<std::min(original.trace.size(),source.trace.size());++j)if(original.trace[j]!=source.trace[j]){std::cerr<<"trace index "<<j<<" values "<<original.trace[j]<<'/'<<source.trace[j]<<'\n';break;}
            return 1;
        }
    }
    std::cout<<"65536 vector resize/callback traces and 16384 complete resource table loads match original (both specializations)\n";
}
