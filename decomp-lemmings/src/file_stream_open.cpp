#include "lemmings/file_stream_open.hpp"
#include <array>

namespace lemmings {
namespace {
char* copy_text(char* destination,const char* source) {
    while((*destination=*source)!='\0'){++destination;++source;}
    return destination;
}
const char* mode_text(std::int32_t mode) {
    switch(mode){
    case 1:return "rb";
    case 2:return "wb";
    case 3:return "wb+";
    case 6:return "ab";
    case 7:return "ab+";
    case 10:return "wt";
    }
    // Caller precondition; no invented original outcome for unknown modes.
    return nullptr;
}
}
std::uint8_t open_file_stream(FileStreamRuntime& stream,std::int32_t mode,
    std::uint8_t required,const FileStreamOpenGlobals& globals,const FileStreamOpenHooks& hooks) {
    hooks.prepare_47dd54(hooks.context,mode,nullptr);
    bool selected=false;
    if(globals.paths.prefix_enabled_5d5604==0){
        bool retry;
        do {
            retry=false;
            if(mode!=1 || hooks.exists_47e89b(hooks.context,stream.name_14.data())!=0){
                stream.archive_11d=0;
                selected=true;
            } else {
                if(hooks.archives_ready_47ef48(hooks.context)==0)
                    hooks.reset_archives_47ef92(hooks.context);
                const auto* entry=hooks.find_archive_47f759(hooks.context,stream.name_14.data());
                if(entry){
                    stream.size_10=entry->size_104;
                    stream.archive_offset_124=entry->offset_108;
                    stream.archive_index_128=entry->archive_index_00;
                    stream.archive_11d=1;
                    selected=true;
                } else {
                    if(required!=0){
                        std::array<char,260> message;
                        char* end=copy_text(message.data(),"Cannot find required file '");
                        end=copy_text(end,stream.name_14.data());
                        copy_text(end,"'.  Click OK to try again, or cancel to exit.");
                        retry=hooks.message_box(hooks.context,message.data(),"Lemmings Revolution",0x2131)==1;
                    }
                    stream.archive_11d=0;
                    selected=false;
                }
            }
            if(retry){
                hooks.clear_registries_47e2f3(hooks.context);
                hooks.register_archives_47e23b(hooks.context);
                hooks.reset_archives_47efe6(hooks.context);
                hooks.reset_archives_47ef92(hooks.context);
            }
        } while(retry);
    } else {
        stream.archive_11d=0;
        selected=true;
    }
    if(!selected)return 0;
    stream.mode_130=static_cast<std::uint32_t>(mode);
    const char* text=mode_text(mode);
    const FileStreamOpenPositionHooks position_hooks{hooks.context,hooks.tell_49db9d,hooks.seek_49dcf5};
    if(stream.archive_11d!=0){
        if(stream.file_118==nullptr){
            const char* archive=hooks.archive_name(hooks.context,stream.archive_index_128);
            stream.file_118=hooks.open_49db6a(hooks.context,archive,text,0x20);
        }
        return finish_archive_file_stream_open(stream,globals.buffer,globals.buffer_base,position_hooks);
    }
    if(stream.file_118!=nullptr){
        hooks.close_49da2c(hooks.context,stream.file_118);
        stream.file_118=nullptr;
    }
    stream.file_118=hooks.open_49db6a(hooks.context,stream.name_14.data(),text,0x20);
    if(stream.file_118==nullptr){stream.ended_11c=1;return 0;}
    return finish_direct_file_stream_open(stream,globals.buffer,globals.buffer_base,position_hooks);
}
}
