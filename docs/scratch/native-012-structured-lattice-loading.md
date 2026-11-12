# native-012: structured surface-index lattice loading

2026-10-30. Implementation prepared for `needs_review`; the supervisor owns
queue transitions and independent acceptance. No original executable, copied
original instructions, oracle, hook, debugger, launcher or capture was run.

## Runtime change and files

- `src/assets/LevelIndex.cpp`: after the unchanged title/header/string sweep,
  invokes the recovered prefix and world readers, then moves the first integer
  array into `grid0`. The previous locator extraction remains the fallback.
- `src/assets/LevelIndex.h`: adds `gridFallbackReason` and distinguishes the
  structured lattice from the heuristic string index and historical observations.
- `src/__test__/test_levelindex.cpp`: adds bounded, asset-free serialized fixtures
  and assertions; all existing synthetic and retail assertions remain.
- This report. Source-built drivers, snapshots, logs and preservation manifests
  are ignored under `build/native-012/`.

Before: the first scanned literal `Cylinder` can be a valid earlier resource
name. The old public `indexLevel` then reads unrelated prefix bytes and returns
an empty or unrelated grid. After: the same supported serialized input supplies
the actual world's first integer array to production callers. The executed
decoy fixture returned an empty grid before and all six expected words after.
This is a source-derived supported-input regression, not an observed retail
frame failure or a claim that this fixture describes a complete playable scene.

Real call path, consumers unchanged: `runApplicationLeg` in
`src/app/main.cpp:427` calls `indexLevel(*blob)` and passes its `grid0` span to
`TubeScene::build` at `:429`. In `src/app/TubeScene.cpp`, `:90` guards
`buildCellMeshes(grid0, {}, collision_, mesh.surface.size())` with collision
availability and a nonempty grid; `:100` guards `buildSurfaceMesh(mesh, grid0,
{})` with a nonempty grid. At `:162`, collision availability and a nonempty
grid enable a copy into `poseGrid0_` and `poseLevel_.grid0`, alongside the
existing surface/template inputs. Empty grids suppress those paths; incorrect
words supply incorrect pool indices. This change repairs the serialized input
selection at that production boundary. Cylinder mesh location, collision/light
loading, topology builders, actor pose, renderer and simulation are untouched.
Their prerequisites and branch conditions still apply.

## Retained evidence and exact projection

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Identity is checked by reading bytes only. Static loader sequence retained from
`decomp-lemmings/docs/evidence/004432c1.md` and `world_geometry_prefix.md`:
`0x004432C1 -> 0x00439FD3`, after the prefix/name tables and bounded string.
The world reader consumes intervening blocks and class-one data, the four
counted arrays, 6144 fixed bytes, two strings and 32 tail lists.

`docs/scratch/phase-a4-cell-mesh-rule.md` identifies the first integer array
as the key table at original `world+0x42C`. Instruction `0x0043A07C` selects
that destination and `0x0043A082` calls `0x0043B640`. Existing consumer key:
`layer + (x + y*0x101)*2`, associated with `0x0043C930`. Original addresses,
runtime offsets and call mechanics are retained static evidence, not portable
layout declarations. This adapter introduces no ABI overlay or new calling
convention; existing recovered source interfaces are unchanged. Historical
arena agreement and pixel failures in the A4 note remain attributed historical
evidence and were not re-measured.

An owning `vector<uint8_t>` adapts the input and outlives one `LevelByteStream`.
Production calls unchanged `read_compiled_level_prefix`, then unchanged
`read_world_geometry_prefix` on that stream. It does not read later sections
or require EOF. Only `world.first_integer_array` is moved into `grid0`, with
every u32 bit and position preserved. `grid1` stays empty. There is no fixed
retail length, index filter, reinterpretation, collision-dimension policy or
new interpretation of other fields. Empty decoded arrays are successful.
No floating-point arithmetic or x87 conversion is introduced by this projection;
the recovered readers retain serialized binary64 bits in their existing fields.

Only `std::invalid_argument` from the two recovered reads enables the legacy
locator. `gridFallbackReason` retains its message and byte cursor whether or
not that locator finds a usable array. It is empty on structured success.
The fallback's executable extraction body is unchanged, including its `n > 0`
check and successful partial result with an empty grid if extraction fails.
Title/header validation and capture, string classification/order/counts,
public signatures, `grid1SolidCells`, asset-name helpers, `loadLevelIndex`
archive/error handling and `levelFileNames` remain unchanged. Allocation
failures and unrelated exceptions are not caught. No CMake, recovery source,
recovery header or registration change was needed: the existing
`webolution_compiled_level` and its transitive stream dependency supply the reads.

## Executed regression and asset-free checks

Before production edits, authentic `LevelIndex.cpp` and `.h` were copied under
ignored `build/native-012/`, then moved into its `pre-change/` directory. No
production file was temporarily replaced. `initial-hashes.json` records frozen
production identities and protected-file hashes; its test hash was taken after
the initial test-only fixture addition. `initial-status.txt` records the initial
dirty state. Frozen source SHA-256 is
`2cecb9c123ea9b8fb5cfe98fc7635fedc8dd2aea15cee4bd83e59d18f5020695`;
header SHA-256 is
`8657082bdce96b1584036f03af452532a60821591474965234d8493afbc96d7d`.
Separate `compare_loader.cpp` executables compile the frozen and
current public implementation/header with registered MinGW-w64 GCC, C++23,
`-O0 -g -Wall -Wextra -Wpedantic`, production `Box.cpp`, and existing recovered
libraries for the current loader. These execute source C++ only.

