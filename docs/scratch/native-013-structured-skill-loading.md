# native-013: structured serialized skill-stock loading

2026-10-30. Implementation prepared for `needs_review`; the supervisor owns
queue transitions, full verification and independent acceptance. No original
executable, copied original instructions, oracle, hook, debugger, capture or
launcher was run. Installation files were read only.

## Runtime change and changed files

`src/assets/LevelObjects.cpp` now calls unchanged
`lemmings::read_compiled_level(bytes, false)` in production `findLevelSkills`,
projecting `gadgets.values_27d0[0..7]` in order. The authentic backward scanner
is retained as internal `findLegacyLevelSkills`, with its executable body
unchanged. `src/assets/LevelObjects.h` adds `LevelSkills::skillFallbackReason`
and corrects the serialized-marker comments. All existing public fields and
function signatures remain. `src/__test__/test_levelobjects.cpp` adds bounded
complete-level fixtures and public-loader checks. This report is the fourth
changed task file; drivers, manifests, snapshots and logs are ignored under
`build/native-013/`.

Before: the loader chose the last incidental little-endian `u32 1, u32 8`
pair and treated eight words 36 bytes earlier as stocks. A supported level
without that pair was rejected, while a later final-record pair could select
unrelated words. After: the same inputs supply the first eight serialized panel
words. These are executed source-derived regressions, not observed retail
gameplay failures or repaired retail frames.

Traced production paths, consumers unchanged:

- `src/app/main.cpp:537` loads `sim::loadRetailLevel`, which delegates to
  `loadRetailLevelReport` (`src/sim/RetailLevel.cpp:192`). The report calls
  `findLevelSkills` at `:72`, then copies the eight counts to `LevelSetup::skills`
  at `:184`. `World::assign` (`src/sim/World.cpp:948`) tests that stock and its
  existing successful-assignment path decrements positive stock. The HUD loading
  branch at `main.cpp:628` uses the same adapter.
- `main.cpp:748` calls `RetailSkillPanel::create`. That function reads the level
  through the production archive library, calls `findLevelSkills`
  (`src/app/SkillPanel.cpp:207`) and copies the eight stocks into
  `initialCounts_` at `:223`. The existing wrapper draws these initial counts
  at `:282`; runtime drawing instead uses the current World's stocks through
  `runtimeSkillPanelCounts` / `drawRuntimeSkillPanel`
  (`src/app/RuntimeSkillPanel.cpp:50`, `:58`).

Selection, assignment eligibility, depletion, command counts, hatch/placement
extraction, panel rendering, archive precedence and gameplay timing are unchanged.
No CMake, recovered-source/header or recovery-subproject change was needed:
the existing `webolution_compiled_level` target and transitive stream dependency
already provide this reader.

## Evidence, projection and fallback contract

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Identity was checked by reading bytes only.

Retained evidence: `decomp-lemmings/docs/evidence/level_entity_tail.md` identifies
the eleven-integer read through `0x0045B86D`, following the entity lists and
preceding the counted integer array, flags/short/integer and final records.
`docs/scratch/phase-c1bg-selected-index.md` independently records ten panel
dwords then one more, and the historical `MM_CLIMB_002` sequence
`10,0,0,0,0,0,0,0,0,1,8`. `phase-c1-panel-indices.md` establishes physical
indices 0..7 as climber, floater, bomber, blocker, builder, basher, miner, digger.
The selected-index and runtime-skill-panel notes retain their original scopes;
no fresh native selection or UI observation was made.

The recovered serialized representation is `LevelGadgetRecords::values_27d0`,
an eleven-u32 array. Only indices 0..7 are copied, with no reordering or numeric
conversion. The other three words receive no projection or new semantics.
Original panel destinations and selected-index offset `+0x28` are retained
evidence, not the layout of portable `LevelSkills` or runtime initialization
performed here. There is no ABI overlay, new calling convention, floating-point
arithmetic or x87 conversion in this projection. Original allocation and object
construction remain outside the serialized-data API.

The adapted uint8 span is backed by the caller's unchanged input storage for
the entire synchronous read. `require_eof=false` accepts bytes after a complete
level. Only `std::invalid_argument` from the recovered read enters legacy
scanning. Structured success leaves `skillFallbackReason` empty. Successful
fallback keeps the reader's exact decorated section/byte-cursor rejection;
combined failure returns both reader and legacy causes. Allocation failures
and unrelated exceptions are not caught.

The existing `count > 200` rejection remains portable adapter policy, not
original validation. It runs after successful decoding, outside the fallback
catch: a decoded 201 returns its validation error even with a later usable
legacy trailer. No `1,8` signature is imposed on structured input.

## Executed source regressions and checks

Authentic `.cpp` and `.h` were frozen before production edits under
`build/native-013/pre-change/`. Frozen SHA-256 values:

