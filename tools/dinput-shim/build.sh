#!/usr/bin/env bash
# Builds the 32-bit dinput.dll proxy. The game is a 32-bit process, so the DLL
# must be 32-bit too.  MSYS2:  pacman -S mingw-w64-i686-gcc
#
# The mingw32 bin directory must be on PATH for gcc to find its own ld/collect2.
set -euo pipefail

MINGW32_BIN="${MINGW32_BIN:-/c/progs/msys2/mingw32/bin}"
export PATH="$MINGW32_BIN:$PATH"
CC="${CC:-$MINGW32_BIN/i686-w64-mingw32-gcc.exe}"

cd "$(dirname "$0")"
"$CC" -shared -O2 -Wall -Wextra -o dinput.dll dinput_shim.c dinput.def \
      -static-libgcc -Wl,--enable-stdcall-fixup -luser32 -lkernel32
echo "built: $(pwd)/dinput.dll"
