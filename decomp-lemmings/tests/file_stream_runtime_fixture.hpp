#pragma once
#include "lemmings/file_stream_runtime.hpp"
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <vector>

namespace file_runtime_test {
using namespace lemmings;
struct Fixture {
    FileStreamRuntime stream;
    FileStreamBufferPointers buffer;
    std::array<unsigned char,0x134> raw{};
    std::array<char,264> input{},prefix{};
    std::array<unsigned char,96> destination{};
    std::vector<std::uint32_t> trace;
    unsigned trial,operation,mutation,count,alignment;
    std::uint8_t prefix_enabled;
    bool native,alias;
    static std::uint32_t read32(const void* p){std::uint32_t value;std::memcpy(&value,p,4);return value;}
    static void write32(void* p,std::uint32_t value){std::memcpy(p,&value,4);}
    explicit Fixture(unsigned t,bool n=false):trial(t),operation(t%11),mutation((t/11)%4),count((t/44)%65),alignment((t/2860)%4),prefix_enabled((t/11440)%3==0?0:((t/11440)%3==1?1:255)),native(n),alias((t/11)%5==0) {
        unsigned random=t+1;
        for(auto& c:raw){random=random*1664525u+1013904223u;c=static_cast<unsigned char>(random>>24);}
        const unsigned prefix_length=(t/33)%9;
        const unsigned length=std::min<unsigned>((t/11)%260,259-(prefix_enabled?prefix_length:0));
        for(unsigned i=0;i<prefix_length;++i)prefix[i]=static_cast<char>('a'+i);
        for(unsigned i=0;i<length;++i){input[alignment+i]=static_cast<char>(1+(i+t)%255);raw[0x14+i]=static_cast<unsigned char>(input[alignment+i]);}
        input[alignment+length]=0;raw[0x14+length]=0;
        write32(raw.data()+0x118,((operation!=3&&operation!=4)&&t%3==0)?0:0x12345678u);
        if(operation==3||operation==4)write32(raw.data()+0x10,count/2);
        const std::uint32_t cursors[]={0,0xffffffffu,0xfffffffeu,0x7fffffffu};
        write32(raw.data()+0x120,cursors[(t/11)%4]);write32(raw.data()+0x130,(t/33)%3==0?1:(t/33)%3);
        const unsigned flags[]={0,1,128,255};raw[0x11c]=static_cast<unsigned char>(flags[(t/44)%4]);raw[0x12c]=static_cast<unsigned char>(flags[(t/176)%4]);
        buffer.next_5d55fc=reinterpret_cast<void*>(0x1000);buffer.end_5d5600=reinterpret_cast<void*>(t%2?0x1000:0x1004);
        destination.fill(0xa5);decode();
    }
    void decode() {
        std::copy_n(raw.begin(),16,stream.unknown_00.begin());stream.size_10=read32(raw.data()+0x10);
        std::memcpy(stream.name_14.data(),raw.data()+0x14,260);stream.file_118=reinterpret_cast<void*>(std::uintptr_t(read32(raw.data()+0x118)));
        stream.ended_11c=raw[0x11c];stream.archive_11d=raw[0x11d];std::copy_n(raw.begin()+0x11e,2,stream.unknown_11e.begin());
        stream.cursor_120=read32(raw.data()+0x120);stream.archive_offset_124=read32(raw.data()+0x124);stream.archive_index_128=read32(raw.data()+0x128);
        stream.opened_12c=raw[0x12c];std::copy_n(raw.begin()+0x12d,3,stream.unknown_12d.begin());stream.mode_130=read32(raw.data()+0x130);
    }
    std::array<unsigned char,0x134> encode()const {
        std::array<unsigned char,0x134> result{};std::copy(stream.unknown_00.begin(),stream.unknown_00.end(),result.begin());write32(result.data()+0x10,stream.size_10);
        std::memcpy(result.data()+0x14,stream.name_14.data(),260);write32(result.data()+0x118,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(stream.file_118)));
        result[0x11c]=stream.ended_11c;result[0x11d]=stream.archive_11d;std::copy(stream.unknown_11e.begin(),stream.unknown_11e.end(),result.begin()+0x11e);
        write32(result.data()+0x120,stream.cursor_120);write32(result.data()+0x124,stream.archive_offset_124);write32(result.data()+0x128,stream.archive_index_128);
        result[0x12c]=stream.opened_12c;std::copy(stream.unknown_12d.begin(),stream.unknown_12d.end(),result.begin()+0x12d);write32(result.data()+0x130,stream.mode_130);return result;
    }
    const char* name()const {return alias?(native?reinterpret_cast<const char*>(raw.data()+0x14):stream.name_14.data()):input.data()+alignment;}
    FileStreamPathEnvironment environment()const {return {prefix_enabled,prefix.data()};}
    void mutate() {
        if(mutation==1||mutation==3)stream.cursor_120=0xfffffffeu;
        if(mutation==2||mutation==3){stream.file_118=reinterpret_cast<void*>(0x87654321u);stream.size_10=17;stream.archive_11d=0x81;stream.ended_11c=0x80;stream.opened_12c=0xff;stream.name_14[0]='#';}
    }
    std::uint32_t read(void* output,std::uint32_t requested,void* file) {
        if(native)decode();
        if(output!=destination.data())std::abort();
        trace.insert(trace.end(),{1,requested,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(file)),stream.cursor_120});
        const unsigned actual=mutation==0?0:(mutation==1?requested/2:requested);
        for(unsigned i=0;i<actual;++i)destination[i]=static_cast<unsigned char>(trial+i);
        mutate();if(native)raw=encode();return actual;
    }
    std::int32_t close(void* file) {
        if(native)decode();
        trace.insert(trace.end(),{2,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(file)),stream.cursor_120});
        mutate();if(native)raw=encode();return -1;
    }
    FileStreamRuntimeHooks hooks(){return {this,[](void* p,void* d,std::uint32_t n,void* f){return static_cast<Fixture*>(p)->read(d,n,f);},[](void* p,void* f){return static_cast<Fixture*>(p)->close(f);}};}
    std::uint32_t run() {
        const auto h=hooks();const auto e=environment();
        switch(operation){
        case 0:if(construct_file_stream(stream)!=&stream)std::abort();break;
        case 1:if(construct_named_file_stream(stream,name(),e)!=&stream)std::abort();break;
        case 2:set_file_stream_name(stream,name(),e);break;
        case 3:return read_file_stream(stream,count,destination.data(),h);
        case 4:read_file_stream(stream,count,destination.data(),h);break;
        case 5:close_file_stream(stream,h);break;
        case 6:destroy_file_stream(stream,h);break;
        case 7:return file_stream_size(stream);
        case 8:return file_stream_at_end(stream,buffer);
        case 9:return file_stream_is_open(stream);
        case 10:reset_file_stream_buffer(buffer,reinterpret_cast<void*>(0x5cd5fc));break;
        }
        return 0;
    }
};
}
