# native-014: structured serialized cylinder loading

2026-10-30. Implementation prepared for `needs_review`; the active supervisor
owns queue transitions, full verification and independent acceptance. No original
executable, copied instructions, oracle, hook, debugger, capture or launcher ran.
Installed files were read only. No CMake or recovery implementation changed.

## Runtime change and changed files

`src/assets/Cylinder.cpp` now invokes the unchanged recovered prefix/world
readers before considering the authentic locator. It projects counted ring
metadata and vertices, then calls the shared existing radius/surface/tile-tail
parser. `src/assets/Cylinder.h` adds `CylinderMesh::meshFallbackReason` and
updates the public loader comment. All pre-existing public fields, signatures,
geometry helpers and the vertex accessor convention remain.
`src/__test__/test_surface.cpp` adds asset-free public-loader assertions before
retail handling; all previous test code remains. This note is the fourth task
file. Frozen sources, source-only drivers, snapshots and reports are ignored
under `build/native-014/`.

Before: the public loader required an initial `(0,0,radius)` signature and
inferred the vertex count by plausibility scanning. It rejected the independently
specified finite non-axis/nonzero-height fixture. After: the same public API
returns its counted vertices in serialized order. This is an executed
source-derived supported-input regression, not an observed retail-frame failure.

The production call path is unchanged outside the loader:

- `src/app/main.cpp:301` defines `runApplicationLeg`; it reads the selected level
  through `AssetLibrary`, calls `findCylinderMesh` at line 422 and returns on
  failure. At line 429 it supplies the mesh and level bytes to `TubeScene::build`.
- `src/app/TubeScene.cpp:48` builds the scene, copies the mesh at line 57, loads
  both existing tile-name vectors at lines 85-86 and builds surface faces at
  line 101. Its skin drawing still obtains corners through `mesh_.vertex`
  at lines 504-507.
- The same scene copies `surface` and the cylinder template into actor-pose
  storage at lines 164-169. `prepareInjectedWalker` consumes the pose level at
  lines 339/362. `src/app/ActorPose.cpp` retains its existing surface-index lookup
  and `interpolateCylinderTemplateD2B8` fallback through `CylinderMesh::vertex`.

Renderer consumers, collision/light/index loading, object construction,
simulation and all user parks remain unchanged.

## Retained evidence and exact adapter mapping

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Identity was checked by reading bytes only. Retained static evidence:
`decomp-lemmings/docs/evidence/004432c1.md` and `world_geometry_prefix.md`.
The sequence is `0x004432C1 -> 0x00439FD3 -> 0x0043D0F0` after the fifth
name table and following bounded string. `0x0043D0F0` reads u32, binary64,
then counted arrays through `0x0043E5A0/0x0043D09A` and
`0x0043E500/0x0043D060`. First-array records contain 64 triples (1,536
serialized bytes); second-array records contain 32 doubles (256 bytes).
The retained evidence pins body addresses, sizes and hashes. This task did
not perform fresh disassembly or run those routines.

The production loader uses one `LevelByteStream` backed by a locally owned
uint8 vector for both reads. It saves the cursor immediately after
`read_compiled_level_prefix` as `worldStart`, then calls
`read_world_geometry_prefix`. Both implementations and their string behavior
remain unchanged. A complete supported world section is needed; later level
sections and EOF are not required.

| Recovered serialized data | Existing compatibility field |
| --- | --- |
| `leading_value` | `faceCount`, retaining its unidentified meaning |
| `following_double_bits` | `thickness` via `bit_cast<double>` |
| `first_blocks.size()` | `ringCount` |
| 64 triples per block / existing two-slot convention | `angularSteps = 32` |
| Blocks, triples, then components in stream order | Complete `vertices`, each component via `bit_cast<double>` |
| Saved prefix cursor | `offset = worldStart` |

No arithmetic, origin check, plausible-vertex check, radial-layer
reinterpretation or reordering is applied to structured vertices. The existing
nonzero/at-most-64 ring and finite-positive-thickness policies are explicit
portable adapter checks after decoding. They are not asserted to be original
validation. Negative-zero and quiet-NaN vertex bits are retained without
evaluating those vertices through geometry arithmetic in the fixture checks.

The shared tail starts at `worldStart + 16 + first_blocks.size() * 1536`.
The sizes derive from the validated serialized shape, with compile-time block
assertions and the 64-block adapter bound. No retail offset or scan positions it.
The tail still reads the repeated radius count, radii, `one`, surface vertices,
zero word and both existing counted-string slot blocks. Its executable body,
error strings, field writes and final tile boundary are preserved; only its
initial cursor is now a parameter. `endOffset` remains that tile boundary,
not the recovered reader's final cursor. There is no projection from
`named_references`, normalization or new tile semantics.

