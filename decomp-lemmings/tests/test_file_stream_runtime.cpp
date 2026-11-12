#include "file_stream_runtime_fixture.hpp"
#include <iostream>
#include <string>
namespace {void require(bool value,const char* message){if(!value){std::cerr<<message<<'\n';std::exit(1);}}}
int main(){
    using namespace file_runtime_test;
    Fixture defaults(0);auto before=defaults.encode();construct_file_stream(defaults.stream);auto after=defaults.encode();
    require(!defaults.stream.file_118&&defaults.stream.cursor_120==0&&defaults.stream.ended_11c==1&&defaults.stream.opened_12c==0,"constructor fields");
    for(unsigned i=0;i<after.size();++i)if(!((i>=0x118&&i<0x11d)||(i>=0x120&&i<0x124)||i==0x12c))require(after[i]==before[i],"constructor preserves uninitialized fields");
    std::strcpy(defaults.stream.name_14.data(),"relative");set_file_stream_name(defaults.stream,defaults.stream.name_14.data(),{1,"root/"});
    require(std::string(defaults.stream.name_14.data())=="root/relative","prefix concatenation with exact self-alias");
    Fixture read(3);read.mutation=3;read.stream.cursor_120=4;read.stream.size_10=1;read.count=5;const auto h=read.hooks();
    require(read_file_stream(read.stream,5,read.destination.data(),h)==5&&read.stream.cursor_120==3,"read adds actual bytes to callback-mutated cursor with wrap");
    require(read.stream.ended_11c==128&&read.stream.opened_12c==255,"read preserves callback flag writes");
    read.stream.file_118=nullptr;before=read.encode();read.trace.clear();close_file_stream(read.stream,h);
    require(read.encode()==before&&read.trace.empty(),"null-handle close must be a complete no-op");
    read.stream.file_118=reinterpret_cast<void*>(0x1234);close_file_stream(read.stream,h);
    require(!read.stream.file_118&&read.stream.ended_11c==1&&read.stream.cursor_120==0&&read.stream.opened_12c==0&&read.stream.size_10==17,"close resets selected fields after callback despite failure");
    read.stream.mode_130=1;read.stream.ended_11c=128;
    require(file_stream_at_end(read.stream,{reinterpret_cast<void*>(1),reinterpret_cast<void*>(2)})==0&&file_stream_at_end(read.stream,{nullptr,nullptr})==1,"read mode EOF requires empty shared buffer and normalizes flag");
    read.stream.mode_130=2;require(file_stream_at_end(read.stream,{nullptr,nullptr})==128,"other modes retain raw EOF byte");
    std::cout<<"file runtime partial initialization, naming, cursor wrap and close/query tests passed\n";
}
