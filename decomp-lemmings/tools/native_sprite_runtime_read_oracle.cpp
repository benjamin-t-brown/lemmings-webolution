#include "../tests/sprite_runtime_read_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>

namespace {
sprite_read_test::Fixture* active;
struct Stream {const std::uint8_t* data;std::size_t size,cursor;};
std::uint32_t __attribute__((thiscall)) stream_read(Stream* s,std::uint32_t n,void* output) {
    auto copied=std::min<std::size_t>(n,s->size-s->cursor);
    std::memcpy(output,s->data+s->cursor,copied);s->cursor+=copied;return copied;
}
__attribute__((noinline)) void* __cdecl allocate(std::uint32_t bytes) {
    const auto caller=reinterpret_cast<std::uintptr_t>(__builtin_return_address(0));
    unsigned kind=99;
    switch(caller) {
    case 0x4842e6:kind=0;break;case 0x485943:kind=1;break;case 0x485ba5:kind=2;break;
    case 0x484674:kind=3;break;case 0x47c71f:kind=4;break;case 0x46038c:kind=5;break;
    default:std::cerr<<"unknown allocation caller "<<std::hex<<caller<<'\n';std::abort();
    }
    const auto count=(bytes-(kind==4?0:4))/sprite_read_test::original_stride(kind);
    auto* elements=static_cast<std::byte*>(active->allocate(kind,count,bytes));
    return kind==4?elements:elements-4;
}
void __cdecl release(void* p){active->release(p);}
void jump(std::uintptr_t address,void* target) {
    auto* p=reinterpret_cast<std::uint8_t*>(address);p[0]=0xe9;
    auto displacement=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-address-5);
    std::memcpy(p+1,&displacement,4);
}
void run_original(sprite_read_test::Fixture& fixture,const std::vector<std::uint8_t>& data) {
    active=&fixture;fixture.global_manager=reinterpret_cast<void*>(0x58cb48);fixture.sync_to_global();
    Stream stream{data.data(),data.size(),0};fixture.live_cursor=&stream.cursor;
    using Read=void(__attribute__((thiscall))*)(void*,Stream*);
    reinterpret_cast<Read>(0x483ef0)(&fixture.resource,&stream);
    fixture.cursor=stream.cursor;fixture.live_cursor=&fixture.cursor;fixture.sync_from_global();
}
}
int main() {
    using namespace sprite_read_test;
    static_assert(sizeof(void*)==4&&sizeof(SpriteDestructionClip)==152&&sizeof(SpriteDestructionFrame)==16&&sizeof(SpriteDestructionTripleArray)==8);
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x190000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);
    while(blocks--){std::uint32_t a=0,n=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);
        if(a<0x400000||n>0xc0000||a>0x4c0000-n)return 2;
        std::cin.read(reinterpret_cast<char*>(a),n);
    }
    if(!std::cin)return 2;
    jump(0x47eef7,reinterpret_cast<void*>(&stream_read));jump(0x49c1a8,reinterpret_cast<void*>(&allocate));jump(0x49c13d,reinterpret_cast<void*>(&release));
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    unsigned cases=0;std::size_t allocations=0;
    for(unsigned seed=0;seed<128;++seed) {
        Fixture original(1+seed%7),source(1+seed%7);
        for(unsigned pass=0;pass<3;++pass) {
            auto data=synthetic(seed+pass);run_original(original,data);source.run(data);
            if(!original.equals(source)||original.cursor!=data.size()) {std::cerr<<"synthetic mismatch "<<seed<<'/'<<pass<<'\n';return 1;}
            ++cases;
        }
        allocations+=source.blocks.size();
    }
    std::uint32_t installed=0;std::cin.read(reinterpret_cast<char*>(&installed),4);
    for(unsigned i=0;i<installed;++i) {
        std::uint32_t size=0;std::cin.read(reinterpret_cast<char*>(&size),4);
        if(size>32*1024*1024)return 2;
        std::vector<std::uint8_t> data(size);std::cin.read(reinterpret_cast<char*>(data.data()),size);if(!std::cin)return 2;
        Fixture original,source;
        for(unsigned pass=0;pass<2;++pass) {
            run_original(original,data);source.run(data);
            if(!original.equals(source)||original.cursor!=data.size()) {std::cerr<<"installed mismatch "<<i<<'/'<<pass<<'\n';return 1;}
        }
        allocations+=source.blocks.size();
    }
    std::cout<<cases<<" synthetic full SPR reads and "<<installed<<" installed SPRs read twice match original; "<<allocations<<" allocations compared\n";
}
