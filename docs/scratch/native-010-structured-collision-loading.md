# native-010: structured collision-grid loading

2026-10-30. Implementation prepared for `needs_review`; full supervisor
verification and independent acceptance review remain pending at worker exit.
No original game, copied instructions, oracle, hook, debugger, capture or
launcher was executed. Installation files were only read.

The production change is in `src/assets/CollisionGrid.cpp` and
`src/assets/CollisionGrid.h`; checks are in `src/__test__/test_collisiongrid.cpp`.
Existing staged/unstaged work was inspected first and preserved. Initial status,
diff, selected protected-file hashes and frozen authentic loader source/header
are retained under ignored `build/native-010/`.

## Runtime path and evidence

`src/app/main.cpp` frontend selection enables simulation; its simulation path
calls `sim::loadRetailLevel` (`src/sim/RetailLevel.cpp`), then
`loadRetailLevelReport` -> `findCollisionGrid` -> `Terrain::fromCollisionGrid`.
Separately, `main.cpp` -> `app::TubeScene::build` (`src/app/TubeScene.cpp`)
calls the same `findCollisionGrid`, retains its cells and uses positive extent
as the scene cylinder extent. These consumers and Terrain conversion are
unchanged, including Terrain's existing Y flip and flag/solidity rules.

Reference EXE, read and SHA-256 checked without execution:
`Lemmings Revolution/Lemmings Revolution.exe`,
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Retained static evidence:

- [004432c1](../../decomp-lemmings/docs/evidence/004432c1.md): prefix/name
  tables followed by the world reader.
- [world_geometry_prefix](../../decomp-lemmings/docs/evidence/world_geometry_prefix.md):
  loader `0x004432C1` calls `0x00439FD3`; ordered world arrays and tail reads.
- [world_state](../../decomp-lemmings/docs/evidence/world_state.md):
  `following_value` passes through `0x0043AA63` to height at world `+0x418`;
  `second_integer_array` is loaded into collision storage at `+0x41C`.
  Intervening arrays correspond to `+0x42C` and `+0x43C`.

The production adapter owns a `uint8_t` copy for the stream's whole lifetime,
calls unchanged `read_compiled_level_prefix`, then unchanged
`read_world_geometry_prefix` on that same stream, and stops there. Entity and
gadget sections and EOF are not required. Projection is exactly:

- width remains 256; `following_value` becomes height after the existing
  positive/4096-row adapter bound;
- `second_integer_array` moves into cells with exactly `256 * height` words,
  preserving every u32 bit and serialized row order;
- `following_double_bits_2` becomes extent by `std::bit_cast<double>`, with no
  floating arithmetic, conversion or x87 rounding. The correspondence to runtime
  world `+0x408` is still the existing inference, not a new native pairing.

The dimensions and ownership are portable adapter policy, not original
validation/allocation. No original calling convention or object layout is
imposed on these portable structures. `world_state.cpp` remains an existing
tooling ownership adapter, not a recovered original routine; it is unchanged.
No unrelated field meanings or projections were added. Recovery sources,
headers, layout assertions, allocator boundaries and build registrations remain
unchanged. The existing compiled-level library supplies these readers and its
transitive stream dependency; no duplicate stream compilation was introduced.

Only a recovered reader's `std::invalid_argument` enables the internal legacy
locator/walk. Its function body was compared to the authentic frozen pre-change
body and is identical. Successful fallback records the decoder's message and
cursor in `CollisionGrid::diagnostic`; structured success leaves it empty. When
fallback fails, the expected error retains both causes. Successfully decoded
dimension/count errors return directly, even with a usable legacy chunk later
in the input. Allocation failures and unrelated exceptions are not caught.

## Before/after and actual checks

