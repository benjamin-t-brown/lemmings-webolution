#include "file_stream_open_fixture.hpp"
#include <iostream>
namespace {void require(bool value,const char* message){if(!value){std::cerr<<message<<'\n';std::exit(1);}}}
int main(){
    using namespace file_open_test;
    OpenFixture missing(4);const auto initial=missing.stream;
    require(missing.run_open()==0&&missing.dialogs==0&&missing.stream.archive_11d==0,"optional missing file returns without dialog");
    require(missing.stream.cursor_120==initial.cursor_120&&missing.stream.mode_130==initial.mode_130&&missing.stream.opened_12c==initial.opened_12c&&missing.stream.ended_11c==initial.ended_11c,"selection failure preserves file state beyond archive byte");
    OpenFixture retry(6);require(retry.run_open()==1&&retry.attempts==2&&retry.dialogs==1,"retry can resolve to archive");
    require(retry.stream.size_10==retry.match.size_104&&retry.stream.archive_offset_124==retry.match.offset_108&&retry.stream.archive_index_128==retry.match.archive_index_00,"archive match fields copied");
    std::string order;for(const auto& event:retry.events)order+=event.substr(0,event.find(':'))+',';
    require(order=="1,2,3,4,5,6,7,8,9,4,2,3,4,5,10,11,14,","required retry effect order");
    OpenFixture cancel(8);require(cancel.run_open()==0&&cancel.attempts==4&&cancel.dialogs==4,"retry remains live until dialog cancels");
    OpenFixture bypass_changed(66);require(bypass_changed.run_open()==1&&bypass_changed.attempts==2&&bypass_changed.paths.prefix_enabled_5d5604==255,"retry does not resample bypass flag");
    OpenFixture direct_failure(9);const auto cursor=direct_failure.stream.cursor_120;
    require(direct_failure.run_open()==0&&!direct_failure.stream.file_118&&direct_failure.stream.ended_11c==1&&direct_failure.stream.opened_12c==255&&direct_failure.stream.cursor_120==cursor,"direct fopen failure only marks ended");
    OpenFixture archive_failure(10);require(archive_failure.run_open()==1&&!archive_failure.stream.file_118&&archive_failure.stream.opened_12c==1,"archive fopen failure is passed unchecked to seek");
    require(archive_failure.events.back().starts_with("14:0|"),"archive seek receives null fopen result");
    constexpr const char* modes[]={"rb","wb","wb+","ab","ab+","wt"};
    for(unsigned i=0;i<6;++i){OpenFixture mode(i*96);require(mode.run_open()==1,"supported open mode succeeds");bool found=false;for(const auto& e:mode.events)if(e.starts_with("11:")&&e.find(std::string("|")+modes[i]+"|32:")!=std::string::npos)found=true;require(found,"mode maps to exact CRT string/share value");}
    OpenFixture prepared(0);auto hooks=prepared.open_hooks();hooks.prepare_47dd54=[](void* p,std::int32_t mode,const char* output){auto& f=*static_cast<OpenFixture*>(p);f.prepare(mode,output);f.paths.prefix_enabled_5d5604=0;};
    require(open_file_stream(prepared.stream,prepared.mode,prepared.required,prepared.globals(),hooks)==0&&prepared.attempts==1,"bypass flag is read after preparation");
    std::cout<<"file open mode, selection, retry, failure and live-global tests passed\n";
}
