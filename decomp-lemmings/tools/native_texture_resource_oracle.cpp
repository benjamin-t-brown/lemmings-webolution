#include "lemmings/texture_resource.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cstring>
#include <iostream>
#include <memory>

namespace {
std::span<const std::uint8_t> active_input;
std::vector<std::unique_ptr<std::uint8_t[]>> allocations;
void* active_file;
unsigned file_events;
void* __cdecl allocate(std::uint32_t size) {
    auto data=std::make_unique<std::uint8_t[]>(size);
    std::memset(data.get(),0xcd,size);
    auto* result=data.get();allocations.push_back(std::move(data));return result;
}
void __cdecl free_memory(void* p) {
    for(const auto& allocation:allocations)if(allocation.get()==p)return;
    std::abort();
}
void* __attribute__((thiscall)) construct_file(void* file,const char* name) {
    if(std::strcmp(name,"fixture.bmp")!=0 || file_events!=0)std::abort();
    active_file=file;++file_events;return file;
}
std::uint32_t __attribute__((thiscall)) open_file(void* file,std::uint32_t a,std::uint32_t b) {
    if(file!=active_file || a!=1 || b!=0 || file_events!=1)std::abort();
    ++file_events;return 1;
}
std::uint32_t __attribute__((thiscall)) file_size(void* file) {
    if(file!=active_file || file_events!=2)std::abort();
    ++file_events;
    return static_cast<std::uint32_t>(active_input.size());
}
void __attribute__((thiscall)) read_file(void* file,void* output,std::uint32_t size) {
    if(file!=active_file || size!=active_input.size() || file_events!=3)std::abort();
    ++file_events;std::memcpy(output,active_input.data(),size);
}
void __attribute__((thiscall)) close_file(void* file) {
    if(file!=active_file || file_events!=4)std::abort();
    ++file_events;
}
void __attribute__((thiscall)) destroy_file(void* file) {
    if(file!=active_file || file_events!=5)std::abort();
    ++file_events;
}
void* __cdecl copy(void* to,const void* from,std::uint32_t size) { return std::memcpy(to,from,size); }
void jump(std::uintptr_t address,void* target) {
    auto* bytes=reinterpret_cast<std::uint8_t*>(address);bytes[0]=0xe9;
    auto delta=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-address-5);
    std::memcpy(bytes+1,&delta,4);
}
}

int main() {
    static_assert(sizeof(void*) == 4);
    _setmode(_fileno(stdin),_O_BINARY);
    void* image = VirtualAlloc(reinterpret_cast<void*>(0x400000),0x100000,
                               MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    auto integer=[] { std::uint32_t value=0; std::cin.read(reinterpret_cast<char*>(&value),4); return value; };
    auto count=integer();
    while (count--) {
        auto address=integer(),size=integer();
        if (address<0x400000 || size>0x100000 || address>0x500000-size) return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
    }
    jump(0x49c1a8,reinterpret_cast<void*>(&allocate));
    jump(0x49c13d,reinterpret_cast<void*>(&free_memory));
    jump(0x47db73,reinterpret_cast<void*>(&construct_file));
    jump(0x47e40d,reinterpret_cast<void*>(&open_file));
    jump(0x47ee27,reinterpret_cast<void*>(&file_size));
    jump(0x43b5b0,reinterpret_cast<void*>(&read_file));
    jump(0x47e844,reinterpret_cast<void*>(&close_file));
    jump(0x47dc28,reinterpret_cast<void*>(&destroy_file));
    jump(0x49d010,reinterpret_cast<void*>(&copy));
    DWORD previous;
    if (!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&previous)) return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Decode=std::uint32_t(__cdecl*)(const void*,void*,std::uint32_t,std::uint32_t);
    const auto cases=integer();
    std::uint64_t total_bytes=0,bitmap_bytes=0;
    unsigned bitmap_count=0,block_count=0;
    for (unsigned trial=0; trial<cases; ++trial) {
        const auto kind=integer(),input_size=integer(),output_size=integer();
        std::vector<std::uint8_t> input(input_size),output(output_size+65536,0xa5);
        std::cin.read(reinterpret_cast<char*>(input.data()),input_size);
        if (!std::cin) return 2;
        if(kind==1) {
            struct Buffer {std::uint32_t size;std::uint8_t* data;} buffer{};
            using Read=void(__attribute__((thiscall))*)(Buffer*,const char*);
            allocations.clear();active_input=input;file_events=0;
            reinterpret_cast<Read>(0x45015a)(&buffer,"fixture.bmp");
            if(file_events!=6)return 1;
            try {
                const auto expected=lemmings::decode_texture_bitmap(input);
                if(buffer.size!=expected.size() || std::memcmp(buffer.data,expected.data(),buffer.size)!=0) {
                    std::cerr<<"texture envelope mismatch "<<trial<<'\n';return 1;
                }
            } catch(const std::exception& error) {
                std::cerr<<"texture envelope rejected "<<trial<<": "<<error.what()<<'\n';return 1;
            }
            ++bitmap_count;bitmap_bytes+=buffer.size;continue;
        }
        const auto written=reinterpret_cast<Decode>(0x467de8)(input.data(),output.data(),input_size,output_size);
        try {
            const auto expected=lemmings::decode_texture_block(input,output_size);
            if (written!=output_size || expected.size()!=written ||
                std::memcmp(expected.data(),output.data(),written)!=0) {
                std::cerr<<"texture codec mismatch "<<trial<<" size="<<output_size<<" original="<<written<<'\n';
                for (unsigned i=0;i<expected.size();++i)if(expected[i]!=output[i]){std::cerr<<"firstbyte "<<i<<'\n';break;}
                return 1;
            }
            for (std::size_t i=written;i<output.size();++i)if(output[i]!=0xa5)return 1;
        } catch (const std::exception& error) {
            std::cerr<<"texture codec rejected original case "<<trial<<": "<<error.what()<<'\n';
            return 1;
        }
        total_bytes+=written;
        ++block_count;
    }
    std::cout<<block_count<<" texture codec blocks / "<<total_bytes<<" decoded bytes match unchanged original\n";
    std::cout<<bitmap_count<<" texture envelopes / "<<bitmap_bytes<<" decoded bytes match original full loader\n";
}
