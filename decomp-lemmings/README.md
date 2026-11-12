# Lemmings Revolution decompilation

This project recovers a buildable C++ codebase from an installed x86 Windows
**Lemmings Revolution** executable. It is intended to supply source-level
evidence to [Lemmings Webolution](../lemmings-webolution). The long-term target
is the original game's behavior and structure as closely as recovery allows.

The target is a **working executable built from this source**, not a
byte-identical rebuild of the original; matching the original compiler's output
is explicitly not a goal. Recovered code therefore runs inside the original
game through the harness in [`hooks/`](hooks), replacing routines one at a time,
so there is a running game from the first function onward. See
[the hybrid runtime notes](docs/HYBRID_RUNTIME.md) for the phase plan and
[the original source layout](docs/ORIGINAL_LAYOUT.md) for the upstream module
names recovered from the executable's assert strings.

This is an incremental recovery, **not yet a standalone game**. The current library recovers
the general gadget point-containment routine at `0x00430BA1`, two actor
searches, local gadget box checks, matching actor ID collection, the
collision grid accessor, cylinder coordinate wrapping, gadget bounds
setters, a timed actor trigger, the terrain blast's grid-clearing loop, and
a length-prefixed level string reader, five compiled-level resource-name
tables, and the first object's nested type-four polygon records.
The next world section's geometry blocks, class-one record, integer arrays,
fixed byte block, and 32 tail lists are also decoded.
The following pair of objects is decoded through both complete child trees,
including class-one and class-two nodes, type-five and type-seven polygon
lists, and auxiliary records. Nested auxiliary values are retained as
serialized bytes.
The remaining model trees, entity lists, tagged subclasses and final level
fields are decoded too. The complete serialized layout now matches all 105
installed compiled levels through exact EOF. Construction of a complete level
runtime remains pending.
Runtime slices now include model advancement and blinking, timed emitters,
actor behavior scheduling and attachment, direction changes, and terrain
contact checks. World coordinate interpolation also uses the recovered level
mesh directly. Falling behavior, landing decisions and frame-based animation
playback are recovered with their remaining dependencies exposed as callbacks.
Walking updates, terrain decisions, surface contact, slope following, actor
animation selection, and extra-state cleanup are also recovered. Climbing,
bouncing, climb topping and terminal type-0x1A behavior now include entry and
update slices with their effect and model dependencies explicit. Four-cell wall
contact and behavior removal by type are recovered too. Runtime recovery also
includes the spawned actor monitor, vertical terrain probes, and actor
finalization with concrete world sampling and model positioning. Actor/base
construction and destruction, inline model initialization/teardown, pointer
registries, drowning, region terminal, forced, flapper and explosion states,
the original random generator, shadow construction/geometry/terrain segments,
complete world update orchestration, full terrain-blast effect order, actor
resource selection, and cache acquisition/release are recovered with explicit
subsystem boundaries. Resource slot registration, cache lookup/loading,
eviction/unload/recreation, manager construction, and sprite factory lifecycle
are now recovered too. The sprite
reader decodes all 19 installed SPR resources through EOF, including 132
clips, 1,526 frames, and 7,354 pieces.
Native comparisons check each recovered slice against the
original instructions, with callback boundaries documented explicitly.
The library also reads the installed `.box` archive index and locates entries
with the original patch-first search order, so source tools can open level
fixtures without copying complete assets into the repository.
Each recovered routine gets a
source implementation, a byte-linked evidence note, and tests for behavior
that the code claims. Unknown fields and behaviors stay marked
unknown rather than being filled from the remake.

The recorded function coverage baseline is **372 of 6,186 functions (6.0%)**,
including partial recoveries. See [the progress benchmark](docs/PROGRESS.md)
for counting rules and future checkpoints.

## Build and check