- `pre-change-plain.log`: the authentic old public loader passes the complete
  independently specified six-word lattice expectation without the decoy.
- `pre-change-decoy.log`: expected exit 2, `words=0`, failed complete lattice
  expectation with the earlier serialized `Cylinder` resource name.
- `current-plain.log` and `current-decoy.log`: both pass with words
  `FFFFFFFF, 80000000, 01234567, 00000000, FEDCBA98, 7FFFFFFF`, in order.
- `asset-free.log`: **56 checks, zero failures**, retail environment unset.
  Explicit little-endian u16/u32/u64 primitives construct the new fixtures;
  no recovered C++ layout or original asset is embedded. Populated first/second
  world blocks, class-one triples/reference, integer triples, second integer
  array, byte array and first/last tail lists establish section-order traversal.
  Every lattice word is asserted independently, with empty `grid1`, unchanged
  input, decoy independence, exact-world acceptance and unrelated trailing bytes.
- An empty structured array has a different world name and a later usable
  `Cylinder` locator containing word 77. It stays empty without a diagnostic.
  Prefix truncation with/without a locator remains partial success with empty
  grids. Fixed-byte/tail truncations retain the legacy six-word grid and the
  exact decoder rejection/cursor. A malformed integer count returns a diagnosed
  empty partial grid. Tests use the public expected interface with no exception
  catch that could hide a leaked `invalid_argument`.
- The original non-level synthetic fixture still extracts its four legacy
  words and now asserts the recovered `invalid level name count` rejection at
  byte 95. The first new diagnostic assertion incorrectly expected byte 44;
  `asset-free-first.log` retains that test failure. Correcting that new expected
  cursor did not change production behavior or any pre-existing assertion.

## Newly measured installed corpus

Both drivers enumerate production `levelFileNames(AssetLibrary)` and read
through the production base/patch overlay. The new run measured **102 effective
levels**, from **105 physical compiled-level entries**. Those counts concern
this installed corpus; the older evidence notes retain their historical scope.

Each driver serializes complete `grid0`, `grid1`, title, header,
`cylinderTextures`, `modelTextures`, `scriptVariables`, `shortNames` and
`polyListCount`. The comparison checks all **23,798,224 snapshot bytes**, not
hashes in place of field comparisons, and stops on discrepancy.

| Source loader | Effective levels | Failures | Fallbacks | Grid0 words | Decode duration | Driver duration |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Authentic frozen | 102 | 0 | 0 (no structured route) | 5,085,516 | 0.486048 s | 0.663903 s |
| Current | 102 | 0 | 0 | 5,085,516 | 1.78433 s | 1.973 s |

Zero discrepancies in complete grids or any pre-existing field. Command wall
durations are separately retained in `pre-change-results.json` and
`current-results.json`. These one-run unoptimized source timings describe the
additional serialized traversal/materialization; they are not an original-game
timing measurement or performance gate.

## Verification, preservation and next action

Full registered verification passed in this worker:
`C:/Program Files/Python313/python.exe tools/agent_control/verify.py --profile all`,
exit 0, recorded in `build/native-012/verify-all.log`. Main: **70/70**, 365.96 s;
recovery: **84/84**, 30.16 s. Existing `actorpose`, `surface` and `cellmesh`
checks passed; `levelreturn` passed in 231.73 s under its unchanged timeout.
Retail-enabled `levelindex` passed **62 checks**, including 102 levels and
26,021 resolving asset references. The verifier read the exact reference digest,
built with registered MinGW-w64 GCC and ran the SDL dummy source tests. Both
inventories retain their required counts; recovery has empty
`LEMMINGS_REFERENCE_EXE`, with `native_int_array` its sole `native_`-prefixed
source test. `git diff --check` passed. No registration was added or removed.

`build/native-012/preservation-check.json` records checks across 5,490 initial
manifest files. Only the two production files and the extended test differ
among those files. All 26 top-level installed files, including EXE, archives
and log, remain byte-identical. The frozen extraction body, title/header/string
sweep, helpers and archive functions compare exactly with their retained
production counterparts. Removing only the new test block, diagnostic assertion
and main call reproduces the tracked original test bytes, preserving every
pre-existing assertion. Queue, supervisor, verifier, tool registry, shared
instructions, CMake/source inventories, STATUS and all recovery files remain
byte-identical to the initial manifest; their pre-existing staged/unstaged
changes remain preserved. The new report is the fourth changed task file.

The supervisor will independently run
`python tools/agent_control/verify.py --profile all` after worker exit, then a
separate read-only acceptance review must check every criterion before
completion. No queue entry is edited here.

Serialized loading is distinct from original allocation, runtime object
construction, topology generation, x87 precision/conversion behavior, frame
fidelity and timing. Other scene locators and resource/rendering prerequisites
remain limitations. No repaired-retail-frame or original-game fidelity claim
is made. STATUS gates, native-006 deferral, recovery 372 / 6,186 baseline,
historical failures and D1/D3/digger parks remain unchanged. Next action:
supervisor verification and independent review; leave this task `needs_review`.
