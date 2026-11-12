#include "coverage.hpp"

#include "hook_engine.hpp"
#include "hook_log.hpp"

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

// One-shot execution tracing of the whole executable.
//
// Static reachability over-counts badly: it cannot tell a routine the game
// runs every frame from one reached only by an unused menu. What matters for a
// standalone build is which functions actually execute, so this records that
// directly.
//
// Every recoverable procedure entry gets its first byte replaced with INT3. A
// vectored exception handler catches the trap, restores the original byte,
// rewinds EIP so the real instruction runs, and records the address. Each
// function therefore reports itself exactly once and costs nothing afterwards,
// which makes it cheap enough to leave running for a whole play session.
//
// Hits are appended to the trace file as they happen rather than dumped at
// exit, because terminating the game skips DLL_PROCESS_DETACH.
//
// Unwind and catch funclets are deliberately absent from the target list:
// patching those would corrupt exception dispatch rather than observe it.

namespace lrhook {
namespace {

struct Target {
    std::uintptr_t address;
    std::uint8_t original;
    volatile LONG claimed;  // interlocked, so only one thread records a hit
};

Target* g_targets = nullptr;
std::size_t g_count = 0;
void* g_handler = nullptr;
volatile LONG g_hits = 0;

char g_trace_path[MAX_PATH * 2] = {};
CRITICAL_SECTION g_write_lock;
bool g_lock_ready = false;

// Binary search: the target list is written in ascending address order.
Target* find(std::uintptr_t address) {
    std::size_t low = 0;
    std::size_t high = g_count;
    while (low < high) {
        const std::size_t mid = low + (high - low) / 2;
        if (g_targets[mid].address == address) {
            return &g_targets[mid];
        }
        if (g_targets[mid].address < address) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return nullptr;
}

void record(std::uintptr_t address, LONG ordinal) {
    if (!g_lock_ready) {
        return;
    }
    EnterCriticalSection(&g_write_lock);
    if (std::FILE* file = std::fopen(g_trace_path, "ab")) {
        std::fprintf(file, "%ld %08lx\n", ordinal,
                     static_cast<unsigned long>(address));
        std::fclose(file);
    }
    LeaveCriticalSection(&g_write_lock);
}

LONG CALLBACK on_exception(EXCEPTION_POINTERS* info) {
    const EXCEPTION_RECORD* record_info = info->ExceptionRecord;
    if (record_info->ExceptionCode != EXCEPTION_BREAKPOINT) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    const auto address =
        reinterpret_cast<std::uintptr_t>(record_info->ExceptionAddress);
    Target* const target = find(address);
    if (target == nullptr) {
        // Not one of ours: a debugger breakpoint, or the CRT's own.
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // Restore the displaced byte so the function runs normally from now on.
    // Writing it more than once is harmless, so the write is not serialised;
    // only the bookkeeping is.
    *reinterpret_cast<volatile std::uint8_t*>(address) = target->original;
    FlushInstructionCache(GetCurrentProcess(),
                          reinterpret_cast<void*>(address), 1);

    if (InterlockedExchange(&target->claimed, 1) == 0) {
        record(address, InterlockedIncrement(&g_hits));
    }

    // EIP sits after the INT3; rewind so the restored instruction executes.
    info->ContextRecord->Eip = static_cast<DWORD>(address);
    return EXCEPTION_CONTINUE_EXECUTION;
}

std::size_t load_targets(const char* path) {
    std::FILE* const file = std::fopen(path, "rb");
    if (file == nullptr) {
        log_line("coverage: cannot open target list %s", path);
        return 0;
    }

    std::size_t capacity = 8192;
    auto* targets = static_cast<Target*>(std::calloc(capacity, sizeof(Target)));
    std::size_t count = 0;
    char line[128] = {};
    while (targets != nullptr && std::fgets(line, sizeof(line), file) != nullptr) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        const auto address =
            static_cast<std::uintptr_t>(std::strtoul(line, nullptr, 16));
        if (address < 0x00401000u || address >= 0x004B0000u) {
            continue;
        }
        if (count == capacity) {
            capacity *= 2;
            auto* grown = static_cast<Target*>(
                std::realloc(targets, capacity * sizeof(Target)));
            if (grown == nullptr) {
                break;
            }
            targets = grown;
        }
        targets[count].address = address;
        targets[count].original = 0;
        targets[count].claimed = 0;
        ++count;
    }
    std::fclose(file);

    g_targets = targets;
    g_count = count;
    return count;
}

} // namespace

bool install_coverage(const char* target_list, const char* trace_path) {
    if (!image_base_matches()) {
        return false;
    }

    std::snprintf(g_trace_path, sizeof(g_trace_path), "%s", trace_path);
    InitializeCriticalSection(&g_write_lock);
    g_lock_ready = true;

    if (std::FILE* file = std::fopen(g_trace_path, "wb")) {
        std::fprintf(file, "# ordinal, entry address, in execution order\n");
        std::fclose(file);
    }

    const std::size_t count = load_targets(target_list);
    if (count == 0) {
        log_line("coverage: no targets loaded; not instrumenting");
        return false;
    }

    g_handler = AddVectoredExceptionHandler(1, &on_exception);
    if (g_handler == nullptr) {
        log_line("coverage: AddVectoredExceptionHandler failed (%lu)",
                 GetLastError());
        return false;
    }

    // One protection change for the whole range, then patch. The pages stay
    // writable: this is a diagnostic build and the handler writes to them.
    auto* const low = reinterpret_cast<void*>(g_targets[0].address);
    const SIZE_T span = (g_targets[count - 1].address - g_targets[0].address) + 16;
    DWORD previous = 0;
    if (VirtualProtect(low, span, PAGE_EXECUTE_READWRITE, &previous) == 0) {
        log_line("coverage: VirtualProtect over %p..+%lu failed (%lu)", low,
                 static_cast<unsigned long>(span), GetLastError());
        RemoveVectoredExceptionHandler(g_handler);
        g_handler = nullptr;
        return false;
    }

    std::size_t patched = 0;
    for (std::size_t i = 0; i < count; ++i) {
        auto* const site = reinterpret_cast<std::uint8_t*>(g_targets[i].address);
        g_targets[i].original = *site;
        if (g_targets[i].original == 0xCC) {
            // Already a breakpoint: leave it and do not claim it as ours.
            g_targets[i].claimed = 1;
            continue;
        }
        *site = 0xCC;
        ++patched;
    }
    FlushInstructionCache(GetCurrentProcess(), low, span);

    log_line("coverage: %u of %u entries instrumented across %#lx..%#lx",
             static_cast<unsigned>(patched), static_cast<unsigned>(count),
             static_cast<unsigned long>(g_targets[0].address),
             static_cast<unsigned long>(g_targets[count - 1].address));
    log_line("coverage: trace -> %s", g_trace_path);
    return true;
}

void report_coverage() {
    if (g_count == 0) {
        return;
    }
    log_line("coverage: %ld of %u instrumented entries executed", g_hits,
             static_cast<unsigned>(g_count));
}

} // namespace lrhook
