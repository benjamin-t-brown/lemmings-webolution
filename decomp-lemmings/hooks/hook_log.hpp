#pragma once

// Minimal append-only logging for the in-process hook harness. The log is the
// only output channel available while running inside the original executable,
// so it doubles as the Phase 1 verification record.

namespace lrhook {

// Opens <directory>\lemmings_hooks.log, or the path in LEMMINGS_HOOK_LOG when
// that variable is set. Safe to call once from DllMain.
void log_open(const char* directory);

void log_line(const char* format, ...);

void log_close();

} // namespace lrhook
