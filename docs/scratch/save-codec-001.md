# save-codec-001 — persist level completion in an original-layout SavedGame.dat

Status: implementation and source-built restart demonstration verified by the
supervisor in run `20261030T231837-6120-8b71fdf9`: **74/74 main, 84/84 recovery**.
Independent review remains pending after this report correction. Workers had no
shell; executed results below come from the supervisor, not worker execution.
Authority: `docs/scratch/save-format-research.md` (reader 0x45F43C, writer 0x45F94D,
reference exe SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`).
No original execution, hook, debugger, capture, oracle, launcher or installation change.

## Milestone step and route

Finish a level -> retain progress after restarting the clone.

```
main() [options.useDefaultSaveStore = true]
 -> runApplication (src/app/main.cpp)           selects options.saveStore, else save::makeDefaultSaveStore(gameDir)
 -> FrontendSession::create(library, store)     SaveStore::load -> SavedGame::parse -> nodeFlags(MapScene::nodeMapping())
                                                -> completionFlags_, map_ (loaded flags), initialMap_ (zero flags)
 -> map click -> level leg -> LevelOutcome success
 -> FrontendSession::completeLevel              in-memory commit, then savedGame_.markComplete(group, slot) + persist()
 -> SaveStore::save -> FileSaveSink::write      temp file + MoveFileExW(REPLACE_EXISTING)
 confirmChoice(yes) (New Game)                  reset, savedGame_ = zero image, persist()
```

Source-traced failure (not an observed retail-frame failure): before this task
`FrontendSession::create` had no flags input, `completionFlags_` started all-zero and
`completeLevel`/`confirmChoice` only mutated memory; `--help` said "no save writes";
`src/shell/Progress.*` has no production caller and is not wired.

## Changed files

| File | Change |
|---|---|
| `src/save/SavedGame.{h,cpp}` (new) | pure codec, no I/O |
| `src/save/SaveStore.{h,cpp}` (new) | `SaveSink` interface, `FileSaveSink`, `SaveStore`, path guard, LocalAppData resolver |
| `src/app/MapScene.{h,cpp}` | `MapScene::nodeMapping()` exposes `kMapping`; `build()` unchanged |
| `src/app/FrontendSession.{h,cpp}` | optional store; load in `create`; save in `completeLevel` and New Game Yes |
| `src/app/AppEntry.h`, `src/app/main.cpp` | `ApplicationRunOptions::{saveStore,useDefaultSaveStore}`; `main()` opts in; help text |
| `cmake/webolution_sources.cmake`, `CMakeLists.txt` | new sources; `test_savedgame`, `test_savepersist` |
| `src/__test__/test_savedgame.cpp`, `test_savepersist.cpp` (new), `test_levelreturn.cpp` | tests (temp stores only) |
| `src/__test__/test_savedemo.cpp` (new, attempt 3), `CMakeLists.txt` | frozen-before / current-after restart driver: `savedemo_after`, and `savedemo_before` when the ignored frozen sources exist |
| `README.md` | where progress is stored; installed save never touched |

## Field table

| Offset | Size | Field | Evidence |
|---|---|---|---|
| 0x000 | 4 | flag int32 LE | disassembly (reader `D:122177`, writer 0x45F94D); meaning inferred only; sample is zero |
| 0x004 + 12n + 0 | 4 | A | disassembly; meaning unknown; preserved |
| 0x004 + 12n + 4 | 4 | B | disassembly; meaning unknown; preserved |
| 0x004 + 12n + 8 | 4 | C (completion) | disassembly (tested `!= 0`, written 1 at 0x485E55); n = group*25 + slot |

Installed sample `SavedGame.dat`: 1504 bytes, all zero. It confirms size and the zero
state only. Nonzero field positions rest on disassembly, not on a sample; a byte-layout
test proves source consistency, not retail compatibility.

## Decisions and deliberate deviations (clone policy, not original behaviour)

* Strict size: exactly 1504 bytes. The original reads short files as indeterminate stack
  data and ignores trailing bytes; the clone rejects both with specific errors
  (`Empty`, `TooShort`, `TooLong`).
* The flag is kept verbatim. The original stores `value != 0` and would rewrite 0/1.
* Writes are atomic (temp file in the same directory + replace). The original rewrites in
  place and ignores open failure. Original write-failure behaviour is not copied (U7).
* Missing file = zero state (matches the original open-fails path). Wrong-size/unreadable
  file = empty session + stderr diagnostic; the file is not touched by loading and is
  replaced only by the next *successful save of new state* (a win or New Game Yes).
* New Game Yes writes the **full zero image**, matching the original reset 0x45F883 +
  write 0x45F94D (which zero the flag and every A/B/C). The task text also says
  "preserving unmapped-record data per the codec policy"; these conflict for New Game.
  I followed the evidenced reset and the acceptance wording "writes the zero image".
  Between load and the next save/New Game, unmapped records are preserved verbatim.
* Save failure after a win: `completeLevel` still succeeds; the results screen and
  in-memory progress stay; `saveError()` is set and stderr reports it. The next save
  retries with the full in-memory image.
* Tests that do not opt in never touch disk: the store defaults to null in
  `FrontendSession::create` and `ApplicationRunOptions`; only `main()` selects the default
  store. (Literal "runApplication selects the default store" is realised as main()
  passing `useDefaultSaveStore`, so CLI startup persists while test callers of
  `runApplication` do not.)
* Because New Game Yes writes the zero image, the ordinary "Return twice" route resets
  progress on every start; **Play Current Game** continues. This follows 0x49BCF4/0x49BCFE.
* A path under the `--game-dir` tree is refused for both read and write (component-wise,
  case-insensitive on Windows, via `weakly_canonical`).
* Windows adapter uses `GetEnvironmentVariableW(LOCALAPPDATA)` (no extra link libraries).
* Read-only-directory failure is not testable portably on Windows (the attribute does not
  block file creation); the tests instead block the temp path with a directory, make the
  parent a regular file, make the destination a directory, and inject a failing sink.

## Attempt 2: verification result and repair

Source: `build/agent-runs/20261030T225320-6120-e235a6c4/verification.log` and
`build/agent-native/main/Testing/Temporary/LastTest.log` (supervisor-run, attempt 1).

Executed by the supervisor on attempt-1 sources: the build succeeded; 71 of 72 tests
passed, including the new `savedgame` (52 checks, 0 failures) and `savepersist` (35
checks, 0 failures). `test_savedgame` printed no "SKIPPED" line, so the exe SHA-256
check and the 102-entry comparison at 0xB3948 actually ran and found **0 differences**:
U3 is resolved, `MapScene::kMapping` equals the reference table, no table was edited.
The installed-save hash check in that test also passed.

The single failure was `levelreturn` (CTest timeout 240.03 s). Diagnosis (source/log
inference, not an observed save defect):

* Earlier verifier runs on **pre-save** sources already sit at the limit: passes of
  190-239 s, and timeouts at 240.02 s in runs 20261030T202350 and 20261030T211909.
* In the failed run the captured stderr shows six "entering MM_CLIMB_002" lines, the last
  inside the loss-only route's retry; the restart block (cheap, no level play) had not
  started. The whole suite also took 400 s, i.e. the machine was slow.
* The save work added to that test is a few small file writes and one startup load.

Repairs in attempt 2 (no assertion removed or weakened):

1. `CMakeLists.txt`: the `levelreturn` guard TIMEOUT 240 -> 360 s, equal to the
   verifier's own ctest ceiling. Success is still decided only by the observers.
2. `src/app/AppEntry.h`: `saveStore = nullptr` default member initialiser, which removes
   the five new `-Wmissing-field-initializers` warnings in `test_levelreturn`,
   `test_entrycuewindow` and `test_levelmenuwindow` (designated initialisers that omit
   `saveStore`).

## Final verification of attempt 2 (supervisor run `20261030T230806-6120-39ea8650`)

Source: `build/agent-runs/20261030T230806-6120-39ea8650/verification.log` and
`build/agent-native/main/Testing/Temporary/LastTest.log`.

* Reference SHA-256 `32675bdb...3737a2f` verified by reading bytes only.
* main: **72 of 72 passed** (70 existing + `savedgame` + `savepersist`). `levelreturn`
  passed in 237.47 s under the raised 360 s guard (assertions and observers unchanged; the
  repair of attempt 2 therefore worked and the restart block at the end of `test_levelreturn`
  ran inside that pass).
* `savedgame`: 52 checks, 0 failures, no SKIPPED line, so the exe hash and the 102 entries at
  file offset 0xB3948 were compared and equal `MapScene::kMapping` (U3 resolved, 0 differences).
* `savepersist`: 35 checks, 0 failures (its corrupt-file section printed the visible
  `save: SavedGame.dat is truncated: 1503 bytes ...` diagnostic).
* recovery: **84 of 84 passed** with an empty `LEMMINGS_REFERENCE_EXE`; `native_int_array`
  passed as the sole native_-prefixed test; `git diff --check` ran.

## Attempt 3: frozen-before / current-after demonstration

Reviewer finding: the frozen/current demonstration had not been run and the reports were
stale. The worker still has no shell, so the demonstration is delivered as two tests that the
supervisor's verifier builds and runs:

| Item | Location |
|---|---|
| Frozen pre-change `FrontendSession.{h,cpp}` | ignored `build/save-codec-001/frozen/app/` |
| Driver (one source, two builds) | `src/__test__/test_savedemo.cpp` |
| Registration | `CMakeLists.txt`: `savedemo_after` always; `savedemo_before` only if the frozen files exist |
| Report files written by the driver | `build/save-codec-001/demo-before.txt`, `demo-after.txt` |

The frozen copies are the current sources with only the save-codec-001 additions removed
(store parameter, load in `create`, `persist()`, the two save call sites, new members). They
were **reconstructed by hand from the working-tree-vs-HEAD diff** captured in
`before-diff.txt`, because the worker cannot run `git show`; they are therefore not a git
checkout, and other uncommitted frontend work present before the task (menu click boxes) is
kept in them. `main.cpp` is not frozen: the driver exercises the session-level contract that
`runApplication` uses (`create` -> `completeLevel`); the pre-change `main.cpp` differs only in
`FrontendSession::create(*library)` without a store and a `main()` without options.

Scenario (both builds, temp directory, never LocalAppData or the installed save): New Game
Yes -> select node 0 (MM_CLIMB_002) -> **source-test injected** winning `LevelOutcome` (not
natural gameplay) -> `completeLevel` -> discard the session -> restart with a fresh session ->
Play Current Game. Each build passes only if it observes what is expected of it:

* before: no `SavedGame.dat` exists, and the restarted clone shows `completionFlags[0] == 0`
  and map states different from run 1 (progress lost);
* after: a 1504-byte file exists, it differs from the zero image only at offset 0x00C, and the
  restart's `completionFlags`, `mapStates` and map image equal the in-memory result of run 1.

The bounded `runApplication` restart integration under SDL dummy drivers remains the
`test_levelreturn` restart block (observer-driven quit; the CTest timeout is a failure guard).

### Demonstration results

Executed by the supervisor in `build/agent-runs/20261030T231837-6120-8b71fdf9/verification.log`.
The driver and both frozen/current builds compiled successfully. CTest reports
`savedemo_after` passed (0.46 s) and `savedemo_before` passed (0.39 s).
The frozen source reconstruction and injected outcome limitations above still apply.

Before (retained driver output):

```text
ok   run 1: New Game Yes reaches the map
ok   run 1: SOURCE-TEST injected win accepted by completeLevel (node 0)
ok   run 1: in-memory completionFlags[0] == 1
ok   run 1: Continue returns to the map
file C:\progs\msys2\tmp\webolution-savedemo-before\LemmingsWebolution\SavedGame.dat exists=0 size=0 differing_offsets=[ ]
ok   run 2 (restart): Play Current Game reaches the map
restart: completionFlags[0]=0 same_flags=0 same_mapStates=0 same_map_image=0
ok   BEFORE: no SavedGame.dat exists after completing a level
ok   BEFORE: the restarted clone shows only the initial map (progress lost)
SaveDemo[before]: 0 failures
```

After (retained driver output):

```text
ok   run 1: New Game Yes reaches the map
ok   run 1: SOURCE-TEST injected win accepted by completeLevel (node 0)
ok   run 1: in-memory completionFlags[0] == 1
ok   run 1: Continue returns to the map
file C:\progs\msys2\tmp\webolution-savedemo-after\LemmingsWebolution\SavedGame.dat exists=1 size=1504 differing_offsets=[ 0xc ]
ok   run 2 (restart): Play Current Game reaches the map
restart: completionFlags[0]=1 same_flags=1 same_mapStates=1 same_map_image=1
ok   AFTER: a 1504-byte SavedGame.dat exists
ok   AFTER: file differs from the zero image only at offset 0x00C (C of record 0)
ok   AFTER: restart shows node 0 complete, identical to the in-memory result of run 1
SaveDemo[after]: 0 failures
```

### Final verification of attempt 3

Main: **74/74 passed** (70 existing + savedgame, savepersist, savedemo_before,
savedemo_after), 396.41 s total. Recovery: **84/84 passed**, 29.60 s total.
`levelreturn` passed in 236.99 s under the 360 s guard introduced in attempt 2;
the increase from the original 240 s guard is a recorded acceptance discrepancy,
not an unchanged timeout. Its assertions and observer-driven exit remain.
Reference identity was verified by reading bytes only. The retained run result
records verifier exit code 0. This documentation correction did not change code.

## Remaining verification limits

No true read-only-directory write failure is tested (blocked-path and injected-sink
failures are). Original retail-save compatibility with a nonzero save remains
unverified: the installed sample is all zero. The frozen FrontendSession sources
were hand-reconstructed, and main.cpp is not frozen. The driver uses an injected
LevelOutcome rather than naturally completed gameplay.

## Unchanged / preserved

docs/STATUS.md gate rows, recovery PROGRESS, native-006 deferral, oracle-001, the
372 / 6,186 baseline, historical failures, D1/D3/digger parks, `src/shell/Progress.*`
(unwired), the installed save and executable. The existing main 70 / recovery 84
registrations are kept; four new main registrations were added (savedgame, savepersist,
savedemo_before, savedemo_after). test_levelreturn retains its assertions and gains a
restart block; its timeout changed from 240 to 360 s as documented above.

## Claims not made

Byte-layout tests prove source consistency with the recovered 0x45F43C/0x45F94D layout only.
The installed sample is all zero, so it confirms size and the zero state. The meaning of the
flag, A and B is unknown and they are preserved verbatim. The original's truncated-read,
write-failure and flag-default (`!= 0` normalisation) behaviour is not copied. **No audio,
timing, full-frame or retail-save-compatibility claim is made**, and nothing here changes
level timing or rendering. No original executable, copied instructions, oracle, hook,
debugger, capture or launcher ran, and the installation files are untouched (the installed
`SavedGame.dat` hash is checked unchanged by `test_savedgame`).

## Unknowns and next step

Meaning of the flag, A and B; mode-3 fopen string (U1); original write-failure behaviour
(U7); language/volume persistence; Emscripten store. Original-format compatibility with a
nonzero retail save is unverified (no sample). Next step: independent review of the corrected reports and retained
verification. The restart path is demonstrated in source-built tests; natural gameplay,
nonzero retail-save compatibility and broader fidelity are not established.