Only `invalid_argument` from the two recovered reads enables the legacy route.
Successful fallback records the exact reader section, byte cursor and cause in
`meshFallbackReason`; structured success leaves it empty. If legacy loading
also fails, the returned `expected` error retains both causes. Decoded metadata
and shared-tail validation return directly, even when a usable locator fragment
follows. Allocation failures and unrelated exceptions are not caught.

## Executed source regressions

Authentic `Cylinder.cpp` and `Cylinder.h` were frozen before production edits
under `build/native-014/pre-change/`. SHA-256:

- `.cpp`: `ad71a40f28369ad24ebc65d680e06be4bbca8a8be1e78d8841c4fecec24ec836`
- `.h`: `f279030ebfd460173d8de62a4c16db36045fa0ce9a170452d8e1d0cd63442d89`

`run_comparison.py pre-change` and `run_comparison.py current` build separate
drivers against their respective public implementation/header, without replacing
production files. They use registered MinGW-w64 GCC 16.1.0 with C++23,
`-O0 -g -Wall -Wextra -Wpedantic`, unchanged production `Box.cpp` and
`LevelIndex.cpp`, and the existing compiled-level/stream libraries. Fixture
construction uses explicit little-endian primitives; expectations specify all
metadata, all triple/radius bits, both slot vectors and both offsets independently
of the implementation. Helpers are extracted from the asset-free test.

| Non-axis-first-vertex supported fixture | Result |
| --- | --- |
| Authentic frozen public loader | Exit 2: `no cylinder mesh found` |
| Changed public loader | Exit 0: complete serialized mesh expectation passes |

Logs: `pre-change-fixture.log`, `current-fixture.log` and both `*-compile.log`.
The fixture has first vertex `(1.25,3.5,50.25)`, two counted blocks, 128
distinguishable triples, thickness 3.25 and 64 distinguishable radius values.
It has a complete supported world tail, three surface vertices and two slots
per tile block. Expected world/radius/tile boundaries are independently fixed
at bytes 76 / 3164 / 3806. Unidentified words are deliberately not retail values.

Final `asset-free.log`: **896 checks, zero failures**, retail environment unset.
Checks include every component bit/order, complete counts, radii/surface,
tile slots, exact offsets, unchanged input, trailing usable locator acceptance,
negative zero and quiet NaN payload `0x7FF8000000001234`. These are serialized
retention checks, not original x87 conversion checks.

Additional public-loader checks cover:

- Complete non-level legacy fragment, preserving its one-step ring, original
  metadata/slot representation, offsets and exact prefix diagnostic at byte 120.
- Rejected world count with later usable fragment, retaining exact world-reader
  diagnostic at byte 92 and the later fragment's actual offset/endOffset.
- Empty input retaining both reader and locator causes; eleven prefix/world/tail
  truncations returning combined errors and six legacy hard-tail truncations.
- Invalid ring counts 0 and 65; accepted counts 1 and 64; thickness positive
  zero, negative zero, negative, infinity and quiet NaN rejected as appropriate.
- Radius-count mismatch and a supported nonempty class-one second-triple array
  rejected by the existing tail policy. Each decoded validation fixture includes
  a proven usable later fragment, yet returns its direct validation error.

No broad test exception catches mask the public `expected` contract.
Final focused registered GCC build and CTest: **4/4 passed, 63.74 seconds**:
`actorpose`, `tileposition`, `surface`, `corpus`. Reproduction:
`python build/native-014/run_checks.py`; logs/JUnit: `focused-build.log`,
`focused-ctest.log`, `focused-results.xml`. Earlier focused checks also passed;
the final run includes the additional world-fallback/zero-word assertions.

## Newly measured installed corpus

Both drivers enumerate production `levelFileNames(AssetLibrary)` and read every
effective level through patch-over-base production overlay. Actual scope:
**102 effective levels / 105 physical `.LVL` entries**, with **zero loader
failures**. Current: **102 structured successes / zero fallbacks**. Frozen:
all 102 use its original locator; structured/fallback counters are inapplicable.
Older 102/105 observations in other notes retain their historical attribution;
these counts are from this run.

The two **4,129,373-byte** snapshots match exactly. They include every name
and pre-existing `CylinderMesh` field: `faceCount`, thickness bits, ring count,
angular steps, `one`, offsets, complete vertices/surface/radii (every double
bit) and both complete tile-name vectors, including empty slots and lengths.
No assertion was weakened; there were **zero mesh discrepancies**.

| Source driver | Loader calls | Corpus wall time | Process duration |
| --- | ---: | ---: | ---: |
| Frozen | 0.0192793 s | 0.0937502 s | 0.1051863 s |
| Current | 1.33632 s | 1.42379 s | 1.4347259 s |

These are one source/asset sweep per driver, including snapshot output and full
ordered world decoding for current calls. They are not gameplay timing or a
benchmark guarantee. See `*-corpus.log`, `*-corpus.bin`, `*-results.json`.
The measured additional decoding cost remains visible rather than being
hidden by the correctness comparison.

