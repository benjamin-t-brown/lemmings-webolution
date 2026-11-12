# Hybrid runtime: running recovered code inside the original game

This project's goal is a buildable executable, not a byte-identical rebuild of
the original. That choice makes a different workflow available: load the
recovered library into the running original executable and replace routines one
at a time, so there is a working game from the first function onward and
coverage becomes a dial rather than a gate.

This is the route OpenRCT2 and devilution took. It replaces the per-function
offline oracle as the primary verification tool, because the running game
supplies real inputs for free.

## Phases

| Phase | Content | State |
| --- | --- | --- |
| 0 | Injection harness: proxy `patch.dll`, detour engine, logging | Done |
| 1 | First detoured routine, verified against the original in-process | Done — `0x0040DE60` |
| 2 | Bulk port behind the harness; the running game is the oracle | Started — `0x0040DEC0` and `0x0041BC10` hooked, confirming the x87 and heap boundaries |
| 3 | Own `WinMain` and platform layer; drop the original executable | Open |

Phase 3 is gated on the software rasteriser noted in
[the original layout](ORIGINAL_LAYOUT.md), which would remove the Direct3D
immediate-mode dependency.

## Why `patch.dll`

The installation's `patch.dll` exports exactly two entry points,
`DirectDrawCreate` and `DirectDrawEnumerateA`, and the executable imports both
statically from it. That makes it the cheapest injection point available: no
launcher, no remote thread, no import-table surgery. The replacement forwards
both calls to the original library, renamed `patch_orig.dll`, so any DirectDraw
wrapper already installed keeps working unchanged.

It also means the harness owns the `IDirectDraw` the game receives, which is
how the renderer can later be replaced without touching `HardwarePC`.

## Detour mechanics

`hooks/hook_engine.cpp` writes a five-byte `JMP rel32` over a function entry
and relocates the displaced prologue into an allocated trampoline that jumps
back to `target + prologue_length`. The trampoline is callable with the
original calling convention, which is what makes shadow comparison possible.

Three safety conditions are checked before anything is written:

1. **Image base.** Recorded virtual addresses are only meaningful when the
   executable is mapped at `0x00400000`. The harness refuses to patch
   otherwise rather than guessing a delta.
2. **Prologue bytes.** The expected original bytes are compared first, so a
   different build of the executable is rejected instead of corrupted.
3. **Instruction boundary.** `prologue_length` must be at least 5 and must land
   on a real boundary. For `0x0040DE60` the prologue is
   `55 8b ec 51 89 4d fc` — `push ebp` / `mov ebp,esp` / `push ecx` /
   `mov [ebp-4],ecx` — whose boundaries sit at 4 and 7, so **7 bytes** are
   relocated, not 5.

The relocated bytes must contain no relative branches and must not be a branch
target from inside the function. Both hold for an ordinary prologue; a routine
whose first seven bytes are reached by a loop would need a different approach.

## Modes

`LEMMINGS_HOOK_MODE` selects how a hooked routine participates. The progression
is deliberate.

| Mode | Behaviour |
| --- | --- |
| `shadow` (default) | Call the original through the trampoline, call the recovered routine, compare, **return the original's result**. Cannot affect the game. |
| `verify` | Same comparison, but return the recovered result. |
| `replace` | Call only the recovered routine. |

`LEMMINGS_HOOK_SELFTEST=0` skips the startup sweep;
`LEMMINGS_HOOK_LOG` overrides the log path.

## Driving the game

The game reads all input through `IDirectInputDevice::GetDeviceState` and
ignores Windows input injection, so `SendInput` and `SendKeys` cannot reach it.
Gameplay evidence needs the `dinput.dll` shim from the sibling remake project,
which serves device state from a named shared-memory block and is inert until a
writer enables it.

```powershell
python tools/drive_hook_session.py --level MM_DIG_000 --seconds 60
python tools/drive_hook_session.py --list-levels
```

