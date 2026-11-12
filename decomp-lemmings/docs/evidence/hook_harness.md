# In-process hook harness: Phase 0 and Phase 1 record

## Identity and evidence

Stock x86 `Lemmings Revolution.exe` SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x00400000`, linker version 6.0 (Visual C++ 6.0).

Installation `patch.dll` before replacement: 10,766 bytes, SHA-256
`c9a09caaa09e89df9bcbb7cb639688916ebf58d2b96ed676d9f58ceab64405ad`,
preserved as `patch_orig.dll` by `hooks/install.py`.

## Injection point

`patch.dll` has an export directory of 108 bytes holding exactly two named
exports, both imported by name from the executable's import table:

| Ordinal | Export RVA | Name |
| --- | --- | --- |
| 1 | `0x00001bae` | `DirectDrawCreate` |
| 2 | `0x00001ae9` | `DirectDrawEnumerateA` |

`ddraw.dll` does **not** appear in the executable's import table, so all
DirectDraw entry goes through `patch.dll`. The replacement therefore sees every
DirectDraw creation call the game makes.

The executable's full static import set is `advapi32`, `dinput`, `dsound`,
`gdi32`, `kernel32`, `ole32`, `patch`, `user32`, `winmm`.

## Detour target

`0x0040DE60`, the collision grid cell accessor, documented in
[the grid note](0040de60.md). Chosen for Phase 1 because it is a leaf routine,
allocates nothing, does not dereference the pointer it returns, and already had
20,248 offline original-byte comparisons behind it.

Original prologue, file offset 56928:

```
55          push ebp
8b ec       mov  ebp, esp
51          push ecx
89 4d fc    mov  [ebp-4], ecx
```

Instruction boundaries sit at 4 and 7 bytes, so a five-byte `JMP rel32`
requires **7 bytes** relocated into the trampoline. The harness compares these
seven bytes against the running image before patching.

Calling convention read from the surrounding code: `mov [ebp-4],ecx` places
`this` from ECX, `3b 91 18 04 00 00` (`cmp edx,[ecx+0x418]`) confirms ECX is
the world base, arguments sit at `[ebp+8]` and `[ebp+0C]`, and the routine ends
in `RET 8`. That is `__thiscall` with two stack arguments.

## ABI verification

Disassembly of the built `patch.dll` confirms the boundary is generated
correctly:

- `mov %ecx,%esi` at hook entry — `this` consumed from ECX.
- `ret $0x8` on both return paths — callee pops its two arguments.
- `sub $0x8,%esp` immediately after the trampoline call — compensates for the
  original's own pop.

Runtime dependencies of the built DLL are `KERNEL32.dll` and `msvcrt.dll` only;
`-static-libgcc -static-libstdc++` keeps the MinGW runtime out of the game
process. Exports are undecorated `DirectDrawCreate` and `DirectDrawEnumerateA`,
with internal name `patch.dll`.

## Second detour target: x87

`0x0040DEC0`, cylinder X wrapping, documented in
[the coordinate note](0040dec0.md). Hooked second and deliberately: it is
entirely floating point, returns its result on the x87 register stack, and is
the only way to test the FPU hazard in the real process.

Its prologue is byte-identical in shape to `0x0040DE60`
(`55 8b ec 51 89 4d fc`), so again 7 bytes relocate; the earliest internal
branch target is `0x0040DEEC`, well clear of the relocated range. `this`
arrives in ECX, is stored at `[ebp-4]`, and is never read. The `double`
argument sits at `[ebp+8]` and the routine ends in `RET 8`.

The interesting detail is at `0x0040DF20`: the subtraction loop ends each pass
with `fstpl 0x8(%ebp)`, writing the running value back into its own argument
slot. That forces a round to `double` every iteration. A port that kept the
value in an x87 register would accumulate at 64-bit mantissa precision instead
and could diverge for inputs needing many passes. The port in
`src/cylinder_coordinate.cpp` already carried a comment noting this; the
in-process sweep is what actually tests it, because only there does the code
run under the control word the game itself installed.

Generated code verified by disassembling the built DLL: `mov %edi,%ecx`
restores `this` into ECX before the trampoline call, `fstpt` saves the 80-bit
result across the comparison call, and both return paths end in `ret $0x8`.

Positive infinity is excluded from the sweep. The original's loop at
`0x0040DF07` through `0x0040DF23` subtracts 256 until the value drops below
256, which never happens for infinity, so feeding it would hang the game. The
port documents the same exclusion.

## Third detour target: the heap boundary

`0x0041BC10`, append-with-growth, documented in
[the collection note](00430f4d.md). Hooked third to exercise the allocator
hazard, which the first two could not: it takes memory from the executable's
own CRT.

Prologue `55 8b ec 83 ec 14` — `push ebp` / `mov ebp,esp` / `sub $0x14,esp`.
Boundaries at 1, 3, 6, 7 and 10, so **6 bytes** relocate; the earliest internal
branch target is `0x0041BC5D`. `this` is the array, `RET 4` confirms one
callee-popped stack argument, and the function spans `0x0041BC10`–`0x0041BCC9`.

**The value argument is a pointer, not an immediate.** `0x0041BCB0` reads
`mov 0x8(%ebp),%ecx` then dereferences with `mov (%ecx),%ecx`. The recovered
port takes the value by itself, which is correct for its own callers but means
the hook signature is `(NativeIntArray*, const std::int32_t*)`.

The CRT pair it calls was confirmed by disassembly, both `__cdecl`:
`0x0049C1A8` pushes a size, calls `0x0049E88A`, pops with `pop ecx` twice and
returns plain; `0x0049C13D` pushes a block, calls `0x0049E340`, pops once and
returns plain.

Shadow comparison works differently for a mutating routine. The original cannot
be run twice — it reallocates and frees, so the input state is gone after the
first call. Instead the recovered routine runs against a deep copy taken from
the **game's** heap, the original runs against the real array, and the results
are compared by count, capacity, growth and element contents. Data pointers are
excluded, because each side allocates its own block.

## Two failure modes found while building this

Both produce `0xC0000142` (`STATUS_DLL_INIT_FAILED`) and a
"The application was unable to start correctly" dialog, with no log written,
which reads like a corrupt DLL rather than a logic error.

**Allocating from `DllMain`.** `patch.dll` is a static import, so `DllMain`
runs during loader initialisation, before the executable's CRT startup. The
game's heap does not exist yet. Installing detours and binding the allocator
are both safe there; calling the game's `operator new` is not. The allocating
sweep is now deferred to the first `DirectDrawCreate`, which the game issues
from `WinMain`, and the hook keeps a `g_heap_ready` flag so it forwards
straight to the original until then.

**A MinGW runtime dependency.** Adding `try`/`catch` pulled libstdc++'s
exception machinery in, which references `pthread_mutex_*` and `pthread_once`
and so linked `libwinpthread-1.dll` — not on the game's search path.
`-static-libstdc++` does not cover it, `-Wl,-Bstatic -lwinpthread` is too late
in the driver's link order, and `-static` conflicts with `-shared`. Forcing the
archive in whole works:
`-Wl,--whole-archive libwinpthread.a -Wl,--no-whole-archive`.
`hooks/build_hooks.sh` now asserts the final dependency list and fails the
build on anything outside the system libraries.

## Observed run

Four launches of the installed game, terminated externally after the startup
sweep: `shadow`, `verify` and `replace` with the grid hook alone, then `shadow`
with both hooks. The game started and kept running in every case, including
`replace`, where the recovered code answers with no fallback to the original.

Transcript of the three-hook `shadow` run:

```
--- lemmings hook harness attached 2026-10-29 11:49:51 ---
module directory: C:\progs\Lemmings Revolution
requested hook mode: shadow
recovered allocator bound to the game CRT (new 0x0049c1a8, free 0x0049c13d), replacing the host pair at 716338a0
detour installed at 0x0041bc10 -> 0x716331f0 (trampoline 0x02710000, 6 bytes relocated)
append_native_int hook active in shadow mode
detour installed at 0x0040de60 -> 0x71631ed0 (trampoline 0x02720000, 7 bytes relocated)
collision_cell hook active in shadow mode
detour installed at 0x0040dec0 -> 0x71632480 (trampoline 0x02730000, 7 bytes relocated)
wrap_cylinder_x hook active in shadow mode
3 detour(s) installed
selftest: 19728 in-process comparisons against the original bytes, 0 mismatches
cylinder selftest: 4219 in-process comparisons against the original bytes, 0 mismatches
startup selftest: PASS
forwarding to C:\progs\Lemmings Revolution\patch_orig.dll (DirectDrawCreate=6cbc1bae, DirectDrawEnumerateA=6cbc1ae9)
CRT is up; running deferred selftests that use the game heap
array selftest: 2340 in-process comparisons against the original bytes through the game heap, 0 mismatches, 0 guarded
deferred selftest: PASS
```

The ordering is load-bearing: the two non-allocating sweeps complete before
`DirectDrawCreate`, and the allocating one only after it.

Hook and trampoline addresses vary between runs because the DLL base and
`VirtualAlloc` are both subject to address randomisation; the detour targets do
not, which is why the image-base check matters.

The forwarding line appears last because the original library is loaded lazily
on the first DirectDraw call rather than from `DllMain`, so its presence
confirms the game reached DirectDraw creation through the proxy and started
normally.

## What this establishes

- The proxy loads, forwards DirectDraw, and the game runs with it installed.
- The image-base check, prologue check and trampoline construction work against
  the real process.
- The `__thiscall` boundary and the `CollisionGridHead` field offsets are
  correct in-process, not only in the offline x86 unit build.
- `collision_cell` matched the original bytes on **19,728 in-process
  comparisons** across heights -2 through 1024, boundary and wrapped X values,
  clamped and out-of-range Y, and a deterministic xorshift sweep.
- `wrap_cylinder_x` matched on **4,219 in-process comparisons** under the FPU
  control word the game installs, covering the threshold constant, values
  straddling every multiple of 256 up to 16384, denormals, NaN, negative
  infinity, and a deterministic sweep bounded at magnitude 100,000. The x87
  return-in-ST0 boundary and the loop's per-pass rounding therefore both
  behave as the port assumes, **for the tested range** — this is empirical
  agreement, not a proof that the port rounds identically everywhere.
- `append_native_int` matched on **2,340 in-process comparisons** with both
  sides allocating and freeing through the game's own CRT, across growth steps
  1 through 32, capacities 0 through 24 at every valid count, and exactly-full
  arrays up to 64 elements. No guarded (thrown) cases were reached in the
  sweep, so the port's corrupt-header check never fired on valid input.
- The detour mechanism generalises: three routines of different shapes —
  integer leaf, x87 leaf, and a mutating allocator — needed no change to
  `hook_engine.cpp`.
- The allocator indirection works: `lemmings::active_allocator()` bound to the
  game's CRT is what makes a recovered allocating routine safe in-process.

## Gameplay run

`tools/drive_hook_session.py` launches the installed game, confirms the input
shim is being polled, enters a named level, holds for a fixed period and reads
the harness's counters back.

Level entry was confirmed by the engine's own answer rather than ours: the path
buffer at `0x004CBBF8` read `Compiled\MM_DIG_000.lvl`, with a 300 second time
limit and a save target of 10.

45 seconds inside that level, mode `shadow`:

| Routine | Gameplay calls | Mismatches |
| --- | ---: | ---: |
| `0x0040DE60` `collision_cell` | 524,288+ | 0 |
| `0x0040DEC0` `wrap_cylinder_x` | 524,288+ | 0 |
| `0x0041BC10` `append_native_int` | 0 | — |

Counts are the last power-of-two milestone the log recorded, so the true totals
lie between 524,288 and 1,048,576 each; the process was terminated rather than
quit, which skips the exact final tally. Either way this is upwards of a
million comparisons against the original bytes on real gameplay data, under the
game's own FPU control word, with no divergence.

`append_native_int` never ran. That is a fact about the level, not a gap in the
hook: the array it grows backs the gadget actor-ID collection at `0x00430F4D`,
which needs gadget interactions this run did not produce. Its evidence remains
the 2,340-case startup sweep.

## Reaching gameplay at all

The game reads **all** input through `IDirectInputDevice::GetDeviceState` and
ignores Windows input injection entirely, so `SendInput` and `SendKeys` never
reach it. That is why the harness's gameplay counters sat at zero initially.

The sibling remake project had already solved this with a `dinput.dll` proxy
that serves device state from a named shared-memory block, together with a
frontend driver that encodes the awkward parts of getting from the title screen
into a level. Those parts are not incidental:

- The retail menus are German. RETURN activates "Neues Spiel starten", RETURN
  again confirms "Ja".
- A key must be held for roughly 20 polled frames; short taps are dropped.
- The fifth main-menu item is "Beenden", so a blind RETURN quits the game. The
  driver locates the highlight variable — it lives on the heap and moves every
  run — and drives it to zero before pressing anything.
- `0x004C416C` distinguishes the frontend screens (3 menu, 4 confirm, 5 map);
  `ui_state` is 3 for all three.
- On the map, arrow keys do nothing and RETURN means "back", so the node must
  be clicked, at render pixel (206, 204).
- Only the stock 640x480 executable works. Under the resolution-patched build
  the map's nodes are 3D-picked at the wrong scale and respond to no click.

`tools/drive_hook_session.py` therefore delegates navigation to that driver
across a process boundary instead of re-deriving it, and the two repositories
stay otherwise separate. The driver swaps a level record into the one slot
whose node position is known and restores it in a `finally`, so the game's own
table is not left altered.

## Limits

- The sweep sets `cells.data` to a fixed non-null value and compares computed
  pointers. That is sound because neither the original nor the port
  dereferences the result, but it is not a test of cell contents.
- Gameplay calls are now observed; see the section below. `append_native_int`
  was never called during the level that was played, so its only evidence
  remains the startup sweep.
- `report_collision_grid_hook` runs from `DLL_PROCESS_DETACH`, which
  `TerminateProcess` skips. The final tally line is only written when the game
  exits normally.
- This is a hooked hybrid, not a standalone build. The original executable and
  `lemmings.box` remain runtime requirements.
