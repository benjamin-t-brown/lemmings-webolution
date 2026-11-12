#pragma once

// How a recovered routine participates while the original executable runs.
//
// The progression is deliberate: `shadow` proves the recovered code computes
// the same answer as the original without being able to affect the game,
// `verify` hands the recovered answer back while still checking it, and
// `replace` runs the recovered code alone.

namespace lrhook {

enum class Mode {
    // Call the original through the trampoline, call the recovered routine,
    // compare, return the original's result.
    Shadow,
    // Same comparison, but return the recovered result.
    Verify,
    // Call only the recovered routine.
    Replace,
};

// Reads LEMMINGS_HOOK_MODE (shadow | verify | replace). Defaults to Shadow.
Mode configured_mode();

const char* mode_name(Mode mode);

} // namespace lrhook
