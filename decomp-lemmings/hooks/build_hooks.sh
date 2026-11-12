#!/usr/bin/env bash
# Builds the 32-bit replacement patch.dll for the installed game.
#
# The DLL is loaded into the original MSVC 6 executable, so it must not pull in
# any runtime DLL of its own and must keep x87 floating-point semantics.

# The mingw32 driver needs its own bin directory on PATH to start at all,
# in POSIX form so the MSYS shell actually searches it.
gxx_dir="$(dirname "${LEMMINGS_MINGW32_GXX:-C:/progs/msys2/mingw32/bin/g++.exe}")"
if command -v cygpath >/dev/null 2>&1; then
    gxx_dir="$(cygpath -u "$gxx_dir")"
fi
export PATH="$gxx_dir:$PATH"
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
gxx="${LEMMINGS_MINGW32_GXX:-C:/progs/msys2/mingw32/bin/g++.exe}"
lib="${LEMMINGS_RECOVERED_LIB:-$root/build/x86/liblemmings_recovered.a}"
out="${1:-$root/build/hooks}"

if [ ! -f "$lib" ]; then
    echo "missing 32-bit recovered library: $lib" >&2
    echo "configure and build the x86 tree first (see README)" >&2
    exit 1
fi

mkdir -p "$out"

# libstdc++'s exception machinery references pthread_mutex_* and pthread_once.
# Resolving those from the import library would make the DLL depend on
# libwinpthread-1.dll, which the game cannot find. Link the archive by path so
# they resolve statically; -Wl,-Bstatic -lwinpthread is not enough, because the
# driver has already seen the import library by then.
pthread="${LEMMINGS_WINPTHREAD_A:-$gxx_dir/../lib/libwinpthread.a}"
if [ ! -f "$pthread" ]; then
    echo "missing static winpthread archive: $pthread" >&2
    echo "set LEMMINGS_WINPTHREAD_A to its location" >&2
    exit 1
fi

"$gxx" -std=c++23 -O2 -DNDEBUG -Wall -Wextra \
    -mfpmath=387 \
    -I "$root/include" -I "$root/hooks" \
    -shared -o "$out/patch.dll" \
    "$root/hooks/patch_proxy.cpp" \
    "$root/hooks/hook_engine.cpp" \
    "$root/hooks/hook_log.cpp" \
    "$root/hooks/hook_config.cpp" \
    "$root/hooks/coverage.cpp" \
    "$root/hooks/hooks_collision_grid.cpp" \
    "$root/hooks/hooks_cylinder_coordinate.cpp" \
    "$root/hooks/hooks_native_int_array.cpp" \
    "$root/hooks/patch_proxy.def" \
    "$lib" \
    -static-libgcc -static-libstdc++ \
    -Wl,--whole-archive "$pthread" -Wl,--no-whole-archive \
    -Wl,--kill-at -Wl,--enable-stdcall-fixup

# The DLL is loaded by the original executable, which has no MinGW runtime on
# its search path. Anything beyond the system libraries below means the game
# silently fails to start, so assert it rather than discovering it at runtime.
# (Using try/catch previously pulled in libwinpthread-1.dll this way.)
allowed='^(KERNEL32|msvcrt|USER32|ADVAPI32|GDI32)\.dll$'
unexpected="$(objdump -p "$out/patch.dll" \
    | sed -n 's/^\s*DLL Name: //p' \
    | grep -Eiv "$allowed" || true)"
if [ -n "$unexpected" ]; then
    echo "ERROR: $out/patch.dll depends on non-system libraries:" >&2
    echo "$unexpected" >&2
    echo "the game cannot load it; link those statically" >&2
    exit 1
fi

echo "built $out/patch.dll"
objdump -p "$out/patch.dll" | sed -n 's/^\s*DLL Name: /  depends on /p'
