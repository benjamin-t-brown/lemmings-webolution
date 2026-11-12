#include "../tests/file_stream_runtime_fixture.hpp"
#include "lemmings/level_stream.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

namespace {
file_runtime_test::Fixture* active;
struct MemoryRead {const std::uint8_t* bytes;std::size_t size,cursor;};
MemoryRead* memory=nullptr;
std::uint32_t __cdecl read_bytes(void* destination,std::uint32_t size,std::uint32_t count,void* file) {
    if(size!=1)std::abort();
    if(memory){
        if(file!=reinterpret_cast<void*>(0x12345678))std::abort();
        const auto actual=std::min<std::size_t>(count,memory->size-memory->cursor);
        std::memcpy(destination,memory->bytes+memory->cursor,actual);memory->cursor+=actual;return static_cast<std::uint32_t>(actual);
    }
    return active->read(destination,count,file);
}
std::int32_t __cdecl close_file(void* file){return active->close(file);}
void jump(std::uintptr_t address,void* target){auto* code=reinterpret_cast<unsigned char*>(address);code[0]=0xe9;auto offset=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-address-5);std::memcpy(code+1,&offset,4);}
}
int main() {
    using namespace file_runtime_test;
    static_assert(sizeof(void*)==4);static_assert(sizeof(FileStreamRuntime)==0x134);static_assert(offsetof(FileStreamRuntime,file_118)==0x118);
    _setmode(_fileno(stdin),_O_BINARY);void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x200000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);
    while(blocks--){std::uint32_t address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0x200000||address>0x600000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
    }
    if(!std::cin)return 2;
    jump(0x49da82,reinterpret_cast<void*>(&read_bytes));jump(0x49da2c,reinterpret_cast<void*>(&close_file));
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Constructor=void*(__attribute__((thiscall))*)(void*);
    using Named=void*(__attribute__((thiscall))*)(void*,const char*);
    using Name=void(__attribute__((thiscall))*)(void*,const char*);
    using Read=std::uint32_t(__attribute__((thiscall))*)(void*,std::uint32_t,void*);
    using ForwardRead=void(__attribute__((thiscall))*)(void*,std::uint32_t,void*);
    using Action=void(__attribute__((thiscall))*)(void*);
    using Word=std::uint32_t(__attribute__((thiscall))*)(void*);
    using Byte=std::uint8_t(__attribute__((thiscall))*)(void*);
    for(unsigned i=0;i<34320;++i){
        Fixture original(i,true),source(i);active=&original;void* p=original.raw.data();std::uint32_t actual=0;
        *reinterpret_cast<std::uint8_t*>(0x5d5604)=original.prefix_enabled;std::memcpy(reinterpret_cast<void*>(0x5d5628),original.prefix.data(),260);
        *reinterpret_cast<const void**>(0x5d55fc)=original.buffer.next_5d55fc;*reinterpret_cast<const void**>(0x5d5600)=original.buffer.end_5d5600;
        switch(original.operation){
        case 0:if(reinterpret_cast<Constructor>(0x47db2f)(p)!=p)return 1;break;
        case 1:if(reinterpret_cast<Named>(0x47db73)(p,original.name())!=p)return 1;break;
        case 2:reinterpret_cast<Name>(0x47e395)(p,original.name());break;
        case 3:actual=reinterpret_cast<Read>(0x47eef7)(p,original.count,original.destination.data());break;
        case 4:reinterpret_cast<ForwardRead>(0x47dac8)(p,original.count,original.destination.data());break;
        case 5:reinterpret_cast<Action>(0x47e844)(p);break;
        case 6:reinterpret_cast<Action>(0x47dc28)(p);break;
        case 7:actual=reinterpret_cast<Word>(0x47ee27)(p);break;
        case 8:actual=reinterpret_cast<Byte>(0x47ee38)(p);break;
        case 9:actual=reinterpret_cast<Byte>(0x47ee8c)(p);break;
        case 10:reinterpret_cast<Action>(0x47daa9)(p);break;
        }
        const auto expected=source.run();
        if(actual!=expected||original.raw!=source.encode()||original.destination!=source.destination||original.trace!=source.trace||*reinterpret_cast<const void**>(0x5d55fc)!=source.buffer.next_5d55fc||*reinterpret_cast<const void**>(0x5d5600)!=source.buffer.end_5d5600){
            std::cerr<<"file runtime mismatch trial "<<i<<" operation "<<original.operation<<'\n';return 1;
        }
    }
    // Existing string-reader proof extended down through original 47EEF7:
    // only fread is adapted here, not the game's block-read routine.
    using StringReader=void(__attribute__((thiscall))*)(void*,char*,int);
    for(unsigned i=0;i<10000;++i){
        const int length=static_cast<int>(i%401),capacity=1+static_cast<int>((i*31)%300);
        std::vector<std::uint8_t> data(4+length);std::memcpy(data.data(),&length,4);for(int j=0;j<length;++j)data[4+j]=static_cast<std::uint8_t>(i+j);
        MemoryRead backing{data.data(),data.size(),0};memory=&backing;
        Fixture original(i,true);Fixture::write32(original.raw.data()+0x118,0x12345678);Fixture::write32(original.raw.data()+0x120,0);
        auto untouched=original.raw;
        std::array<char,512> actual,expected;actual.fill(static_cast<char>(0xa5));expected=actual;
        reinterpret_cast<StringReader>(0x47d940)(original.raw.data(),actual.data(),capacity);
        LevelByteStream source{data};read_level_string(source,expected.data(),capacity);
        Fixture::write32(untouched.data()+0x120,static_cast<std::uint32_t>(source.cursor));
        if(actual!=expected||backing.cursor!=source.cursor||original.raw!=untouched){std::cerr<<"composed string/file mismatch "<<i<<'\n';return 1;}
    }
    std::cout<<"34320 file-runtime traces and 10000 composed original string/file reads match (CRT read/close adapters only)\n";
}