Requires CMake 3.28+ and a C++23 compiler. From PowerShell:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
python tools/verify_reference.py "C:\progs\Lemmings Revolution\Lemmings Revolution.exe"
```

Pass `-DLEMMINGS_ARCHIVE="C:/progs/Lemmings Revolution/lemmings.box"`
and optionally `-DLEMMINGS_PATCH_ARCHIVE="C:/progs/Lemmings Revolution/patch.box"`
at configuration time to check complete compiled-level reads against the
installed `.LVL` entries. These checks read the archives in place.

On a Windows x86 build, supply the same executable path at configure time
to run 67 offline original-byte differential tests with CTest:

```powershell
$env:PATH = "C:\progs\msys2\mingw32\bin;" + $env:PATH
cmake -S . -B build/x86 -G Ninja -DCMAKE_CXX_COMPILER=C:/progs/msys2/mingw32/bin/g++.exe -DLEMMINGS_REFERENCE_EXE="C:/progs/Lemmings Revolution/Lemmings Revolution.exe"
cmake --build build/x86
ctest --test-dir build/x86 --output-on-failure
```

The executable is supplied from a user-owned installation; no complete
original game binary or asset files are stored here. The byte verifier
identifies the exact reference build. See
[the function ledger](docs/FUNCTIONS.md) for recovery status and evidence.

The resource integration harness loads installed sprites through the recovered
registration, cache, factory, animation binding and cleanup routines:

```powershell
build/lemmings_resources.exe --archive "C:/progs/Lemmings Revolution/lemmings.box" --patch "C:/progs/Lemmings Revolution/patch.box" --all
```

Use `--sprite LemmingsLo` instead of `--all` for one resource. The installed
fixture checks all 19 sprites, cache hits, retained-node recreation, eviction,
reference counts and slot reuse. It builds and checks runtime arrays and 167
texture registrations, generates rectangle geometry for all 7,354 pieces, then
frees their nested storage through recovered code. File opening and heap storage
use portable adapters; graphics texture creation and rendering remain pending.
See [the integration evidence](docs/evidence/resource_lifecycle_integration.md).

The texture inspector decodes packed bitmap payloads without extracting assets:

```powershell
build/lemmings_textures.exe --archive "C:/progs/Lemmings Revolution/lemmings.box" --patch "C:/progs/Lemmings Revolution/patch.box" --all
```

The installed fixture checks 10,383 distinct BMP entries after patch precedence.
The packed envelope and Huffman/backreference decoder also have original-byte
comparisons. See [texture payload evidence](docs/evidence/texture_payload_integration.md).

The [plan to a standalone build](docs/PLAN.md) sets out the remaining tracks,
the measurement needed before scheduling them, and the risks.

The [recovery map](docs/RECOVERY_MAP.md) identifies the executable entry,
main loop, level loading, and world-update paths for the next source slices.
Use `python tools/inspect_box.py "C:\progs\Lemmings Revolution\lemmings.box" --filter .LVL`
to list compiled levels in the installed archive without copying game assets.
Add `--name "COMPILED\MM_CLIMB_002.LVL" --level-header` to inspect one
level's still unnamed header fields.

The C++ inspector reads a complete level and emits a JSON summary:

```powershell
build/lemmings_inspect.exe --archive "C:/progs/Lemmings Revolution/lemmings.box" --patch "C:/progs/Lemmings Revolution/patch.box" --level "COMPILED\MM_CLIMB_002.LVL"
```

Use the archive's backslash entry names (for example
`COMPILED\MM_CLIMB_002.LVL`) when selecting a level. Replace `--level` and its
value with `--list-levels` to list the 102 distinct effective compiled levels.
The public `read_compiled_level` API returns all recovered serialized
sections and checks EOF by default; `load_compiled_level` applies archive
precedence. These composition APIs and the inspector are new recovery tools.

The `lemmings_motion` diagnostic composes recovered falling and walking slices
with installed level terrain and sprite animation, emitting CSV coordinates
and animation frames. For example:

```powershell
build/lemmings_motion.exe --archive "C:/progs/Lemmings Revolution/lemmings.box" --patch "C:/progs/Lemmings Revolution/patch.box" --level 'COMPILED\MM_CLIMB_002.LVL' --sprite 'SPRITES\LEMMINGSHI.SPR' --ticks 600
```

It starts one synthetic actor at the selected emitter (`--emitter`, default 0),
with explicit initial sample offsets and depth. It stops at unsupported gadget
or behavior boundaries. This tool is a new composition for inspecting recovered
motion; it does not reproduce original actor construction or a full world loop.
See the [motion diagnostic notes](docs/evidence/motion_trace.md) for its CSV
time accounting, stop reasons, verification and remaining boundaries.

## Hybrid runtime harness

The harness replaces the installation's two-export `patch.dll` with a proxy
that forwards `DirectDrawCreate` and `DirectDrawEnumerateA` to the original
library and installs entry-point detours for recovered routines. The game runs
normally with it in place.

```bash
bash hooks/build_hooks.sh
```

```powershell
python hooks/install.py --install "C:\progs\Lemmings Revolution"
# play, then read C:\progs\Lemmings Revolution\lemmings_hooks.log
python hooks/install.py --remove  "C:\progs\Lemmings Revolution"
```

The build links `build/x86/liblemmings_recovered.a`, so configure and build the
x86 tree first. The install backs up `patch.dll` to `patch_orig.dll`, records
its SHA-256, and refuses any swap that would lose the original. No game data is
copied into this repository.

`LEMMINGS_HOOK_MODE` selects `shadow` (default: compare but return the
original's result, so the hook cannot affect play), `verify` (compare and
return the recovered result), or `replace` (recovered code only). Each hook
also sweeps itself against the original bytes at startup, so launching as far
as the menu verifies a routine even when gameplay never calls it.

Currently hooked, all matched against the original bytes in-process with no
mismatches: the collision grid accessor `0x0040DE60` (19,728 comparisons),
cylinder X wrapping `0x0040DEC0` (4,219 comparisons, under the game's own FPU
control word), and array append-with-growth `0x0041BC10` (2,340 comparisons
with both sides allocating through the game's CRT). See
[the harness record](docs/evidence/hook_harness.md).

The first two were additionally exercised by real gameplay — 524,288+ calls
each inside `MM_DIG_000`, no divergence. `0x0041BC10` is never called during a
level, so its only evidence is the startup sweep.

Recovered code that allocates must take memory from
`lemmings::active_allocator()` rather than the global operators, so that it can
be bound to the game's own CRT when running in-process. Sweeps that allocate
cannot run from `DllMain`, which executes before the executable's CRT starts.

Gameplay evidence needs the `dinput.dll` shim from the remake project, because
the game reads all input through DirectInput and ignores Windows input
injection:

```powershell
python tools/drive_hook_session.py --level MM_DIG_000 --seconds 60
```

## Recovery method

1. Verify the reference executable digest before making a claim about its code.
2. Record the virtual address, the entry prologue, the calling convention and
   argument pop, and the field offsets the routine touches. Use REA and
   decompiler output as a draft; read the original bytes where a decompiler
   cannot be trusted — x87 floating point, unions and type punning, calling
   conventions, struct offsets, and visibly garbled output.
3. Implement the observed behavior in readable C++, retaining original field
   offsets and comparison order where known. Keep the `offsetof` assertions.
4. Hook it in `shadow` mode and run the game. Promote to `verify` and then
   `replace` once the startup sweep and real gameplay report no mismatches.
   Offline unit tests still cover edge cases the game may not reach.

Matching the original compiler's output is not a goal. See
[AGENTS.md](AGENTS.md) for the rules this method keeps and the ones it drops.

The [remake project](../lemmings-webolution) remains separate; this repository
is the source recovery workspace, not its renderer or gameplay implementation.
