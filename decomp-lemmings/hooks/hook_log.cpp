#include "hook_log.hpp"

#include <windows.h>

#include <cstdarg>
#include <cstdio>

namespace lrhook {
namespace {

char g_path[MAX_PATH * 2] = {};
CRITICAL_SECTION g_lock;
bool g_ready = false;

} // namespace

void log_open(const char* directory) {
    if (g_ready) {
        return;
    }
    InitializeCriticalSection(&g_lock);

    const DWORD override_length =
        GetEnvironmentVariableA("LEMMINGS_HOOK_LOG", g_path, sizeof(g_path));
    if (override_length == 0 || override_length >= sizeof(g_path)) {
        std::snprintf(g_path, sizeof(g_path), "%s\\lemmings_hooks.log",
                      directory != nullptr ? directory : ".");
    }
    g_ready = true;

    // Truncate on attach so each run produces a self-contained record.
    if (std::FILE* file = std::fopen(g_path, "wb")) {
        std::fclose(file);
    }

    SYSTEMTIME now = {};
    GetLocalTime(&now);
    log_line("--- lemmings hook harness attached %04u-%02u-%02u %02u:%02u:%02u ---",
             now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond);
}

void log_line(const char* format, ...) {
    if (!g_ready) {
        return;
    }
    EnterCriticalSection(&g_lock);
    if (std::FILE* file = std::fopen(g_path, "ab")) {
        std::va_list args;
        va_start(args, format);
        std::vfprintf(file, format, args);
        va_end(args);
        std::fputc('\n', file);
        std::fclose(file);
    }
    LeaveCriticalSection(&g_lock);
}

void log_close() {
    if (!g_ready) {
        return;
    }
    log_line("--- detached ---");
    g_ready = false;
    DeleteCriticalSection(&g_lock);
}

} // namespace lrhook
