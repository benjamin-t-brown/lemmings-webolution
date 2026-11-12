# Reconstruction gap audit (task `evidence-001`)

Date: 2026-10-29/30 UTC. Author: Claude worker for queue task `evidence-001`.

Reference identity used throughout, unchanged from
[`agent/tools.windows.json`](tools.windows.json) and
[`decomp-lemmings/docs/FUNCTIONS.md`](../decomp-lemmings/docs/FUNCTIONS.md):
installed `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, x86.

**Method of this audit.** Read-only inspection of repository files with the
Read/Glob/Grep tools only. No original game, hook, debugger, capture, REA
session, compiler, CMake or oracle was run, and no original machine code was
executed. Nothing below is a new measurement of the original program. Where a
number appears it is either (a) a count I obtained by reading repository text,
or (b) a claim retained verbatim from an existing evidence note, labelled as
such.

Inspected: [`docs/STATUS.md`](../docs/STATUS.md),
[`agent/IMPLEMENTATION_PLAN.md`](IMPLEMENTATION_PLAN.md),
[`agent/tasks.json`](tasks.json), [`agent/README.md`](README.md),
[`tools/agent_control/verify.py`](../tools/agent_control/verify.py),
[`decomp-lemmings/CMakeLists.txt`](../decomp-lemmings/CMakeLists.txt),
[`decomp-lemmings/docs/FUNCTIONS.md`](../decomp-lemmings/docs/FUNCTIONS.md),
[`decomp-lemmings/docs/PROGRESS.md`](../decomp-lemmings/docs/PROGRESS.md),
[`decomp-lemmings/docs/RECOVERY_MAP.md`](../decomp-lemmings/docs/RECOVERY_MAP.md),
[`decomp-lemmings/docs/benchmarks/coverage.json`](../decomp-lemmings/docs/benchmarks/coverage.json),
the `decomp-lemmings/docs/evidence/` notes for the renderer, texture and file
slices, the `src/`, `include/`, `tests/` and `tools/` file inventories, and
`src/sprite_render_shapes.cpp`.

---

## A. Static facts established by reading this repository

These are checkable by re-reading the cited files. They are facts about the
repository, not about the original executable's behavior.

**A1. Seven recovered source modules are not in the registered recovery
build.** `decomp-lemmings/CMakeLists.txt` lines 11-86 list 74 sources in
`lemmings_recovered`. `decomp-lemmings/src/` contains 81 `.cpp` files. The
seven absent from the library are:

| Module | Header | Unit test | x86 oracle + runner | Evidence note |
| --- | --- | --- | --- | --- |
| `src/sprite_render_shapes.cpp` | `include/lemmings/sprite_render_shapes.hpp` | `tests/test_sprite_render_shapes.cpp` (+ `tests/sprite_render_shapes_fixture.hpp`) | `tools/native_sprite_render_shapes_oracle.cpp`, `tools/run_native_sprite_render_shapes_oracle.py` | `docs/evidence/sprite_render_shapes.md`, `sprite_render_shapes_rea.json` |
| `src/sprite_render_submission.cpp` | `.../sprite_render_submission.hpp` | `tests/test_sprite_render_submission.cpp` (+ fixture) | `native_sprite_render_submission_oracle.cpp` + runner | `sprite_render_submission.md`, `.asm`, `.metadata.json` |
| `src/sprite_render_clip.cpp` | `.../sprite_render_clip.hpp` | `tests/test_sprite_render_clip.cpp` | `native_sprite_render_clip_oracle.cpp` + runner | `sprite_render_clip.md`, `.asm`, `.metadata.json` |
| `src/sprite_render_draw.cpp` | `.../sprite_render_draw.hpp` | `tests/test_sprite_render_draw.cpp` | `native_sprite_render_draw_oracle.cpp` + runner | `sprite_render_draw.md` |
| (composition only) | — | `tests/test_sprite_render_pipeline.cpp` (+ fixture) | `native_sprite_render_pipeline_oracle.cpp` + runner | `sprite_render_pipeline.md` |
| `src/texture_creation.cpp` | `.../texture_creation.hpp` | `tests/test_texture_creation.cpp` | `native_texture_creation_oracle.cpp` + runner | `texture_creation.md` |
| `src/texture_pixel_format.cpp` | `.../texture_pixel_format.hpp` | `tests/test_texture_pixel_format.cpp` | `native_texture_pixel_format_oracle.cpp` + runner | `texture_pixel_format.md` |
| `src/file_stream_open.cpp` | `.../file_stream_open.hpp` | `tests/test_file_stream_open.cpp` | `native_file_stream_open_oracle.cpp` + runner | `file_stream_open.md` |

**A2. Eight unit tests exist but are unregistered.** `decomp-lemmings/tests/`
holds 75 `test_*.cpp` files. `CMakeLists.txt` registers 67 of them (45 in the
line-107 `foreach`, 22 added individually). The eight unregistered ones are
exactly the tests in A1. With `resource_lifecycle_boundaries` and the eight
`installed_*` data tests, the registered total is 76 — matching the
`76/76` recovery result retained in the `repo-001` verification log. So the
registered verifier currently neither compiles nor runs any of the modules in
A1; they appear to have been built only by hand (`decomp-lemmings/build/`
contains `test_sprite_render_shapes_manual.exe` and
`x86/native_sprite_render_shapes_O0.exe`/`_O2.exe`).

**A3. The ledger and the source tree disagree.** `docs/FUNCTIONS.md` ends at
`0x0047A190` (`sprite_render_geometry`, row 127). Grep finds no row for
`sprite_render_shapes`, `sprite_render_submission`, `sprite_render_clip`,
`sprite_render_draw`, `texture_creation`, `texture_pixel_format` or
`file_stream_open`, and no mention of `0x0046D460`, `0x00473B1E`, `0x004743DD`,
`0x0047218A` or `0x00472490`. `0x0047E40D` appears only as the two interior
finalization blocks already credited to `file_stream_runtime.cpp` (row 124).
Separately, `src/resource_table_load.cpp` **is** registered and tested
(`CMakeLists.txt` lines 84, 107) but has no `docs/evidence/` note and no ledger
row; `src/compiled_level.cpp` is likewise registered with no ledger row.

**A4. `RECOVERY_MAP.md` item 2 is stale.** It still says "Continue rendering
from rectangle geometry `47A190` through submission `414920`; adjacent
trapezoid `47A565` and anchored rectangle `47A75D` remain inspected context."
All three, plus clipping `46D3C1` and packing `46D460`, now have source and
evidence notes (A1). `PROGRESS.md`'s `372 / 6,186` baseline predates them.

**A5. The relevant addresses are on the traced playable path.**
`docs/benchmarks/coverage.json` lists `0047a565`, `0047a75d`, `00470f29`,
`00414920`, `0046d3c1`, `0046d460`, `00473b1e`, `0047e40d`, `004743dd`,
`0047218a` and `00472490` among the 2,024 executed entries. That trace is
historical (one frontend run plus 50 s of `MM_DIG_000`, per `PROGRESS.md`) and
is not re-measured here.

**A6. Registering these targets cannot make the default verifier execute
original code.** `verify.py` lines 25-35 reject any test named `native_*`
(except the ordinary `test_native_int_array.exe`) and any test whose command
mentions the reference EXE; line 74 configures the recovery profile with an
empty `LEMMINGS_REFERENCE_EXE`, so `CMakeLists.txt`'s line-316 guard leaves
every `native_*` test unregistered; and the whole oracle block at line 245 is
gated on `WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4`, which the registered x64
`g++` does not satisfy. Adding oracle targets and `LEMMINGS_REFERENCE_EXE`-gated
tests therefore changes nothing in `--profile all`.

**A7. Legacy external paths persist in evidence notes.**
`docs/evidence/file_stream_open.md` lines 157-159 give reproduction commands
with `--exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe'`, outside
this repository. Root `AGENTS.md` requires repo-root-relative game paths. This
is a documentation/harness debt; it is not exercised by `--profile all`.

