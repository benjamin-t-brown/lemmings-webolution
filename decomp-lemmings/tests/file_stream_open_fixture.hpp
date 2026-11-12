#pragma once
#include "file_stream_runtime_fixture.hpp"
#include "lemmings/file_stream_open.hpp"
#include <bit>
#include <sstream>
#include <string>

namespace file_open_test {
using namespace lemmings;
struct OpenFixture: file_runtime_test::Fixture {
    FileStreamPathEnvironment paths{};
    FileStreamArchiveMatch match;
    std::array<unsigned char,0x10c> raw_match{};
    std::array<unsigned char,0x200> raw_archive{};
    void* archive_slot=nullptr;
    std::vector<std::string> events;
    unsigned scenario,script,steps=0,attempts=0,dialogs=0,tells=0;
    std::int32_t mode;
    std::uint8_t required;
    explicit OpenFixture(unsigned t,bool n=false):file_runtime_test::Fixture(t,n),scenario(t%12),script((t/12)%8),mode(1),required(scenario>=5&&scenario<=8?255:0) {
        constexpr int modes[]={1,2,3,6,7,10};
        if(scenario==0||scenario==1||scenario==9)mode=modes[(t/96)%6];
        if(scenario==11)mode=modes[1+(t/96)%5];
        paths.prefix_enabled_5d5604=scenario==0?255:0;
        stream.file_118=(scenario==3||(t/576)%2)?reinterpret_cast<void*>(0x11110000):nullptr;
        stream.cursor_120=0xfffffff0u;stream.mode_130=0xa5a5a5a5;stream.ended_11c=128;stream.opened_12c=255;
        std::strcpy(stream.name_14.data(),"Sprites\\OpenProbe.spr");
        match={static_cast<std::uint32_t>(t%3),0x12345678u+t,0x80000000u+t};
        std::strcpy(reinterpret_cast<char*>(raw_archive.data()+0x38),"Archive\\Selected.box");archive_slot=raw_archive.data();raw=encode();
    }
    void begin(unsigned event,const std::string& arguments={}) {
        std::ostringstream output;output<<event<<':'<<arguments<<':'<<std::hex
            <<reinterpret_cast<std::uintptr_t>(stream.file_118)<<':'<<stream.cursor_120<<':'<<stream.size_10<<':'<<stream.mode_130<<':'
            <<unsigned(stream.archive_11d)<<':'<<unsigned(stream.ended_11c)<<':'<<unsigned(stream.opened_12c)<<':'<<unsigned(paths.prefix_enabled_5d5604)<<':'
            <<reinterpret_cast<std::uintptr_t>(buffer.next_5d55fc)<<':'<<reinterpret_cast<std::uintptr_t>(buffer.end_5d5600);
        events.push_back(output.str());++steps;
    }
    void mutate(unsigned event) {
        if((script==1||script==7)&&event>=10)stream.file_118=reinterpret_cast<void*>(std::uintptr_t(0x11110000+steps*4));
        if(script==2||script==7)stream.name_14[0]=static_cast<char>('A'+steps%26);
        if(script==3||script==7){stream.archive_11d=0x80;stream.mode_130=steps%2?1:7;stream.ended_11c=0x81;stream.opened_12c=0xfe;}
        if(script==4||script==7){stream.cursor_120+=37;stream.size_10=0xabcd0000+steps;stream.archive_offset_124^=0x45670000u;}
        if((script==5||script==7)&&event==6)paths.prefix_enabled_5d5604^=255;
        if(script==6&&event==12)stream.file_118=reinterpret_cast<void*>(0x22220000);
        if(script==7){buffer.next_5d55fc=reinterpret_cast<void*>(std::uintptr_t(0x2000+steps));buffer.end_5d5600=reinterpret_cast<void*>(0x3000);}
    }
    void prepare(std::int32_t argument,const char* output){if(output)std::abort();begin(1,std::to_string(argument));mutate(1);}
    std::uint8_t exists(const char* name){begin(2,name);++attempts;mutate(2);return scenario==1||scenario==9||(scenario==7&&attempts>=3)?255:0;}
    std::uint8_t ready(){begin(3);mutate(3);return trial/96%2?255:0;}
    void reset92(){begin(4);mutate(4);}
    const FileStreamArchiveMatch* find(const char* name){begin(5,name);mutate(5);return scenario==2||scenario==3||scenario==10||(scenario==6&&attempts>=2)?&match:nullptr;}
    std::int32_t message(const char* text,const char* caption,std::uint32_t type){
        begin(6,std::string(text)+'|'+caption+'|'+std::to_string(type));++dialogs;mutate(6);
        if(scenario==6||scenario==7||(scenario==8&&dialogs<=3))return 1;
        return trial/96%2?2:0;
    }
    void clear(){begin(7);mutate(7);}
    void register_archives(){begin(8);mutate(8);}
    void resetE6(){begin(9);mutate(9);}
    const char* archive_name(std::uint32_t index){begin(10,std::to_string(index));mutate(10);return reinterpret_cast<const char*>(raw_archive.data()+0x38);}
    void* open(const char* name,const char* text,std::uint32_t share){begin(11,std::string(name)+'|'+text+'|'+std::to_string(share));mutate(11);return scenario==9||scenario==10?nullptr:reinterpret_cast<void*>(0x33330000);}
    std::int32_t close(void* file){begin(12,std::to_string(reinterpret_cast<std::uintptr_t>(file)));mutate(12);return -1;}
    std::int32_t tell(void* file){begin(13,std::to_string(reinterpret_cast<std::uintptr_t>(file)));++tells;mutate(13);constexpr std::uint32_t values[]={0,0xffffffffu,0x80000000u,37};return std::bit_cast<std::int32_t>(values[(trial/96+tells)%4]);}
    std::int32_t seek(void* file,std::int32_t offset,std::int32_t origin){begin(14,std::to_string(reinterpret_cast<std::uintptr_t>(file))+'|'+std::to_string(offset)+'|'+std::to_string(origin));mutate(14);return trial%3?-1:0;}
    FileStreamOpenGlobals globals(){return {paths,buffer,reinterpret_cast<void*>(0x5cd5fc)};}
    FileStreamOpenHooks open_hooks(){return {this,
        [](void* p,std::int32_t m,const char* o){static_cast<OpenFixture*>(p)->prepare(m,o);},
        [](void* p,const char* n){return static_cast<OpenFixture*>(p)->exists(n);},
        [](void* p){return static_cast<OpenFixture*>(p)->ready();},
        [](void* p){static_cast<OpenFixture*>(p)->reset92();},
        [](void* p,const char* n){return static_cast<OpenFixture*>(p)->find(n);},
        [](void* p){static_cast<OpenFixture*>(p)->clear();},
        [](void* p){static_cast<OpenFixture*>(p)->register_archives();},
        [](void* p){static_cast<OpenFixture*>(p)->resetE6();},
        [](void* p,std::uint32_t i){return static_cast<OpenFixture*>(p)->archive_name(i);},
        [](void* p,const char* t,const char* c,std::uint32_t f){return static_cast<OpenFixture*>(p)->message(t,c,f);},
        [](void* p,const char* n,const char* m,std::uint32_t s){return static_cast<OpenFixture*>(p)->open(n,m,s);},
        [](void* p,void* f){return static_cast<OpenFixture*>(p)->close(f);},
        [](void* p,void* f){return static_cast<OpenFixture*>(p)->tell(f);},
        [](void* p,void* f,std::int32_t o,std::int32_t w){return static_cast<OpenFixture*>(p)->seek(f,o,w);}};}
    std::uint8_t run_open(){return open_file_stream(stream,mode,required,globals(),open_hooks());}
};
}
