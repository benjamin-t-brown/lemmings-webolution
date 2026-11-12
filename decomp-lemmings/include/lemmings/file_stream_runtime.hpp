#pragma once
#include <array>
#include <cstdint>

namespace lemmings {
struct FileStreamRuntime {
    std::array<std::uint8_t,0x10> unknown_00{};
    std::uint32_t size_10=0;
    std::array<char,260> name_14{};
    void* file_118=nullptr;
    std::uint8_t ended_11c=0,archive_11d=0;
    std::array<std::uint8_t,2> unknown_11e{};
    std::uint32_t cursor_120=0,archive_offset_124=0,archive_index_128=0;
    std::uint8_t opened_12c=0;
    std::array<std::uint8_t,3> unknown_12d{};
    std::uint32_t mode_130=0;
};
struct FileStreamPathEnvironment {
    std::uint8_t prefix_enabled_5d5604=0;
    const char* prefix_5d5628=nullptr;
};
struct FileStreamBufferPointers {
    const void* next_5d55fc=nullptr;
    const void* end_5d5600=nullptr;
};
struct FileStreamRuntimeHooks {
    void* context=nullptr;
    // Original fread(destination, 1, count, captured FILE*). The original CRT
    // implementation/OS stream remains external. Return actual byte count.
    std::uint32_t (*read_49da82)(void*,void* destination,std::uint32_t count,void* file);
    // Original fclose(captured FILE*); its return code is ignored.
    std::int32_t (*close_49da2c)(void*,void* file);
};

// Normal-return original constructors. Only the named constructor writes the
// filename; other unknown fields, size, archive metadata and mode are retained.
FileStreamRuntime* construct_file_stream(FileStreamRuntime&); // 47DB2F
FileStreamRuntime* construct_named_file_stream(FileStreamRuntime&,const char*,
    const FileStreamPathEnvironment&); // 47DB73
// 47E395: prefix/name or name must fit 260 bytes including NUL. No partial
// overlap; exact name self-alias is supported, including prefixed self-alias.
void set_file_stream_name(FileStreamRuntime&,const char*,const FileStreamPathEnvironment&);
// 47EEF7 and its 47DAC8 forwarding wrapper. Adds actual bytes to the live
// cursor after the callback, with uint32 wrap; no archive-size or EOF check.
std::uint32_t read_file_stream(FileStreamRuntime&,std::uint32_t,void*,const FileStreamRuntimeHooks&);
// 47E844. If file_118 is null, no fields change and no close is called.
void close_file_stream(FileStreamRuntime&,const FileStreamRuntimeHooks&);
// 47DC28 destructor calls close, then an empty base destructor (47EF3D).
void destroy_file_stream(FileStreamRuntime&,const FileStreamRuntimeHooks&);
std::uint32_t file_stream_size(const FileStreamRuntime&); // 47EE27
std::uint8_t file_stream_is_open(const FileStreamRuntime&); // 47EE8C
// 47EE38: mode 1 normalizes the byte and requires the shared buffer empty;
// other modes return the raw ended byte, including noncanonical values.
std::uint8_t file_stream_at_end(const FileStreamRuntime&,const FileStreamBufferPointers&);
// 47DAA9 resets end then next to the same explicit shared buffer base.
void reset_file_stream_buffer(FileStreamBufferPointers&,const void* buffer_base);

struct FileStreamOpenPositionHooks {
    void* context=nullptr;
    std::int32_t (*tell_49db9d)(void*,void* file);
    std::int32_t (*seek_49dcf5)(void*,void* file,std::int32_t offset,std::int32_t origin);
};
// Exact already-open blocks inside 47E40D, not complete open operations.
// Direct: 47E770..47E81B measures size and restores the captured position.
// Archive: 47E6CC..47E6F4 seeks to archive_offset, then joins 47E7E0..47E81B.
// Both ignore CRT seek errors, preserve the logical byte cursor, clear ended,
// reset shared buffer only for live mode 1, set opened, and return byte one.
std::uint8_t finish_direct_file_stream_open(FileStreamRuntime&,FileStreamBufferPointers&,
    const void* buffer_base,const FileStreamOpenPositionHooks&);
std::uint8_t finish_archive_file_stream_open(FileStreamRuntime&,FileStreamBufferPointers&,
    const void* buffer_base,const FileStreamOpenPositionHooks&);
}
