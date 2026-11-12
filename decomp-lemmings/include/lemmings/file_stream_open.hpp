#pragma once
#include "lemmings/file_stream_runtime.hpp"

namespace lemmings {
// Known fields of the archive-entry result returned by 47F759. This portable
// view omits the unused name bytes between original +00 and +104.
struct FileStreamArchiveMatch {
    std::uint32_t archive_index_00=0,size_104=0,offset_108=0;
};
struct FileStreamOpenGlobals {
    FileStreamPathEnvironment& paths;
    FileStreamBufferPointers& buffer;
    const void* buffer_base;
};
struct FileStreamOpenHooks {
    void* context;
    void (*prepare_47dd54)(void*,std::int32_t mode,const char* optional_output);
    std::uint8_t (*exists_47e89b)(void*,const char* path);
    std::uint8_t (*archives_ready_47ef48)(void*);
    void (*reset_archives_47ef92)(void*);
    const FileStreamArchiveMatch* (*find_archive_47f759)(void*,const char* name);
    void (*clear_registries_47e2f3)(void*);
    void (*register_archives_47e23b)(void*);
    void (*reset_archives_47efe6)(void*);
    // Original 47F210 indexes a pointer slot; 47F0B0 returns its archive's name.
    const char* (*archive_name)(void*,std::uint32_t index);
    std::int32_t (*message_box)(void*,const char* text,const char* caption,std::uint32_t type);
    void* (*open_49db6a)(void*,const char* name,const char* mode,std::uint32_t share);
    std::int32_t (*close_49da2c)(void*,void* file);
    std::int32_t (*tell_49db9d)(void*,void* file);
    std::int32_t (*seek_49dcf5)(void*,void* file,std::int32_t offset,std::int32_t origin);
};

// Complete normal-return orchestration of 47E40D. Valid modes are 1,2,3,6,7,10;
// other original modes use uninitialized mode text and are outside this API.
// Hooks return normally, preserve live referenced storage, and may mutate the
// file/global views. A returned archive match must remain live and disjoint
// from the file while its fields are copied. When a required-file dialog is
// reached, its formatted message must fit 260 bytes including NUL.
// The original has no retry limit and does not check archive fopen/seek errors.
std::uint8_t open_file_stream(FileStreamRuntime&,std::int32_t mode,
    std::uint8_t required,const FileStreamOpenGlobals&,const FileStreamOpenHooks&);
}
