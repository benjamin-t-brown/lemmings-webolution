#include "lemmings/file_stream_runtime.hpp"
#include <bit>

namespace lemmings {
namespace {
char* copy_string(char* destination,const char* source) {
    while((*destination=*source)!='\0'){++destination;++source;}
    return destination;
}
void initialize_file_fields(FileStreamRuntime& stream) {
    stream.file_118=nullptr;
    stream.cursor_120=0;
    stream.ended_11c=1;
    stream.opened_12c=0;
}
}
FileStreamRuntime* construct_file_stream(FileStreamRuntime& stream) {
    initialize_file_fields(stream);
    return &stream;
}
void set_file_stream_name(FileStreamRuntime& stream,const char* name,const FileStreamPathEnvironment& environment) {
    if(environment.prefix_enabled_5d5604==0)copy_string(stream.name_14.data(),name);
    else {
        std::array<char,260> temporary;
        char* end=copy_string(temporary.data(),environment.prefix_5d5628);
        copy_string(end,name);
        copy_string(stream.name_14.data(),temporary.data());
    }
}
FileStreamRuntime* construct_named_file_stream(FileStreamRuntime& stream,const char* name,const FileStreamPathEnvironment& environment) {
    set_file_stream_name(stream,name,environment);
    initialize_file_fields(stream);
    return &stream;
}
std::uint32_t read_file_stream(FileStreamRuntime& stream,std::uint32_t count,void* destination,const FileStreamRuntimeHooks& hooks) {
    const auto actual=hooks.read_49da82(hooks.context,destination,count,stream.file_118);
    stream.cursor_120+=actual;
    return actual;
}
void close_file_stream(FileStreamRuntime& stream,const FileStreamRuntimeHooks& hooks) {
    if(stream.file_118){
        hooks.close_49da2c(hooks.context,stream.file_118);
        stream.file_118=nullptr;
        stream.ended_11c=1;
        stream.cursor_120=0;
        stream.opened_12c=0;
    }
}
void destroy_file_stream(FileStreamRuntime& stream,const FileStreamRuntimeHooks& hooks) {
    close_file_stream(stream,hooks);
}
std::uint32_t file_stream_size(const FileStreamRuntime& stream) {return stream.size_10;}
std::uint8_t file_stream_is_open(const FileStreamRuntime& stream) {return stream.opened_12c;}
std::uint8_t file_stream_at_end(const FileStreamRuntime& stream,const FileStreamBufferPointers& buffer) {
    if(stream.mode_130==1)return buffer.next_5d55fc==buffer.end_5d5600 && stream.ended_11c!=0;
    return stream.ended_11c;
}
void reset_file_stream_buffer(FileStreamBufferPointers& buffer,const void* base) {
    buffer.end_5d5600=base;
    buffer.next_5d55fc=buffer.end_5d5600;
}
namespace {
std::uint8_t finish_open_flags(FileStreamRuntime& stream,FileStreamBufferPointers& buffer,const void* base) {
    stream.ended_11c=0;
    if(stream.mode_130==1)reset_file_stream_buffer(buffer,base);
    stream.opened_12c=1;
    return 1;
}
}
std::uint8_t finish_direct_file_stream_open(FileStreamRuntime& stream,FileStreamBufferPointers& buffer,
    const void* base,const FileStreamOpenPositionHooks& hooks) {
    const auto position=hooks.tell_49db9d(hooks.context,stream.file_118);
    hooks.seek_49dcf5(hooks.context,stream.file_118,0,2);
    stream.size_10=static_cast<std::uint32_t>(hooks.tell_49db9d(hooks.context,stream.file_118));
    hooks.seek_49dcf5(hooks.context,stream.file_118,position,0);
    return finish_open_flags(stream,buffer,base);
}
std::uint8_t finish_archive_file_stream_open(FileStreamRuntime& stream,FileStreamBufferPointers& buffer,
    const void* base,const FileStreamOpenPositionHooks& hooks) {
    hooks.seek_49dcf5(hooks.context,stream.file_118,std::bit_cast<std::int32_t>(stream.archive_offset_124),0);
    return finish_open_flags(stream,buffer,base);
}
}
