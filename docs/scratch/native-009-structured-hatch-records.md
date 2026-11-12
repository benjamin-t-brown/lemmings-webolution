# native-009: hatch records from the structured level reader

Reference identity: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Read as bytes only. No original executable, copied original instruction,
oracle runner, hook, debugger, capture, launcher or installation change was
used or authorised for this task.

## Traced call path

`src/app/main.cpp:331-352` runs the frontend and sets `args.level` from the map
selection with `args.sim = true`; `main.cpp:537` (and the `--hud-ticks` route at
`main.cpp:628`) calls `sim::loadRetailLevel` -> `loadRetailLevelReport`
(`src/sim/RetailLevel.cpp:56-66`) -> `webo::findLevelObjects`
(`src/assets/LevelObjects.cpp:172`). `loadRetailLevelReport` then consumes the
hatch records as gameplay inputs, not diagnostics:

| Hatch field | Consumer |
| --- | --- |
| `x`, `y` | `hatchSpawnCell` -> `Entrance.x/.y` (RetailLevel.cpp:104-111) |
| `type` | `Entrance.sheetType` |
| `count` | `RetailHatchSummary.count` |
| `rate`, `rateMin`, `rateMax` | clamp -> `RELEASERATE.PC` row -> `releaseIntervalSeconds` / `releaseIntervalTicks` (RetailLevel.cpp:121-137) |

`RetailLevel.cpp:88-91` rejects the level outright when the record list is
empty: no hatch record means no entrance, so nothing can be released.

## Before / after

* Before: `findLevelObjects` obtained the records only from the byte-pattern
  locator `findHatchRecords` (LevelObjects.cpp:302), which requires seven
  consecutive integral doubles with `v[2]==0.0`, `v[5]==1.0`, `v[6]==0.0`, an
  exactly 8 x 5 box, `type <= 8`, `count` in `[1,200]` and `x`/`y` under 256. A
  supported record with a populated counted triple array, or a box that is not
  exactly 8 x 5, is invisible to it, so such a level loads with "no hatch
  record" and releases nothing.
* After: `findLevelObjects` reads the records through the unchanged recovered
  `lemmings::read_compiled_level` (`decomp-lemmings/src/compiled_level.cpp`),
  projecting `gadgets.entities_425e50` in stream order. The locator remains
  only as the fallback for inputs that reader rejects with
  `std::invalid_argument`, and its message is recorded in the new
  `LevelObjects::hatchFallbackReason`.

Executed source-built regression on 2026-10-30: the unchanged
`testSyntheticStructuredHatches` fixture in `src/__test__/test_levelobjects.cpp`
contains two records, one hidden by a populated counted triple array and one
by an 11 x 7 box. An identical driver also tests each record in its own Level
23 buffer. Both runs call the production `findLevelObjects` implementation,
not just the locator:

| Source-built loader | Combined fixture | Populated array alone | Non-8x5 box alone | Result |
| --- | --- | --- | --- | --- |
| Pre-change production source | 0 hatches, expected 2 | 0, expected 1 | 0, expected 1 | 7 checks, 3 expected failures; exit 1 |
| Current production source | 2 hatches | 1 | 1 | 47 checks, 0 failures; exit 0 |

The pre-change source is the exact `src/assets/LevelObjects.cpp` from commit
`b2615a2834efcee5b0a9e7c68a997c39e4f18a21`, extracted read-only with `git show`
to ignored `build/native-009/regression/LevelObjects.before.cpp`; its SHA-256 is
`9a7de5bb3eb00c477faf5fa237abb685fd5610da48ce0d16eceae2f851f5fb97`.
This exactly matches the retained supervisor's pre-implementation
`before-source-hashes.json` entry for `src/assets/LevelObjects.cpp`.
It directly assigns `findHatchRecords(d)` and contains no `readHatchRecords`.
Both builds use the current header and the same unrelated source-built objects
and libraries; this is a comparison of the loader boundary, not a rebuild of a
historical application. No working-tree production source was replaced.

