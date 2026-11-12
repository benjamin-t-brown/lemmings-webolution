#include "../tests/sprite_piece_runtime_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

namespace {
sprite_piece_test::Fixture* active;
struct Stream {const std::uint8_t* data;std::size_t size,cursor;};
std::uint32_t __attribute__((thiscall)) stream_read(Stream* stream,std::uint32_t count,void* target) {
    const auto copied=std::min<std::size_t>(count,stream->size-stream->cursor);
    std::memcpy(target,stream->data+stream->cursor,copied);stream->cursor+=copied;return copied;
}
void* __cdecl allocate(std::uint32_t size){return active->runtime_allocate(size).cookie;}
void __cdecl deallocate(void* p){active->runtime_deallocate(reinterpret_cast<lemmings::ResourceCacheEntry*>(static_cast<unsigned char*>(p)+4));}
void jump(std::uintptr_t address,void* target) {
    auto* bytes=reinterpret_cast<std::uint8_t*>(address);bytes[0]=0xe9;
    const auto delta=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-address-5);
    std::memcpy(bytes+1,&delta,4);
}
}
int main() {
    using namespace sprite_piece_test;
    static_assert(sizeof(void*)==4 && sizeof(ResourceCacheEntry)==0x88);
    static_assert(offsetof(registration_test::Block,entries)==4);
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x190000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t count=0;std::cin.read(reinterpret_cast<char*>(&count),4);
    while(count--){std::uint32_t address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0xc0000||address>0x4c0000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
    }
    if(!std::cin)return 2;
    jump(0x47eef7,reinterpret_cast<void*>(&stream_read));
    jump(0x49c1a8,reinterpret_cast<void*>(&allocate));jump(0x49c13d,reinterpret_cast<void*>(&deallocate));
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Read=void(__attribute__((thiscall))*)(void*,Stream*);
    using Constructor=RuntimeSpritePiece*(__attribute__((thiscall))*)(RuntimeSpritePiece*);
    using Find=int(__attribute__((thiscall))*)(void*,const char*);
    using Contains=unsigned char(__attribute__((thiscall))*)(void*,const char*);
    std::size_t callback_count=0;
    for(unsigned i=0;i<32768;++i) {
        Fixture original(i,true),source(i);active=&original;
        if(reinterpret_cast<Constructor>(0x484e30)(&original.piece)!=&original.piece)return 1;
        construct_runtime_sprite_piece(source.piece);
        if(!original.same(source))return 1;
        const auto* original_query=original.input.data()+original.alignment;
        const auto* source_query=source.input.data()+source.alignment;
        const auto found=reinterpret_cast<Find>(0x4607d0)(original.raw_manager.data(),original_query);
        const auto exists=reinterpret_cast<Contains>(0x449b30)(original.raw_manager.data(),original_query);
        if(found!=find_sprite_texture_name(source.state,source_query)||bool(exists)!=contains_sprite_texture_name(source.state,source_query)) {
            std::cerr<<"name lookup mismatch "<<i<<'\n';return 1;
        }
        original.global_manager=reinterpret_cast<void*>(0x58cb48);
        std::memcpy(original.global_manager,original.raw_manager.data(),64);
        Stream stream{original.bytes.data(),original.bytes.size(),0};
        reinterpret_cast<Read>(0x485ce0)(&original.piece,&stream);
        original.cursor=stream.cursor;
        std::memcpy(original.raw_manager.data(),original.global_manager,64);
        source.run_piece();
        callback_count+=source.piece_trace.size()/14;
        if(!original.same(source)) {
            std::cerr<<"runtime piece mismatch "<<i<<" mode "<<original.mode<<" IDs "<<original.piece.texture_id_40<<'/'<<source.piece.texture_id_40<<" cursors "<<original.cursor<<'/'<<source.cursor<<'\n';return 1;
        }
    }
    std::cout<<"32768 complete sprite reader/name registration chains, 65536 searches, 32768 constructors and "<<callback_count<<" callback snapshots match original\n";
}
