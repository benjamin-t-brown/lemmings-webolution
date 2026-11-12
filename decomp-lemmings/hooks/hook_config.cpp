#include "hook_config.hpp"

#include <windows.h>

#include <cstring>

namespace lrhook {

Mode configured_mode() {
    char value[32] = {};
    const DWORD length =
        GetEnvironmentVariableA("LEMMINGS_HOOK_MODE", value, sizeof(value));
    if (length == 0 || length >= sizeof(value)) {
        return Mode::Shadow;
    }
    if (_stricmp(value, "replace") == 0) {
        return Mode::Replace;
    }
    if (_stricmp(value, "verify") == 0) {
        return Mode::Verify;
    }
    return Mode::Shadow;
}

const char* mode_name(Mode mode) {
    switch (mode) {
    case Mode::Replace:
        return "replace";
    case Mode::Verify:
        return "verify";
    case Mode::Shadow:
        break;
    }
    return "shadow";
}

} // namespace lrhook
