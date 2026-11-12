#pragma once
#include "file_stream_runtime_fixture.hpp"
#include <bit>

namespace file_runtime_open_test {
using namespace lemmings;
struct Fixture: file_runtime_test::Fixture {
    unsigned script,steps=0,tells=0;
    bool archive;
    std::vector<std::uint32_t> events;
    explicit Fixture(unsigned t,bool n=false):file_runtime_test::Fixture(t,n),script((t/2)%8),archive(t%2!=0) {
        stream.mode_130=(t/16)%4;
        stream.archive_offset_124=(t%4==0)?0x80000000u:(t%4==1?0xffffffffu:t*97);
        stream.file_118=reinterpret_cast<void*>(0x11110000);
        raw=encode();
    }
    void before(unsigned kind,void* file,std::uint32_t offset=0,std::uint32_t origin=0) {
        if(native)decode();
        events.insert(events.end(),{kind,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(file)),offset,origin,stream.size_10,stream.cursor_120,stream.mode_130,stream.ended_11c,stream.opened_12c});
    }
    void change() {
        ++steps;
        if(script==1||script==7)stream.file_118=reinterpret_cast<void*>(std::uintptr_t(0x11110000+steps*4));
        if(script==2||script==7)stream.cursor_120+=13;
        if(script==3||script==7)stream.size_10=0xabcd0000+steps;
        if(script==4||script==7)stream.mode_130=(trial/128)%2?1:7;
        if(script==5||script==7){stream.ended_11c=128;stream.opened_12c=255;}
        if(script==6||script==7)stream.archive_offset_124=0x12340000+steps;
        if(script==7){buffer.next_5d55fc=reinterpret_cast<void*>(std::uintptr_t(0x2000+steps));buffer.end_5d5600=reinterpret_cast<void*>(0x3000);}
        if(native)raw=encode();
    }
    std::int32_t tell(void* file) {
        before(1,file);++tells;
        constexpr std::uint32_t values[]={0,0xffffffffu,0x80000000u,0x7fffffffu,37,0xfffffffeu};
        const auto result=std::bit_cast<std::int32_t>(values[(trial/16+tells-1)%6]);
        events.push_back(static_cast<std::uint32_t>(result));change();return result;
    }
    std::int32_t seek(void* file,std::int32_t offset,std::int32_t origin) {
        before(2,file,static_cast<std::uint32_t>(offset),static_cast<std::uint32_t>(origin));
        change();return trial%3?-1:0;
    }
    FileStreamOpenPositionHooks position_hooks() {
        return {this,[](void* p,void* f){return static_cast<Fixture*>(p)->tell(f);},
            [](void* p,void* f,std::int32_t o,std::int32_t w){return static_cast<Fixture*>(p)->seek(f,o,w);}};
    }
    std::uint8_t run_open() {
        auto h=position_hooks();
        if(archive)return finish_archive_file_stream_open(stream,buffer,reinterpret_cast<void*>(0x5cd5fc),h);
        return finish_direct_file_stream_open(stream,buffer,reinterpret_cast<void*>(0x5cd5fc),h);
    }
};
}