**A8. User parks are untouched by this audit.** `docs/STATUS.md` keeps D1
FAIL/parked, D3 FAIL/parked, D2 digger-clock parked and C3 FAIL. Nothing
proposed here touches simulation timing, digger behavior or whole-level replay.

---

## B. Retained claims from the existing evidence notes (not re-measured)

Quoted so the next task can cite them without restating them as new results.
Every one of these is an **offline, synthetic, x86 original-byte comparison**
with declared adapters. None is an in-process hybrid run inside the game, and
none claims visible rendering or gameplay.

| Module | Routines claimed | Retained verification claim |
| --- | --- | --- |
| `sprite_render_shapes` | `47A565` trapezoid, `47A75D` anchored rectangle, `470F29` filtering state | 60,000 original shape calls per x86 `-O0`/`-O2`; x64 `-O2` + x86 `-O0` units; only final submission and device COM adapted |
| `sprite_render_submission` | `414920` and 17 helpers (`414A10`, `414B90`, `414D20`, `414D00`, `47261F`, `414D80`, `414B70`, `414D60`, `414EF0`, `414F10`, `414F30`, `414A60`, `414A40`, `4150A0`, `4150B0`, `4150D0`, `4150F0`) | 129,024 submission comparisons plus 256 resize/cleanup pairs per x86 build; adapters: texture cache search/load, heap, CRT cleanup registration, final clipping, device COM |
| `sprite_render_clip` | `46D3C1` + 32 listed helpers | 12,000 four-plane comparisons and 40,000 interpolation comparisons per x86 build; only heap and final drawing adapted; 16 bytes at vertex +58..+67 explicitly indeterminate |
| `sprite_render_draw` | `46D460` + 12 helpers | 30,000 draw/cleanup comparisons per x86 build; allocation, free, CRT cleanup registration and the device COM method adapted |
| `sprite_render_pipeline` | composition of the three above | 20,000 uninterrupted original-chain cases per x86 build; 1,000-case x64/x86 composition units |
| `texture_creation` | `4743DD`, `471720`, `47218A`, `47244B`, `472490`, `450120`, `450138`, `445830`, `473DA0`, `47C7F0`, `47C5B0`, `414D00`, `406500` | 29,160 normal traces + 20 isolated fatal-open subprocesses per x86 build; GDI, CRT, file, loader `45015A`, reporting/exit and graphics init `472F99` adapted |
| `texture_pixel_format` | `473B1E` (636 bytes) | 2,457,600 selection cases at x86 `-O0`/`-O2`; only CRT `memcpy` adapted |
| `file_stream_open` | `47E40D` (1039 bytes), `47F0B0` | 27,648 complete original-function comparisons at x86 `-O0`/`-O2`; drive/registry prep, filesystem existence, archive lookup/reset, MessageBox and CRT open/close/tell/seek adapted |