Reproduce with `python build/native-009/capture_regression.py`. It reads the
registered GCC path and retains compiler argument arrays, build logs, driver,
exact legacy source, before/after logs, exit codes and durations in
`build/native-009/regression/`. MinGW retained an unused retail-sweep reference
from the included test file on the initial link attempt (retained in
`initial-link-failure.log`). A separate translation unit copies the current
`readHatchRecords` helper verbatim solely to resolve that reference in the
legacy build; neither that loader nor the synthetic driver calls it. The
legacy production source is unmodified. The current build uses its production
helper directly. Passing checks compare all specified words, box bits and model
names; the tests do not infer those values from production output.

These fixtures exercise `findLevelObjects`; they do not construct a complete
`loadRetailLevelReport` input. The downstream no-hatch rejection and its
removal for an otherwise supported level follow from the unchanged consumer
at `RetailLevel.cpp:88-91`.

## Field mapping

The hatch list is the third gadget list call `0x004487F0`, whose records are
read by `0x00425E50`: the shared spatial base `0x0043130E` (two triples into
hatch+0x48 and hatch+0x60, then two *counted* triple arrays) followed by the
hatch's own fields. `decomp-lemmings/src/level_gadget_records.cpp:62-97`
implements that order; this task did not modify any recovery source or header.

| Recovered field | Original offset | HatchRecord |
| --- | --- | --- |
| `value_b0` | +0xB0 | `type` |
| `shorts_98_9a_9c[0..2]` | +0x98, +0x9A, +0x9C | `count`, `x`, `y` |
| `values_a0_a4_a8[0..2]` | +0xA0, +0xA4, +0xA8 | `rate`, `rateMin`, `rateMax` |
| `double_bits_48_60[0..2]` | +0x48 | `boxMin` (`std::bit_cast`, no arithmetic) |
| `double_bits_48_60[3..5]` | +0x60 | `boxMax` (`std::bit_cast`, no arithmetic) |
| record's optional model name | -- | `model` (empty when absent) |

`HatchRecord::at` stays 0: the structured reader supplies no field offset. No
HatchRecord field was deleted or renamed. `id`, `flag_28`, `flag_38`,
`flag_40`, `flag_41`, `flag_ac`, `flag_ad`, `flag_b6`, `short_b4`, the five
base `double_bits` and both counted triple arrays are deliberately **not**
projected and remain unexplained.

## Inference about the locator's seventh double

INFERENCE, recorded as inference and not as fact: the locator's seventh double
is not a field. The spatial base reads the two triples into +0x48 / +0x60 and
then two counted triple arrays; when both counts are zero their eight bytes
read back as binary64 `+0.0`, which is exactly what the pattern tests for.
That also explains why the pattern's following dword lands on the type at
+0xB0 and the following three u16 on +0x98/+0x9A/+0x9C. It follows that the
pattern can only match records whose arrays are empty. The disassembly cited
in `LevelObjects.cpp` does not state this; it is a reading of the two readers
side by side.

## Retained claims (not re-measured here)

* "list call `0x004487F0` reads its records through `0x00425E50` (149 records
  across the 105 installed entries)" and the shared-reader description are
  RETAINED from `decomp-lemmings/docs/evidence/level_entity_tail.md`. The 149
  figure counts archive entry visits, including base entries overridden by
  `patch.box`, not the 102 effective level names. It was not re-measured by
  this task.
* "OBSERVED over all 144 records in the 102 retail levels" and the MEASURED
  spawn-word figures in `src/assets/LevelObjects.h` are RETAINED from the
  earlier phase-3 notes.
* `src/__test__/test_levelobjects.cpp`'s new retail sweep prints the actual
  structured and locator totals and every structured-only record it finds, so
  the relationship between those two figures is reported from the installed
  data rather than assumed.

## Limits

* This establishes the serialized **read** layout only. It establishes neither
  the original in-memory hatch object, its allocation, its resource resolution
  nor any timing behaviour, and it is not evidence of original-game fidelity.
* No repaired retail frame and no repaired timing gate is claimed. The
  release-timing model, `hatchSpawnCell`, the trigger helpers, the teleport
  pairing and every renderer/simulation consumer are unchanged.
* `require_eof` is off deliberately: this extracts records and is not a file
  validator.
* Source-built tests passing does not prove agreement with the original game.
* Production archive reads stay on `src/assets/Box.cpp`; the recovered
  `box_archive.cpp` is linked only because `load_compiled_level` shares a
  translation unit with `read_compiled_level`.

