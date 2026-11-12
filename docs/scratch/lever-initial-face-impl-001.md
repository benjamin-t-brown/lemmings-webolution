# lever-initial-face-impl-001: apply the serialized lever starting face (state + load fan-out)

Status: **implemented; supervisor verification pending at worker exit (section 6 lists exactly what this worker did and did not observe).**
Source-built only. The original executable, copied original bytes, debugger, hooks and capture were never used. No static REA session was opened: the accepted lever-links / lever-init / lever-timing / lever-gates reports already carry the addresses and the `0x00444417 -> 0x0043703F -> 0x00436E6A` initialization chain, and no new binary question arose.

Reference identity (`agent/tools.windows.json`): `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

Labels: STATIC = accepted trace; SOURCE = clone source; SERIALIZED = read straight from the level bytes by `lemmings::read_compiled_level`, independent of the production pairing/state code; REAL = installed MM_W_012 through the public loader, no injected actors; SYNTHETIC = a walker placed in a real lever's box (fixture, not natural play); FIXTURE = installed level bytes with one stored-face word rewritten and served to the unchanged loader through a temporary `patch.box`, or a hand-built `LevelSetup`.

## 1. Defect and contract

STATIC (lever-links section 1, lever-timing section 3): `0x00444417` calls vt+0x28 = `0x0043703F` = `0x00436E6A(this, [+0xDC])` per tagged object. `0x00436E6A(face)`: face +1 -> `[+0xB8] = 1`, send list `+0xCC`; face -1 -> `[+0xB8] = -1`, send list `+0xBC`; any other value does nothing. The lever accepts a pulling facing `-state` (`0x00436D30`). So the stored face is the **state** and the **list that is sent**, in one step, at load.

SOURCE before the change (lever-gates-impl-001 section 5, exposed difference): `fanOutLeverLoadFaces` sent the stored face's list but did not set `Lever::state`, which stayed at the constructed `+1` for every lever.

## 2. What changed

| File | Change |
| --- | --- |
| `src/sim/World.cpp` | `fanOutLeverLoadFaces` now calls `activateLeverFace(setup, lever, lever.initialFace)` for each **linked** lever: `state := face` and the matching list is dispatched, in one place, once. `activateLeverFace` already ignores a face that is not +-1 (state kept, nothing sent) and touches neither the cooldown (the load path is `0x00436E6A`, not the cooldown-reloading `0x00436E14`) nor the cue. Unlinked (legacy/synthetic) levers are skipped exactly as before. |
| `src/sim/World.h`, `src/sim/RetailLevel.cpp` | comments only (the loader still calls `fanOutLeverLoadFaces` once, after every lever record and receiver exists). |
| `CMakeLists.txt` | registers `leverinitface` (`test_leverinitface.cpp`). The pre-edit driver `leverinitface_baseline` was registered for its genuine pre-edit run and then unregistered (a comment explains; registering it would overwrite the retained evidence). |

No hardcoded level or coordinate exists in production. Pairing, id lookup, cooldown loading, receiver initialization and the 528-cell pass are untouched. Production call path: `loadRetailLevel(Report)` -> `linkLevelGates` (receivers, load pass, pairing) -> `fanOutLeverLoadFaces` -> `activateLeverFace` -> `dispatchLeverMessages`; runtime: `RuntimeSimState::advanceElapsed -> World::advanceElapsedMs -> updateSegment -> stepLeverContacts -> activateLeverFace`.

## 3. Genuine baseline (before any production edit)

Pre-edit production sources and the test about to be edited are preserved in `build/lever-initial-face-impl-001/pre-edit/`. The observation-only driver `src/__test__/test_leverinitface_baseline.cpp` (public loader + `advanceElapsedMs` only) was registered as `leverinitface_baseline` (ctest #4) and run by `python tools/agent_control/verify.py --profile main` against the unedited tree; it **Passed** (0.44 s) in the worker run whose output is `C:\progs\msys2\tmp\claude\C--progs-lemmings-webolution\eb5ab95b-64cf-4901-9d28-b979371cfb99\tasks\bdu90vwzv.output`. Retained: `build/lever-initial-face-impl-001/baseline-before-edit.txt`:

| Lever (id) | SERIALIZED stored face | REAL loaded state (before) | Walker facing accepted before | Correct (stored face, accepted facing = -face) | First accepted pull sends |
| --- | --- | --- | --- | --- | --- |
| (79,60) id 0 | **-1** | 1 (**wrong**) | -1 (wrong; sent the empty -1 list, `delivered=0`) | state -1, facing +1 | +1 list: ids 8, 7 type 4 |
| (72,34) id 1 | +1 | 1 | -1 | same | -1 list: ids 9, 18-22 type 4 |
| (183,60) id 2 | +1 | 1 | -1 (ids 5, 6 raised, `delivered=2`) | same | -1 list: ids 5, 6 type 4 |
| (189,35) id 3 | **-1** | 1 (**wrong**) | -1 (wrong; `delivered=0`) | state -1, facing +1 | +1 list: ids 4, 17, 16, 15, 14, 13 type 4 |

All four stored-face lists are empty (nothing sent at load), `zoneWaterCells=528` at load. Both -1 levers therefore ignored the correct pulling direction and fired on the wrong one; a correct +1 pull on them was rejected and would have left gates 7/8 and 4/13-17 drained.

## 4. After evidence

Registered test `leverinitface` (`src/__test__/test_leverinitface.cpp`) writes `build/lever-initial-face-impl-001/after-observations.txt` and temporary fixture archives under `build/lever-initial-face-impl-001/fixture/`. Expectations derive from SERIALIZED records and the contract above (accepted facing `= -stored face`), never from the new output.

| Case | Label | Asserted |
| --- | --- | --- |
| stored faces | SERIALIZED | four type-1 records; cross-check (79,60) -1, (189,35) -1, (183,60) +1, (72,34) +1; every face is +-1; every stored-face list empty |
| load | REAL | each lever loads `state == initialFace == stored face` (all four, both former -1 levers); `cooldownRemaining` 0; 0 messages at load; 528 cells; receivers keep stored request / done 2 / armed false |
| first pull | SYNTHETIC | per lever, one update: facing `-face` accepted (state := that facing, `delivered ==` size of that facing's list, every target gate `request 0, done 2, h == 0.016*2.0` in the **same update**); facing `face` rejected (state unchanged, `delivered 0`, targets untouched) |
| repeat | SYNTHETIC | two walkers alternate pulls over four updates: states `-f, f, -f, f`; delivered `n, n+m, 2n+m, 2n+2m`; a re-sent type 4 does not restart the ramp (`h == 4*0.032`) |
| ramp regression | SYNTHETIC | lever (183,60) ids 5/6: 1000 x 16 ms, 384 cells, total 912, `h == 24.5` |
| reload | REAL | after a mutated world, a fresh load has every lever back at its stored face, 0 messages, 528 cells, receivers rebuilt; the earlier world is unaffected |
| unlinked / unsupported | FIXTURE | an unlinked lever with a stray `initialFace` keeps state 1 and sends nothing; a linked lever with face 5 keeps state 1 and sends nothing |
| once-only fan-out | FIXTURE | lever (183,60)'s stored-face word rewritten to -1 (the unique `1, 2, 5, 4` record words located by a byte search) and served through an unchanged-loader `patch.box`: state -1, `delivered == 2` exactly (not 4), ids 5/6 `request 0, done 2, h 0` at load, 528 cells; a facing -1 walker is rejected, a facing +1 walker accepted (empty +1 list, no further message); reload dispatches 2 on the fresh setup. Word rewritten to 5: state 1, 0 delivered, 0 unsupported |

Retained after output of this worker's edited-tree run: `build/lever-initial-face-impl-001/after-observations.txt` (`leverinitface` ctest #4, Passed, 1.06 s; no `FAIL` line). Sample lines (SERIALIZED / REAL / SYNTHETIC / FIXTURE as labelled in the file):

```
REAL load: lever[0] (79,60) stored face=-1 loaded state=-1        (before: state=1)
REAL load: lever[3] (189,35) stored face=-1 loaded state=-1       (before: state=1)
REAL load: lever[2] (183,60) stored face=1 loaded state=1         (unchanged)
SYNTHETIC lever (79,60) stored -1 walker facing 1: state -1 -> 1, delivered=2   gate8/gate7 request=0 done=2 h=0.032
SYNTHETIC lever (79,60) stored -1 walker facing -1: state -1 -> -1, delivered=0 (rejected)
SYNTHETIC lever (189,35) stored -1 walker facing 1: state -1 -> 1, delivered=6   gates 4,17,16,15,14,13 request=0 h=0.032
SYNTHETIC lever (189,35) stored -1 walker facing -1: state -1 -> -1, delivered=0 (rejected)
SYNTHETIC lever (183,60) stored 1 walker facing -1: state 1 -> -1, delivered=2    gate5/gate6 request=0 h=0.032
SYNTHETIC repeat lever (79,60): states 1 -1 1 -1 delivered 2 2 4 4
SYNTHETIC ramp (183,60): h5=24.5 h6=24.5 cells5=192 cells6=192 total=912
REAL reload: all four levers back at their stored faces, 528 cells, receivers request 1 / done 2 / h 0
FIXTURE stored face -1: state=-1 delivered=2 gate5(req=0 done=2 h=0) gate6(req=0 done=2 h=0) cells=528
FIXTURE stored face 5: state=1 delivered=0
```

The edited `sharedtiming_baseline` rewrote `build/shared-timing-impl-001/legacy-wrapper-observations.txt` (a registered per-run output, as before): `lever[0] ... state=-1`, `scenario order ... lever0.state=-1 (stored face -1)`, `scenario reload: before tick=2 lever0.state=1; after reload tick=0 lever0.state=-1`. The historical `baseline-before-edit.txt` there is untouched.

## 5. Changed assertions and why each old expectation was wrong

Only `src/__test__/test_sharedtiming_baseline.cpp` (explicitly authorized). The helper `storedFace()` reads the lever's stored face from the level bytes independently of the loader. Historical artifacts under `build/shared-timing-impl-001/` and the pre-edit copy of this test in `build/lever-initial-face-impl-001/pre-edit/` are unchanged.

| Scenario | Old | New | Why the old one was wrong |
| --- | --- | --- | --- |
| 7 reload (line ~185) | `reloaded...levers[0].state == 1` | `== storedFace(lever 0)` (-1), plus a new check that the pulled world's lever left its stored face; walker faces the accepted direction | lever 0 (79,60) stores -1; "state as loaded" is the stored face, not the constructed +1 |
| 6 two-actor order (line ~173) | Left then Right, final `state == 1` | accepted facing first (Right for a stored -1), other second, final `state == storedFace` (two toggles restore the stored face); new check that the stored face is -1 | with the corrected start -1 the old first actor (Left) is rejected; `== 1` held only by the wrong default coinciding with the other arrangement |

Unchanged on purpose: scenarios 1-5 still place a Left-facing walker on lever 0. Their assertions are tick counts and do not depend on lever acceptance (that walker is now rejected by lever 0; the lever is simply not pulled there). Scenario 8 (lever (183,60), stored +1) is unchanged and still passes by the same contract. No timeout, other assertion or coverage was removed or weakened; assertions were added.

## 6. Verification status (truthful, at worker exit)

* **Supervisor mandatory full main/recovery verification: pending.**
* Worker pre-edit run (unedited production, `verify.py --profile main`, output path in section 3): built (1,376-step graph); tests #1-#52 Passed including `levergates` (#3), `leverinitface_baseline` (#4) and `targetedplay_play_1` (#35, 732 s), before the client's background limit; not a full-suite result.
* Worker edited-tree main run (`verify.py --profile main`, output `C:\progs\msys2\tmp\claude\C--progs-lemmings-webolution\eb5ab95b-64cf-4901-9d28-b979371cfb99\tasks\b15f2umkt.output`): built after an earlier attempt (`...\b0agg1nhz.output`) stopped on a `test_levelreturn.exe` link lock held by the still-running pre-edit run; ctest #1 `sharedtiming_baseline`, #2 `sharedtiming`, #3 `levergates`, #4 `leverinitface` **Passed**, and the remaining tests were still running when this report was written; see the supervisor log for the rest. I do not claim the full main profile passed.
* Worker `verify.py --profile recovery` (run twice, foreground, on the current tree; recovery does not build any file this task touched): **84/84 Passed** both times (`Total Test time` 39.11 s and 37.45 s).
* **Not claimed:** the full main profile passing on the edited tree.
* Tests most exposed and therefore the ones the supervisor log must be read for: `sharedtiming_baseline` (edited), `leverinitface`, `levergates`, `sharedtiming`, `targetedplay_play_1/2` and `playableexpansion_play_*` (real MM_W_012 routes whose lever (79,60)/(189,35) pull direction now follows the stored face), `leverlinks_measure`, `leverinit_measure`, `test_world`, `retaillevel`.

## 7. Limits and unknowns (not repaired)

* Natural play, visible fluid and full timing fidelity are not claimed; every pull uses an injected walker. Tagged-to-placed lever pairing still uses pose; legacy `tick()` does not step receivers; kind 1 has no mask plane; hybrid timing is unchanged.
* The unsupported-face behaviour (state kept at +1, nothing sent) follows `0x00436E6A` ignoring such values; the constructed state of a lever with an unsupported stored face in the original is not separately traced.
* The fixture archives are derived from installed data and live only under ignored `build/`.

## 8. Next decision

Stop after independent review. Scope of visible fluid / downstream consumers is a separate task; no automatic follow-on work.
