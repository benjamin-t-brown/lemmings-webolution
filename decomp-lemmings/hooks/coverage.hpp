#pragma once

#include <cstddef>
#include <cstdint>

namespace lrhook {

// Replaces the first byte of every entry in `target_list` with INT3 and
// records each address the first time it traps, in execution order, to
// `trace_path`.
//
// `target_list` is the output of `tools/classify_inventory.py --addresses`:
// one hex address per line, ascending, with unwind funclets excluded. Patching
// a funclet would corrupt exception dispatch, so the list must not contain
// them.
//
// Mutually exclusive with the routine detours: both write over a function's
// first bytes. The caller installs one or the other, never both.
bool install_coverage(const char* target_list, const char* trace_path);

void report_coverage();

} // namespace lrhook