Boundaries the notes state explicitly and that remain **unrecovered**: graphics
initialization `472F99`, the DirectDraw/Direct3D device and driver, texture
destruction, packed-file I/O and GDI, archive index loading, drive discovery,
MSVC exception/allocation-failure recovery, and the indeterminate interpolated
vertex tail.

---

## C. Inferences (clearly marked as inference)

1. The seven modules in A1 were most likely developed with manual compile
   scripts before the CMake list was extended, so the project's own registered
   check does not protect them. **Inference**, from the unregistered targets
   plus the `*_manual.exe` artifacts in `decomp-lemmings/build/`; I did not
   compile anything.
2. Because they are unbuilt by `--profile all`, these ~60-70 routines' source
   can silently stop compiling (for example after a shared-header change) with
   no signal. This is the highest-value, lowest-risk gap I found: the
   verification debt is in the build wiring, not in the evidence.
3. `sprite_render_{geometry,shapes,submission,clip,draw}` together form one
   subsystem boundary — the sprite submission path from piece geometry to the
   single device draw call — which matches `AGENTS.md`'s "small evidence-backed
   routine or subsystem boundary per task".
4. Nothing here bears on the main tree's open render gates. **C3 remains FAIL
   and C2 remains OPEN**; the recovery tree's sprite path is a faithful port of
   the original's own pipeline and is not wired into `src/` production
   rendering. Any future bridge between the two is separate, later work and
   would need its own evidence.

---

## D. The one bounded next implementation task

Proposed as the concrete replacement scope for the `native-001` placeholder.
I did not edit `agent/tasks.json`; the planner/controller owns the queue.

**Title.** Register and ledger the recovered sprite submission path.

**Scope.** Add to `decomp-lemmings/CMakeLists.txt`:

- library sources `src/sprite_render_shapes.cpp`, `src/sprite_render_submission.cpp`,
  `src/sprite_render_clip.cpp`, `src/sprite_render_draw.cpp`;
- unit tests `sprite_render_shapes`, `sprite_render_submission`,
  `sprite_render_clip`, `sprite_render_draw`, `sprite_render_pipeline`
  (add the five names to the line-107 `foreach`);