Frozen source SHA-256:
`29bbab73ea33350a89f6d0be148b6bfc87d8fc9b64fe24b6ecec9ae837f9ab6c`.
`build/native-010/compare_driver.cpp` was compiled with that unmodified loader
before production edits, using registered UCRT64 GCC 16.1.0, C++23, `-O0 -g`.
The pre-change public loader rejects the earlier name-table `Cylinder` with
`collision grid: implausible height 0` (exit 2). With the decoy removed it loads
256x2/512 cells, extent bits `8000000000000000`. The current public loader loads
both variants with those dimensions and bits. `prechange-{decoy,plain}.log` and
`current-{decoy,plain}.log` retain output. The decoy input used for this authentic
regression is byte-identical to the final C++ test's synthetic patch payload.
This is a supported serialization scenario, not an observed retail-frame bug.

Final focused source checks:

- Registered CMake target `test_collisiongrid` builds successfully.
- No `WEBOLUTION_GAME_DIR`: **57 checks, 0 failures** before the retail early
  return. Includes complete independently specified cells, high-bit words,
  negative-zero bits, unchanged caller input, decoy independence, populated
  intervening arrays/tail lists, trailing unrelated bytes, both reader fallback
  routes, both-cause errors, truncations and decoded dimension/count rejection.
- Synthetic base/patch archive loading passes in fresh test-owned directories
  under `build/native-010/archive-test-*`, resolved via `WEBOLUTION_SOURCE_DIR`.
  Fixtures are generated bytes; no retail assets are copied into them.
- Installed-data direct run: **111 checks, 0 failures**; all original corpus,
  actor-light, Terrain and historical dump assertions remain. Five existing
  historical dumps were compared successfully. No dumps were captured; absent
  dumps are explicitly reported as skipped.
- Registered CTest `collisiongrid`: **1/1 passed, 1.45 s**.
- Read-only inventory check: **70 main / 84 recovery registrations**; recovery
  cache has empty `LEMMINGS_REFERENCE_EXE`, with `native_int_array` the only
  `native_`-prefixed test. This inventory check does not claim full-suite passage.
- `git diff --check` passes; changed source files have LF endings. Git retained
  its pre-existing line-ending warnings for installation files, which were not
  modified. Final status adds only the three intended source/test files and
  this note to the initial dirty state; no initial status entry was removed.

One intermediate added test expected a truncated-double error; the unchanged
reader actually rejects the insufficient tail record count first. The assertion
was corrected to `invalid world tail count` after reading that guard. Failed
logs remain as `world-tail-expectation-failure.log` and
`ctest-world-tail-expectation-failure.log`. No production logic was changed for
that test correction.

The authentic pre-change and current drivers read every effective installed
`.LVL` through `AssetLibrary` overlay resolution. **102 levels, 2,506,752 cell
words, zero discrepancies**, comparing names, width, height, every cell word
and raw extent bits. Current fallback count is **0/102**; the old loader used
its locator for all 102 (its driver counter is 0 because it had no structured
fallback concept). Measured load-and-snapshot loop durations: old **0.0938904 s**,
current **1.35393 s** in these debug source drivers. This includes parsing and
snapshot output; it is not a game-frame timing measurement. Structured loading
does more parsing/allocation than the former locator. Both complete comparison
snapshots are 10,031,693 bytes and compare identically; see `comparison.log` and
`compare_snapshots.py`. Historical **105 archive entries** in recovery evidence
and the original live-grid observations are retained history, not new counts.

## Limits and handoff

This establishes serialized loading, not original allocation, object
construction, resource resolution, collision timing, frame fidelity, or a
repaired retail frame. It does not reopen native-006, D1/D3/digger parks or
change STATUS gates, historical failures, recovery coverage or the retained
372 / 6,186 baseline. No queue or controller files were edited.

The worker ran the focused checks above and stops early for the enclosing
supervisor to run `python tools/agent_control/verify.py --profile all` using
registered tools after worker exit, as instructed. The full suite is pending,
not claimed passed here. Next: retain that supervisor result and independently
review every native-010 acceptance criterion before marking the task done.