- `LevelObjects.cpp`: `3d568292273962ceea5dd8358c7d5064703cdda62183c305e3fd4b99b20f648f`
- `LevelObjects.h`: `386b1e6a640bb0c9f4eb0dc90cd782e5502df6abdfca2dc287ff559f8e51a5eb`

Separate source-built drivers compile the frozen or current implementation and
header, without replacing production files. Both use registered MinGW-w64 GCC
16.1.0, C++23, `-O0 -g -Wall -Wextra -Wpedantic`, unchanged production archive,
index and mesh dependencies, and the existing recovered libraries. The fixture
construction is extracted from the test's explicit little-endian primitives;
the driver independently specifies all eight expected stocks.

| Public-loader fixture | Frozen result | Current result |
| --- | --- | --- |
| Complete panel without `1,8` | exit 2: `skill trailer marker not found` | exit 0, `0,200,2,17,33,41,59,83` |
| Later populated final-record list containing `1,8` | exit 2: wrong vector `102,103,104,105,106,107,108,109` | exit 0, `0,200,2,17,33,41,59,83` |

Logs: `pre-change-{plain,later}.log`, `current-{plain,later}.log`;
reproduction: `python build/native-013/run_comparison.py pre-change` and
`python build/native-013/run_comparison.py current`. Initial standalone linking
failed for missing `unrollCylinder`; `first-compile-failure.log` retains it.
Linking unchanged production SurfaceMesh/Cylinder dependencies resolved that
driver setup failure; no stubs or production replacements were used.

`asset-free.log`: **68 checks, zero failures**, retail environment unset.
Complete fixtures retain bounded empty earlier collections and populate the
following three-integer array and four final three-word records. Distinct
unprojected panel words are `12345678,89ABCDEF,FEDCBA98` (hex). Checks cover all
eight stocks, zero/200 boundaries, unchanged input, unrelated usable trailing
legacy words and empty structured diagnostics. A 44-byte non-level trailer
returns all eight legacy stocks and exact diagnostic
`prefix at byte 44: truncated level flags`. Empty input retains both causes.
Five truncations cover prefix, panel, following array and final records; an
invalid following-array count also returns both causes through `expected`.
A decoded 201 at slot 5 with a usable later trailer returns only
`implausible skill count 201 at slot 5`. Tests do not catch leaked exceptions.

Final focused GCC build and CTest: **5/5 passed, 9.37 seconds**: `levelobjects`,
`retaillevel`, `world`, `skillpanel`, `runtimeskillpanel`. Logs/JUnit:
`focused-build.log`, `focused-ctest.log`, `focused-results.xml`;
`run_checks.py` reproduces these checks. All prior hatch fixtures/assertions,
literal tutorial/four-skill vectors and consumer assertions remain intact.
Removing only native-013 additions reproduces the initial test SHA-256.

## Newly measured installed corpus and remaining acceptance

Both public-loader drivers enumerate production
`levelFileNames(AssetLibrary)` and read through production patch-over-base
overlay. Actual corpus: **102 effective levels**, **105 physical compiled-level
entries**, **816 stock values**. Every name and complete eight-count vector
matches byte-for-byte in the two 5,909-byte snapshots. There were **zero loader
errors and zero stock discrepancies**. Frozen loading uses the legacy route
for all 102; its disabled structured-fallback counter is not applicable.
Current loading has **102 structured successes and zero fallbacks**.

| Driver | Public-loader calls | Corpus wall time | Process duration |
| --- | ---: | ---: | ---: |
| Frozen | 0.0000216 s | 0.0678992 s | 0.0780809 s |
| Current | 0.339354 s | 0.410479 s | 0.420642 s |

These are one source/asset sweep per driver, including full ordered decoding
for current calls, not gameplay timing measurements or a benchmark guarantee.
Details: `*-corpus.log`, `*-corpus.bin`, `*-results.json`. Historical corpus and
live observations in earlier notes retain their attribution and exclusions.

`preservation-check.json` confirms unchanged unrelated tracked-file hashes,
reference/archives, queue, STATUS gates, recovery PROGRESS/coverage baseline,
native-006 deferral and parks; unchanged legacy executable body; and unchanged
non-skill LevelObjects.cpp code. Fresh read-only CTest inventories retain
**70 main / 84 recovery** registrations, with empty `LEMMINGS_REFERENCE_EXE`
and `native_int_array` as the sole recovery `native_` source test. Task text
uses LF. No assets were added to version control or installed files modified.

The full `python tools/agent_control/verify.py --profile all` run is deliberately
left to the active supervisor after this worker exits, as instructed. The
focused checks and inventory inspection above do not claim that pending full
run has passed. Next: supervisor full registered verification, then separate
read-only review of every acceptance criterion before `done`.

This establishes serialized stock loading and source/asset consistency only.
Original runtime allocation, selection initialization, command behavior, UI
fidelity and gameplay timing remain outside this task's evidence. No new
original-game fidelity, repaired-frame or parked D1/D3/digger claim is made.