## Changed files

* `src/assets/LevelObjects.h` -- `readHatchRecords` declaration,
  `LevelObjects::hatchFallbackReason`, `HatchRecord::at` comment.
* `src/assets/LevelObjects.cpp` -- `readHatchRecords`, the preference and
  fallback in `findLevelObjects`, the seventh-double inference comment.
  `findHatchRecords` keeps its signature and behaviour.
* `cmake/webolution_sources.cmake`, `CMakeLists.txt` -- the narrow
  `webolution_compiled_level` static library, linked transitively through
  `webolution_objects`. `level_stream.cpp` stays in
  `webolution_sprite_resource`, which the new target links, so it is compiled
  once per binary. `webolution_texture_decoder` and
  `webolution_sprite_resource` are otherwise unchanged.
* `src/__test__/test_levelobjects.cpp` -- synthetic Level 23 fixture, the
  fallback case and the retail sweep. Every existing assertion kept.
* `docs/scratch/native-009-structured-hatch-records.md` and ignored
  `build/native-009/report.md` -- evidence, verification results and limits.
  This revision changes only those reports and adds ignored source-built
  regression artifacts; the implementation and all four named test files
  are unchanged from the reviewed integration.

## Actual verification and discrepancies

Retained supervisor run `20261030T153427-59928-d8a231a7` executed
`python tools/agent_control/verify.py --profile all`: **70/70 main tests pass**
(372.20 s) and **84/84 recovery tests pass** (31.51 s), verification exit 0.
See `build/agent-runs/20261030T153427-59928-d8a231a7/verification.log` and
`result.json`. This is retained verification, not a new full-suite run during
this revision. It checked reference SHA-256 by reading bytes only and used
`C:/progs/msys2/ucrt64/bin/g++.exe`. The supervisor will rerun the registered
verifier after this worker exits; that result is still pending here.

Fresh registered CTest `levelobjects` run in this revision: **2103 checks,
0 failures**, exit 0; complete test **4.07 s**. The synthetic cases and exact
fallback diagnostic `prefix at byte 4: truncated level string` pass. Log:
`build/native-009/levelobjects-after.log`.

The fresh retail sweep visits **102 effective installed .LVL names** through
the production base/patch overlay: **144 structured records, 144 locator
records, 0 structured-only records, 0 rejected levels**. Every locator record
matched one-to-one on count/x/y/type/rate/rateMin/rateMax. No conflicting
per-level data or existing assertion failure was observed. Sweep duration:
**1.875 s wall**, including **0.334 s structured parsing** and **1.482 s
locator scanning**, rounded independently. The 4.07 s full test and structured
parse remain inside the verifier's 360 s per-test timeout. These measurements
do not re-measure the retained 149-record figure over 105 archive entries.

Recovery cache still has empty `LEMMINGS_REFERENCE_EXE`; its 84-test inventory
has only `native_int_array` with a `native_` prefix and no original-byte oracle.
`docs/STATUS.md`, `decomp-lemmings/docs/PROGRESS.md`, recovery sources/headers,
queue, native-006's deferral and unrelated staged/unstaged work are preserved.
All existing assertions in `test_levelobjects.cpp`, `test_retaillevel.cpp`,
`test_world.cpp` and `test_corpus.cpp` remain unchanged in this revision.
Fresh read-only reference hashing confirms the stated digest. The revision's
`build/native-009/regression/preservation.json` records identical staged diff,
70/84 registrations, empty recovery reference path and source-hash preservation
for every pre-existing nonignored file except this updated note. Both reports
and the regression source/script use LF; `git diff --check` passes. Git
also reports pre-existing installation CRLF normalization warnings; those files
are preserved because installation modification is not authorized.

## Next action

Finish `needs_review`, without editing the queue. The supervisor must run fresh
`--profile all` verification, then an independent read-only reviewer must check
every acceptance criterion, especially the captured pre-change failures and
these completed reports. Only the controller can mark the task done. No further
runtime work, retail frame/timing claim or reopening of D1/D3/digger parks is
part of this task; original object construction, allocation, resource resolution
and timing remain unknown at this serialized-data boundary.
