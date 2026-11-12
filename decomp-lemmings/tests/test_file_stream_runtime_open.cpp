#include "file_stream_runtime_open_fixture.hpp"
#include <iostream>
namespace {void require(bool value,const char* message){if(!value){std::cerr<<message<<'\n';std::exit(1);}}}
int main() {
    using namespace file_runtime_open_test;
    Fixture direct(0);direct.stream.mode_130=1;const auto cursor=direct.stream.cursor_120;
    require(direct.run_open()==1&&direct.tells==2&&direct.steps==4,"direct open tell/seek call count");
    require(direct.events[0]==1&&direct.events[10]==2&&direct.events[19]==1&&direct.events[29]==2,"direct open effect order");
    require(direct.stream.size_10==0xffffffffu&&direct.stream.cursor_120==cursor,"negative tell preserved as size and cursor not reset");
    require(direct.events[12]==0&&direct.events[13]==2&&direct.events[31]==0&&direct.events[32]==0,"seek end then restore saved position");
    require(direct.stream.ended_11c==0&&direct.stream.opened_12c==1&&direct.buffer.next_5d55fc==reinterpret_cast<void*>(0x5cd5fc)&&direct.buffer.end_5d5600==direct.buffer.next_5d55fc,"success flags and shared reset");
    Fixture archive(1);const auto size=archive.stream.size_10,position=archive.stream.cursor_120;const auto buffer=archive.buffer;
    require(archive.run_open()==1&&archive.steps==1&&archive.tells==0,"archive suffix uses one seek and ignores its error");
    require(archive.events[2]==0xffffffffu&&archive.events[3]==0&&archive.stream.size_10==size&&archive.stream.cursor_120==position,"archive offset signed bit pattern, size/cursor preservation");
    require(archive.buffer.next_5d55fc==buffer.next_5d55fc&&archive.buffer.end_5d5600==buffer.end_5d5600,"non-read mode leaves shared buffer");
    Fixture live(14);const auto live_cursor=live.stream.cursor_120;live.run_open();
    require(live.events[1]==0x11110000&&live.events[11]==0x11110004&&live.events[20]==0x11110008&&live.events[30]==0x1111000c,"each CRT call rereads handle");
    require(live.stream.size_10==0xabcd0004&&live.stream.cursor_120==live_cursor+52u,"last seek callback size/cursor mutations persist");
    std::cout<<"file open finalization ordering, errors, live fields and buffer tests passed\n";
}