- the same five names to the two x86 oracle lists (lines 246 and 318) so the
  programs and `LEMMINGS_REFERENCE_EXE`-gated tests exist for a future,
  separately authorized x86 comparison. Per A6 this cannot affect
  `--profile all`.

Then add one `docs/FUNCTIONS.md` row per module, using the retained wording in
section B, and correct `RECOVERY_MAP.md` item 2 (A4).

**Addresses in scope.** Primary: `0x0047A565`, `0x0047A75D`, `0x00470F29`,
`0x00414920`, `0x0046D3C1`, `0x0046D460`. Supporting helpers exactly as listed
in the four evidence notes; do not invent additional addresses.

**Dependency closure.** Already registered and built:
`sprite_render_geometry.cpp`, `sprite_resource.cpp`,
`sprite_resource_destruction.cpp`, `resource_cache*.cpp`, `allocator.cpp`.
The four new sources include only their own header plus `<bit>`, `<cstring>`,
`<cmath>` (verified by grep), and `src/sprite_render_shapes.cpp` uses host
pointers and `long double` intermediates with explicit `volatile double`
stores, so x64 compilation is plausible. **Inference, not verified** — nothing
was compiled for this audit.

**Unknowns and risks to record rather than resolve.**
- Whether every module compiles clean at x64 `-std=c++23` with the registered
  GCC 16.1.0; the notes claim x64 `-O2` unit passes historically, at an
  unrecorded compiler version.
- The indeterminate 16-byte interpolated vertex tail (`sprite_render_clip.md`)
  must stay explicitly unknown; do not initialize it to make a test tidier.
- `472F99`, the device/driver, texture destruction and archive index loading
  stay unrecovered adapters.
- `src/resource_table_load.cpp` and `src/compiled_level.cpp` still have no
  evidence note / ledger row (A3).

**Acceptance tests.**
1. `python tools/agent_control/verify.py --profile all` passes, with the
   recovery profile's registered test count rising from **76 to 81** and the
   five new tests passing at x64 Debug.
2. No `native_*` test is registered in that profile and the reference hash
   check still passes — i.e. A6 holds after the edit.
3. Any source edit needed to compile is behavior-preserving: no change to
   `double`/`long double` types, store order, truncation helpers, growth
   arithmetic or field offsets. Each such edit is listed in the task report
   with its reason; if a module cannot be made to build within the worker
   budget, register the ones that do and record the exact compiler error.
4. `docs/FUNCTIONS.md` gains the rows, each stating the adapters and the
   "offline synthetic original-byte comparison, not in-process gameplay" limit
   from section B, with no new or upgraded claim.

**Explicitly excluded.** No original-game launch, hook, debugger, capture or
install change. No execution of copied original instructions (the x86 oracles
are built-but-unrun by the default verifier and need separate authorization).
No new gameplay, render-gate or complete-equivalence claim; C3 stays FAIL and
C2 stays OPEN. No change to `PROGRESS.md`'s `372 / 6,186` baseline — a coverage
checkpoint needs the counting rules in `PROGRESS.md` §"Counting rules" applied
with deduplication against existing rows, which this audit did not do. No park
reopened.

---

## E. Follow-ups found but deliberately left out of task D

In my suggested order, each one bounded:

1. Register and ledger `texture_creation` + `texture_pixel_format` (A1/A2);
   their `472F99` boundary is a platform adapter, not a port target.
2. Register and ledger `file_stream_open`, and migrate its note's
   reproduction commands to repo-root-relative paths (A7).
3. Write the missing evidence note and ledger row for
   `src/resource_table_load.cpp`, and decide whether `src/compiled_level.cpp`
   is a composition module that needs no row (A3).
4. A fresh coverage checkpoint in `PROGRESS.md` once the ledger reflects the
   recovered modules, following the existing counting rules and keeping the
   `372` baseline row unchanged.
5. Separately and later: decide whether the faithful recovery-tree sprite
   path should inform the main tree's C3 investigation. That is an
   integration question with its own evidence requirements, not a registration
   task.

## F. What this audit does not claim

It does not re-verify any historical comparison, does not establish agreement
with the original game for any routine, does not measure coverage, and does not
assert that the recovery tree renders or plays anything. Source tests passing
after task D would show source consistency only.