## Preservation, verification limits and next action

`preservation-check.json` accounts for 1,627 captured initial files. Only the
three intended existing source/test files changed. All other captured hashes,
including reference/archives, installation files, queue, tool registry,
supervisor/verifier, STATUS gates, recovery PROGRESS and coverage baseline
remain unchanged. Staged work is byte-for-byte preserved. The complete locator
is textually identical when its shared-tail call is replaced by the frozen tail;
the shared tail body and geometry helpers are unchanged. Removing only the new
surface test code/includes/main call reproduces its original SHA-256.
All task text uses LF; no assets were added to version control. Pre-existing
installation CRLF warnings were observed during snapshots and those files
were left untouched.

Fresh read-only inventories retain **70 main / 84 recovery registrations**.
Recovery has empty `LEMMINGS_REFERENCE_EXE` and `native_int_array` as its sole
`native_` source test. The full `python tools/agent_control/verify.py --profile all`
run remains assigned to the active supervisor after this worker exits, as
instructed. The focused runs and inventory checks do not claim that pending
full run passed. Next: supervisor full verification with registered tools,
then independent read-only review of every native-014 acceptance criterion
before `done`. The worker does not edit the queue.

Remaining limitation: the shared tail is the existing compatibility parser,
including its zero-word requirement, length/count limits and two slot-block
interpretation. It does not become a general projection of recovered class-one
records. A supported serialized class-one second-triple array can therefore
decode successfully and still be rejected directly by this tail; the new test
records this limit. Changing that representation or renderer consumers needs
separate evidence and a bounded runtime task. A possible next task is to
establish the required tail contract for such serialized variants before any
further renderer integration.

This task establishes serialized loading and unchanged compatibility output on
the measured corpus. Original allocation, ABI/calling conventions, object graph
construction, coordinate-layer ordering, x87 arithmetic/conversion, renderer
topology, timing and frame fidelity remain outside the claim. There is no
original memory-layout overlay or CRT reconstruction. The historical
372 / 6,186 recovery baseline, native-006 deferral, failed gates and user-parked
D1/D3/digger work remain untouched.

## Retry: supervisor timeout remains unresolved

The first native-014 supervisor run
`20261030T181125-40868-e466f60a` executed the registered
`python tools/agent_control/verify.py --profile all`. Main verification passed
69/70 tests but `levelreturn` timed out at 240.03 seconds; recovery was not
reached. This is a failed full verification, not a full pass. The preceding
native-013 supervisor run `20261030T174831-40868-e1752c89` passed 70/70 main
and 84/84 recovery tests, with `levelreturn` taking 231.81 seconds. Those are
historical results, not this retry's results.

The retry inspected the existing implementation and retained it without further
production/test edits. `build/native-014/retry_checks.py` built fresh frozen and
current public-loader drivers using the registered compiler, and stored new
logs/snapshots under `build/native-014/retry/` without replacing the first
attempt's evidence. The supported non-axis fixture again fails with the frozen
loader (exit 2) and passes with current (exit 0). Both installed sweeps again
cover 102 effective levels / 105 physical entries, with zero failures; current
has 102 structured successes and zero fallbacks. Complete 4,129,373-byte
snapshots again match exactly, including every pre-existing mesh field.

| Retry driver | Loader calls | Corpus wall time | Process duration |
| --- | ---: | ---: | ---: |
| Frozen | 0.0199116 s | 0.0976798 s | 0.1094611 s |
| Current | 1.32808 s | 1.41281 s | 1.4235414 s |

The registered focused targets rebuilt successfully. The fresh asset-free
public-loader run passed **896 checks / zero failures**. An isolated registered
`levelreturn` run then timed out again at **240.07 seconds** (CTest exit 8).
The retry script stops at that failure; its later four-test command did not
execute. The earlier focused 4/4 pass remains attributed to the first attempt.
No assertions, test timeouts, verifier, build settings or consumers were changed
to accommodate the failure. These observations do not establish the timeout's
cause or rule out a regression elsewhere in the application.

`retry/static-audit.json` records another exact comparison of the preserved
legacy scan, shared tail and geometry helpers, preservation of every original
surface assertion, authentic frozen hashes, LF task text, the reference digest
read from bytes, 70/84 retained registrations, empty recovery reference-execution
setting and sole `native_int_array` native-prefixed registration. All captured
files were unchanged from retry entry during these checks; the subsequent
changes are this note and ignored reports only. The queue remains controller-owned.

Implementation is prepared for `needs_review`, but the full-verification
acceptance criterion remains unmet. Next: the active supervisor runs the full
registered verifier after worker exit. If `levelreturn` still times out, retain
the failure and stop for a bounded test-runtime investigation decision; do not
mark native-014 done or weaken its acceptance criteria. Independent read-only
acceptance is still required after verification passes. The compatibility-tail
and original-fidelity limitations above remain unchanged.
