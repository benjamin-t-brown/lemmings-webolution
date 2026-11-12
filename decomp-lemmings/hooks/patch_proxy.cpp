// Replacement patch.dll for the installed Lemmings Revolution.
//
// The shipped patch.dll exports exactly two entry points, DirectDrawCreate and
// DirectDrawEnumerateA, and the executable imports both statically. That makes
// it the cheapest injection point in the installation: this build forwards both
// calls to the original library (renamed patch_orig.dll, so whatever
// DirectDraw wrapper is installed keeps working) and uses its own DllMain to
// install recovered-routine detours.
//
// Forwarding is deliberately lazy. Calling LoadLibrary from DllMain runs under
// the loader lock, so the original library is resolved on the first forwarded
// call instead.

#include "hook_engine.hpp"
#include "hook_log.hpp"
#include "coverage.hpp"
#include "hooks.hpp"

#include <windows.h>

#include <cstdio>
#include <cstring>

namespace {

constexpr const char* kOriginalLibrary = "patch_orig.dll";

HMODULE g_self = nullptr;
char g_directory[MAX_PATH] = {};

HMODULE g_original = nullptr;
CRITICAL_SECTION g_original_lock;
bool g_lock_ready = false;

using DirectDrawCreateFn = HRESULT(WINAPI*)(void*, void*, void*);
using DirectDrawEnumerateAFn = HRESULT(WINAPI*)(void*, void*);

// GetProcAddress returns a generic FARPROC, so every use is a function-type
// cast. Routing through void* keeps that intent explicit in one place.
template <typename Fn>
Fn proc_cast(FARPROC procedure) {
    return reinterpret_cast<Fn>(reinterpret_cast<void*>(procedure));
}

DirectDrawCreateFn g_create = nullptr;
DirectDrawEnumerateAFn g_enumerate = nullptr;

void capture_directory() {
    char path[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameA(g_self, path, sizeof(path));
    if (length == 0 || length >= sizeof(path)) {
        std::snprintf(g_directory, sizeof(g_directory), ".");
        return;
    }
    char* const separator = std::strrchr(path, '\\');
    if (separator == nullptr) {
        std::snprintf(g_directory, sizeof(g_directory), ".");
        return;
    }
    *separator = '\0';
    std::snprintf(g_directory, sizeof(g_directory), "%s", path);
}

// Loads patch_orig.dll from this library's own directory, never from the
// process search path.
bool ensure_original() {
    if (!g_lock_ready) {
        return false;
    }
    EnterCriticalSection(&g_original_lock);
    if (g_original == nullptr) {
        char path[MAX_PATH * 2] = {};
        std::snprintf(path, sizeof(path), "%s\\%s", g_directory, kOriginalLibrary);
        g_original = LoadLibraryA(path);
        if (g_original == nullptr) {
            lrhook::log_line("failed to load %s (error %lu); DirectDraw forwarding "
                             "is unavailable",
                             path, GetLastError());
        } else {
            g_create = proc_cast<DirectDrawCreateFn>(
                GetProcAddress(g_original, "DirectDrawCreate"));
            g_enumerate = proc_cast<DirectDrawEnumerateAFn>(
                GetProcAddress(g_original, "DirectDrawEnumerateA"));
            lrhook::log_line("forwarding to %s (DirectDrawCreate=%p, "
                             "DirectDrawEnumerateA=%p)",
                             path, reinterpret_cast<void*>(g_create),
                             reinterpret_cast<void*>(g_enumerate));
        }
    }
    LeaveCriticalSection(&g_original_lock);
    return g_original != nullptr;
}

bool selftests_enabled() {
    char value[8] = {};
    const DWORD length =
        GetEnvironmentVariableA("LEMMINGS_HOOK_SELFTEST", value, sizeof(value));
    return !(length > 0 && length < sizeof(value) && std::strcmp(value, "0") == 0);
}

// Coverage tracing and the routine detours both overwrite a function's first
// bytes, so only one of them can be installed. LEMMINGS_HOOK_COVERAGE names
// the target list and selects tracing.
bool coverage_requested(char* list, std::size_t size) {
    const DWORD length = GetEnvironmentVariableA("LEMMINGS_HOOK_COVERAGE",
                                                 list, static_cast<DWORD>(size));
    return length > 0 && length < size;
}

void install_hooks() {
    if (!lrhook::image_base_matches()) {
        return;
    }

    char target_list[MAX_PATH * 2] = {};
    if (coverage_requested(target_list, sizeof(target_list))) {
        char trace[MAX_PATH * 2] = {};
        std::snprintf(trace, sizeof(trace), "%s\\lemmings_coverage.txt",
                      g_directory);
        lrhook::log_line("coverage tracing requested; routine detours disabled");
        lrhook::install_coverage(target_list, trace);
        return;
    }

    const lrhook::Mode mode = lrhook::configured_mode();
    lrhook::log_line("requested hook mode: %s", lrhook::mode_name(mode));

    unsigned installed = 0;
    // First: it binds the recovered allocator to the game's CRT, which every
    // later allocating routine depends on.
    if (lrhook::install_native_int_array_hook(mode)) {
        ++installed;
    }
    if (lrhook::install_collision_grid_hook(mode)) {
        ++installed;
    }
    if (lrhook::install_cylinder_coordinate_hook(mode)) {
        ++installed;
    }
    lrhook::log_line("%u detour(s) installed", installed);

    if (installed == 0) {
        return;
    }

    // The original code is mapped at its recorded address in this process, so
    // the startup sweep is a true original-byte comparison and does not depend
    // on reaching gameplay. Set LEMMINGS_HOOK_SELFTEST=0 to skip it.
    if (!selftests_enabled()) {
        return;
    }

    // Only sweeps that allocate nothing may run here. patch.dll is a static
    // import, so this is still before the executable's CRT startup; touching
    // the game's heap now is fatal. The allocating sweeps are deferred to
    // run_deferred_selftests below.
    const bool grid = lrhook::selftest_collision_grid();
    const bool cylinder = lrhook::selftest_cylinder_coordinate();
    lrhook::log_line("startup selftest: %s", (grid && cylinder) ? "PASS" : "FAIL");
}

// Runs once, on the first DirectDraw call. By then WinMain is executing, so the
// executable's CRT is initialised and its heap is usable.
void run_deferred_selftests() {
    static bool done = false;
    if (done) {
        return;
    }
    done = true;

    if (!selftests_enabled()) {
        return;
    }
    // In coverage mode no detour was installed, so there is no trampoline to
    // compare against and nothing to report.
    char unused[MAX_PATH * 2] = {};
    if (coverage_requested(unused, sizeof(unused))) {
        return;
    }
    lrhook::log_line("CRT is up; running deferred selftests that use the game heap");
    const bool array = lrhook::selftest_native_int_array();
    lrhook::log_line("deferred selftest: %s", array ? "PASS" : "FAIL");
}

} // namespace

extern "C" {

HRESULT WINAPI DirectDrawCreate(void* guid, void* dd, void* unknown_outer) {
    if (!ensure_original() || g_create == nullptr) {
        return E_FAIL;
    }
    run_deferred_selftests();
    return g_create(guid, dd, unknown_outer);
}

HRESULT WINAPI DirectDrawEnumerateA(void* callback, void* context) {
    if (!ensure_original() || g_enumerate == nullptr) {
        return E_FAIL;
    }
    return g_enumerate(callback, context);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        g_self = instance;
        DisableThreadLibraryCalls(instance);
        InitializeCriticalSection(&g_original_lock);
        g_lock_ready = true;
        capture_directory();
        lrhook::log_open(g_directory);
        lrhook::log_line("module directory: %s", g_directory);
        install_hooks();
        break;
    case DLL_PROCESS_DETACH:
        lrhook::report_coverage();
        lrhook::report_native_int_array_hook();
        lrhook::report_cylinder_coordinate_hook();
        lrhook::report_collision_grid_hook();
        lrhook::log_close();
        break;
    default:
        break;
    }
    return TRUE;
}

} // extern "C"
