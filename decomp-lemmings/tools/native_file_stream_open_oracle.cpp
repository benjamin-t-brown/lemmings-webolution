#include "../tests/file_stream_open_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cstdio>
#include <iostream>

namespace {
file_open_test::OpenFixture* active;
void before(){active->decode();active->paths.prefix_enabled_5d5604=*reinterpret_cast<std::uint8_t*>(0x5d5604);active->buffer.next_5d55fc=*reinterpret_cast<const void**>(0x5d55fc);active->buffer.end_5d5600=*reinterpret_cast<const void**>(0x5d5600);}
void after(){active->raw=active->encode();*reinterpret_cast<std::uint8_t*>(0x5d5604)=active->paths.prefix_enabled_5d5604;*reinterpret_cast<const void**>(0x5d55fc)=active->buffer.next_5d55fc;*reinterpret_cast<const void**>(0x5d5600)=active->buffer.end_5d5600;}
void __cdecl prepare(std::int32_t mode,const char* output){before();active->prepare(mode,output);after();}
std::uint8_t __cdecl exists(const char* name){before();auto result=active->exists(name);after();return result;}
std::uint8_t __cdecl ready(){before();auto result=active->ready();after();return result;}
void __cdecl reset92(){before();active->reset92();after();}
void* __attribute__((thiscall)) find_archive(void* registry,const char* name){
    if(registry!=reinterpret_cast<void*>(0x5d5608))std::abort();
    before();const auto* match=active->find(name);after();
    if(!match)return nullptr;
    active->write32(active->raw_match.data(),match->archive_index_00);active->write32(active->raw_match.data()+0x104,match->size_104);active->write32(active->raw_match.data()+0x108,match->offset_108);return active->raw_match.data();
}
void __cdecl clear(){before();active->clear();after();}
void __cdecl register_archives(){before();active->register_archives();after();}
void __cdecl resetE6(){before();active->resetE6();after();}
void** __attribute__((thiscall)) archive_slot(void* registry,std::uint32_t index){
    if(registry!=reinterpret_cast<void*>(0x5d5608))std::abort();
    before();active->archive_name(index);after();return &active->archive_slot;
}
int __cdecl format_message(char* destination,const char* format,const char* name){
    if(std::strcmp(format,"Cannot find required file '%s'.  Click OK to try again, or cancel to exit."))std::abort();
    const int count=std::snprintf(destination,260,format,name);if(count<0||count>=260)std::abort();return count;
}
int __stdcall message_box(HWND window,const char* text,const char* caption,UINT type){if(window)std::abort();before();auto result=active->message(text,caption,type);after();return result;}
void* __cdecl open_adapter(const char* name,const char* mode,std::uint32_t share){before();auto* result=active->open(name,mode,share);after();return result;}
std::int32_t __cdecl close_adapter(void* file){before();auto result=active->close(file);after();return result;}
std::int32_t __cdecl tell_adapter(void* file){before();auto result=active->tell(file);after();return result;}
std::int32_t __cdecl seek_adapter(void* file,std::int32_t offset,std::int32_t origin){before();auto result=active->seek(file,offset,origin);after();return result;}
void jump(std::uintptr_t address,void* target){auto* code=reinterpret_cast<unsigned char*>(address);code[0]=0xe9;auto offset=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-address-5);std::memcpy(code+1,&offset,4);}
}
int main(){
    using namespace file_open_test;
    static_assert(sizeof(void*)==4);_setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x200000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t count=0;std::cin.read(reinterpret_cast<char*>(&count),4);
    while(count--){std::uint32_t address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0x200000||address>0x600000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
    }
    if(!std::cin)return 2;
    jump(0x47dd54,reinterpret_cast<void*>(&prepare));jump(0x47e89b,reinterpret_cast<void*>(&exists));jump(0x47ef48,reinterpret_cast<void*>(&ready));jump(0x47ef92,reinterpret_cast<void*>(&reset92));jump(0x47f759,reinterpret_cast<void*>(&find_archive));
    jump(0x47e2f3,reinterpret_cast<void*>(&clear));jump(0x47e23b,reinterpret_cast<void*>(&register_archives));jump(0x47efe6,reinterpret_cast<void*>(&resetE6));jump(0x47f210,reinterpret_cast<void*>(&archive_slot));jump(0x49ca6a,reinterpret_cast<void*>(&format_message));
    jump(0x49db6a,reinterpret_cast<void*>(&open_adapter));jump(0x49da2c,reinterpret_cast<void*>(&close_adapter));jump(0x49db9d,reinterpret_cast<void*>(&tell_adapter));jump(0x49dcf5,reinterpret_cast<void*>(&seek_adapter));
    *reinterpret_cast<void**>(0x4b01cc)=reinterpret_cast<void*>(&message_box);
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Open=std::uint8_t(__attribute__((thiscall))*)(void*,std::int32_t,std::uint32_t);
    for(unsigned i=0;i<27648;++i){
        OpenFixture original(i,true),source(i);active=&original;after();
        const auto actual=reinterpret_cast<Open>(0x47e40d)(original.raw.data(),original.mode,original.required);
        const auto expected=source.run_open();
        if(actual!=expected||original.raw!=source.encode()||original.events!=source.events||*reinterpret_cast<std::uint8_t*>(0x5d5604)!=source.paths.prefix_enabled_5d5604||*reinterpret_cast<const void**>(0x5d55fc)!=source.buffer.next_5d55fc||*reinterpret_cast<const void**>(0x5d5600)!=source.buffer.end_5d5600){
            std::cerr<<"file open mismatch "<<i<<" scenario "<<original.scenario<<" script "<<original.script<<'\n';
            const auto n=std::min(original.events.size(),source.events.size());for(unsigned j=0;j<n;++j)if(original.events[j]!=source.events[j]){std::cerr<<original.events[j]<<"\n"<<source.events[j]<<'\n';break;}
            return 1;
        }
    }
    std::cout<<"27648 complete original file-open traces match, including retries and failure paths\n";
}