That script launches the game, confirms the shim is being polled, delegates
frontend navigation to the remake project's `goto_level.py`, holds inside the
level, and reads the harness counters back. Navigation is delegated rather than
reimplemented because the frontend has several traps — German menus, ~20-frame
key holds, a fifth menu item that quits, a heap-resident highlight variable,
and a level map that only responds to a click at a specific render pixel on the
stock 640x480 build. See
[the harness record](evidence/hook_harness.md#reaching-gameplay-at-all).

This is the one place the two repositories touch, and only as a test driver —
never as evidence of what the original does.

## The startup sweep

Reaching gameplay requires input the harness cannot supply, and many routines
are never called from the frontend. So each hook also runs a differential sweep
at attach time, comparing the recovered routine against the original bytes
reached through its own trampoline.

This is a stronger form of the offline oracles in `tools/`: the original code
is already mapped at its recorded address in a real process, so no
address-mapping helper and no ASLR retry loop is needed. Launching the game as
far as the menu is enough to verify a routine.

## DllMain runs before the game's CRT

`patch.dll` is a **static** import of the executable, so `DllMain` runs during
loader initialisation — before the executable's CRT startup. The game's heap
does not exist yet.

Installing detours there is fine: it is only memory patching. Binding
`lemmings::set_active_allocator` is fine: it only stores pointers. **Calling**
the game's `operator new` is not — it enters an uninitialised MSVCRT heap and
the process dies before showing a window, with

```
The application was unable to start correctly (0xc0000142)
```

which is `STATUS_DLL_INIT_FAILED`. The symptom looks like a broken DLL rather
than a logic error, so it is worth recognising.

Anything that allocates therefore has to be deferred. The harness runs those
sweeps from the first `DirectDrawCreate`, which the game calls from `WinMain`
once the CRT is up, and `hooks_native_int_array.cpp` keeps a `g_heap_ready`
flag so its hook forwards straight to the original until that point. The two
non-allocating sweeps still run from `DllMain`.

## The DLL must not depend on the MinGW runtime

The game has no MinGW runtime on its search path, so any dependency beyond the
system libraries means the executable cannot start — the same `0xc0000142`.

This is easy to trip over: adding `try`/`catch` pulls in libstdc++'s exception
machinery, which references `pthread_mutex_*` and `pthread_once` and so links
`libwinpthread-1.dll`. `-static-libstdc++` does not cover it, and neither does
`-Wl,-Bstatic -lwinpthread` (the driver has already seen the import library by
then). `-static` conflicts with `-shared`. What works is forcing the archive in
whole:

```
-Wl,--whole-archive /path/to/libwinpthread.a -Wl,--no-whole-archive
```

`hooks/build_hooks.sh` asserts the final dependency list after linking and
fails the build on anything unexpected, so this cannot regress silently.

## Four hazards

These are the mechanical failure modes specific to running MinGW-built code
inside an MSVC 6 executable.

1. **`__thiscall`.** Original member functions take `this` in ECX, push
   arguments right to left, and pop them in the callee. `0x0040DE60` ends in
   `RET 8`, so it is thiscall with two stack arguments. The harness uses GCC's
   `__attribute__((thiscall))`; the generated epilogue must be `ret $0x8`, and
   a call to the trampoline must be followed by `sub $0x8,%esp` to compensate
   for the callee's pop. Both were confirmed by disassembling the built DLL.
2. **Two heaps.** Memory the game frees must have been allocated by *its*
   `operator new` (`0x0049C1A8`, freed through `0x0049C13D`), never by MinGW's.
   Resolved by `lemmings::active_allocator()` — see the allocator boundary
   section above. Verified in-process: 2,340 comparisons of `0x0041BC10` with
   both sides allocating and freeing through the game's CRT.
3. **x87.** The original is pure x87 with no SSE. The harness builds with
   `-mfpmath=387` and must not disturb the FPU control word the game sets. The
   outstanding "game FPU mode pending" caveat on the floating-point routines
   stops being a footnote once they run in-process.
4. **Struct layout.** Hooked code reads the game's own objects, so every field
   offset must be exact. The `static_assert(offsetof(...) == ...)` lines in
   `include/lemmings` are what keep this from becoming a crash; they are
   checked at compile time for the 32-bit build.

## The allocator boundary

Recovered code must not allocate on MinGW's heap, because the original releases
those blocks through its own `free` at `0x0049C13D`. A block allocated on one
heap and released on the other is corruption, not a wrong answer — and the
offline oracles cannot show it, because they substitute a host allocator
adapter on *both* sides.

`lemmings::active_allocator()` (in `include/lemmings/allocator.hpp`) is the
indirection that fixes this. It defaults to the host `::operator new` /
`::operator delete` pair, so unit tests and the oracles are unaffected;
`install_native_int_array_hook` replaces it with the game's own
`operator new` (`0x0049C1A8`) and `free` (`0x0049C13D`), both `__cdecl`, before
any recovered allocating code can run. That is why it installs first.

`src/native_int_array.cpp` is converted. Any further recovered routine that
allocates must take its memory from `active_allocator()` rather than the global
operators.

**Exceptions still may not cross the boundary.** `append_native_int` throws
`std::length_error` on a corrupt array header, where the original would write
out of bounds. A C++ exception cannot propagate into MSVC 6 frames that know
nothing about it. The array hook therefore validates the header before calling
and catches everything at the boundary as defence in depth. Note that the catch
is what pulls libstdc++'s exception machinery into the DLL, with the link
consequences described above — a hooked routine that avoids throwing entirely
is cheaper.

## Build and install

The harness is built by a script rather than CMake, because it needs the 32-bit
MinGW toolchain specifically and produces a DLL destined for a game directory
rather than a test binary.

```bash
bash hooks/build_hooks.sh
```

It links `build/x86/liblemmings_recovered.a`, so configure and build the x86
tree first. Override `LEMMINGS_MINGW32_GXX` or `LEMMINGS_RECOVERED_LIB` if your
paths differ.

```powershell
python hooks/install.py --status  "C:\progs\Lemmings Revolution"
python hooks/install.py --install "C:\progs\Lemmings Revolution"
python hooks/install.py --remove  "C:\progs\Lemmings Revolution"
```

The install backs up `patch.dll` to `patch_orig.dll`, records its SHA-256, and
refuses to run if it would overwrite the original with the proxy. `--remove`
restores the backup after checking that digest. No game data is copied into the
repository.

Then run the game and read `lemmings_hooks.log` in the installation directory.
Call tallies are logged at powers of two, so terminating the game from outside
— which skips `DLL_PROCESS_DETACH` — still leaves evidence behind.

## Adding a hook

1. Record the entry address, the original prologue bytes, and an instruction
   boundary at or past 5.
2. Determine the calling convention from the epilogue: `RET n` means callee-pop,
   and `mov [ebp-4],ecx` in the prologue means `this` arrives in ECX.
3. Add a `hooks_<subsystem>.cpp` with the typed trampoline, the hook entry, a
   startup sweep over the routine's interesting input space, and an installer.
4. Declare the installer in `hooks/hooks.hpp`, call it from `install_hooks` in
   `hooks/patch_proxy.cpp`, and add the source to `hooks/build_hooks.sh`.
5. Run in `shadow` until the sweep and real gameplay report zero mismatches,
   then promote to `verify`, then `replace`.
