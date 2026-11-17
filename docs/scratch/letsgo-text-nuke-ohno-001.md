# letsgo-text-nuke-ohno-001

Status: **needs_review for the implemented patch**; audible-explosion requirement
**blocked on approval**, supervisor smoke pending. Playable v1,
[playtest 10 Z3/Z4](../playtest/2026-11-16-playtest-10.md), preserving
[playtest 9 Y13](../playtest/2026-11-16-playtest-9.md).
Difficulty routine; recommendation Sonnet; assigned Codex override used.
Routes: **MM_CLIMB_002 / Just Climb**, **MM_BOMB**, entry/Retry/Nuke.
Before: the human preferred the previous text and reported unwanted Nuke Ohno.
[Detailed evidence and preservation record](../../build/letsgo-text-nuke-ohno-001/details.md).

## Production path

`runApplicationLeg -> EntryStartCue(introEnabled) -> advanceBeforeFrame -> frame
-> drawEntryStartCue` restores **9a39257**: 3.5 s entry/1.5 s Retry seed, visible
at <=1.5 s, quadratic shrink to base at 0.5 s, additive below zero, retirement
strictly below -1 s. EntryStartCue, AppEntry/LevelStartSequence headers and both
`test_levelreturn.cpp` Retry expectations match that commit. The former
`entryDelay` was the identical timer; substituting `entryCue.active()` in the
existing release predicate changes no release or simulation behavior.

Voice stays independent: `RuntimeSimState::advanceElapsed -> afterUpdate`
consumes `firstReleasePending` on `world.spawned()>0`, then `CuePlayback -> Mixer`
requests Letsgo once per attempt. Later hatches/pause/speed do not retrigger it.
The restored existing hatch tests sampled unpaused time in 0.1 s steps:

| Route | Text visible / base / retired | First release/voice request | Voice after retirement |
| --- | --- | --- | --- |
| MM_CLIMB_002 and MM_BOMB entry | 2.0 / 3.0 / 4.5 s | 5.9 s | 1.4 s |
| MM_CLIMB_002 Retry | first frame / 1.0 / 2.5 s | 4.2 s | 1.7 s |

Thus entry voice follows first text by 3.9 s, base size by 2.9 s. These are clone
observations plus a source-traced audio call, not listening measurements. Offsets
vary with frame quantization, pause, door duration and speed. The existing fast
route switches speed off before release and also measures 5.9 s.

`RuntimeNukeInput -> World::nuke -> stepNuke/step -> World::cue -> takeCues ->
CuePlayback::drain` now drops **OhNo only** while `nukeRequested_` is set, including
an already lit ordinary fuse. Countdown/stagger, states, animation, crater,
deaths and Explode event emission stay unchanged. Ordinary Bomber still says Ohno.

## Static authority and outstanding approval

Registered reference SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**,
REA/Ghidra **12.1.4**; complete returned packets/IDs/limitations in
[static/](../../build/letsgo-text-nuke-ohno-001/static/), indexed in the detailed
record. Pseudocode was checked against instructions; closed without snapshots.

- `ohno-inst.json`: `414420` reads guard, `414428` skips playback when nonzero;
  `414443` plays handle `4E32CC` otherwise. State installation still runs.
- `final-static-2.json`: Nuke helper `444A98` writes receiver+`5138` at `444AB1`.
  **Level base `4C6A08 + 5138 = 4CBB40`: the guard is the Nuke state.**
- `nuke-evidence-1.json`: consumer `442646` tests that field; its direct flapper
  path `44292B/442962` bypasses the ordinary Ohno helper. `dispatch-inst.json`
  shows replay event 7 also writes the absolute flag at `457017`.
- Correction to the retained round-7 notes: their zero readings were ordinary
  bomb observations. Their claim that nothing writes the byte missed the
  receiver-relative Nuke store and replay writer; it did not establish Nuke audio.

**Audible explosions are not established:** existing `CuePlayback.cpp` leaves
Explode unrouted. Preserving its events preserves that silence. Protected smoke
`test_cueplayback.cpp` asserts seven routes, unrouted Explode and no blast sound.
New `explosion-1.json` proves original blast enter `4081FC` loads `4E32C0` at
`408205` and plays at `40821C` without the Nuke guard; retained registration
`45E570..45E582` stores it at level+`1C8B8`. The user was asked for the narrow
exception to update those obsolete smoke assertions while adding the route.
[Concrete proposed patch](../../build/letsgo-text-nuke-ohno-001/proposed-explode-routing.patch)
is **not applied**. This acceptance item remains unmet. Only the two explicitly
assigned Retry assertions were changed in smoke; no other smoke assertion changed.

## Checks and handoff

Registered GCC/CMake/Ninja, `-O2 -g`, assertions, eight build jobs:
[final build exit 0](../../build/letsgo-text-nuke-ohno-001/final-build.json).
[Focused 4/4 PASS](../../build/letsgo-text-nuke-ohno-001/focused.log), 40.11 s:
levelreturn (39.86 s), entrystartcue, cueplayback, gameplay_sounds. Added a
seconds-long legacy unit for unprimed/prelit Nuke silence with explosion events
and deaths preserved. Existing smoke checks ordinary Bomber's Ohno.
Restored all five ui-sounds-retired legacy registrations without source/assertion/
timeout edits: entrycuewindow, levelmenuwindow, hatch_timing_after/fast/retry;
[5/5 PASS](../../build/letsgo-text-nuke-ohno-001/restored.log), 43.36 s.
Inventory: **95 smoke / 237 total**. No full suite duplicated; supervisor gate pending.

All **17 active promoted replays PASS**, exit 0, won=1, completed=1:
[each exact file/command/result](../../build/letsgo-text-nuke-ohno-001/replays.json).
Saved/needed: M06_KF_1 93/50; MM_BASH_002 10/10; MM_BASH_MINE_WLR1 20/10;
MM_BLOCK_001_A 24/15; MM_BLOCK_BOMB_A 12/10; MM_BOMB 17/10; MM_BUILD 20/10;
MM_CLIMB_002 10/1; MM_DIG_000 19/10; MM_FLOAT 10/1; MM_MINE 20/10; MM_W_012 16/10;
MM_E1_3_SKILLS 10/5; MM_E2_4SKILLS 12/10; MM_BASH_DIG_001_A 20/10;
MM_BUILD_BLOCK_A 15/10; MM_FLOAT_CLIMB2 10/6. Retired recordings left alone.

Changed: `main.cpp`, `EntryStartCue.{h,cpp}`, `AppEntry.h`, `LevelStartSequence.h`,
`World.{h,cpp}`, `CuePlayback.cpp`, `test_levelreturn.cpp`, legacy
`test_gameplay_sounds.cpp`, `CMakeLists.txt`, this report. Backups/git-show and
observations are under the task build directory. Get-Process checked live runners;
supervisor PID 32152 owns RepositoryLease. No protected controls edited; incoming
hooks-log/queue/status edits preserved. Private LocalAppData:
`build/letsgo-text-nuke-ohno-001/appdata`; installed/normal saves unused.
LF and scoped git diff --check pass. No original execution, debugger/hooks/capture,
installs, agents, new scripted routes, commits or pushes. Human listening and
playability remain unproved; original fidelity/timing parks and gates unchanged.

Handoff: text/Retry restored; Nuke Ohno suppression statically established; nine checks and 17 replay wins pass.
Handoff: needs_review for implemented work; supervisor smoke/human playtest next; audible Explode routing awaits the protected-assertion exception.

What to check: **MM_CLIMB_002 / Just Climb**, enter/Retry: early oversized Let's Go
shrinks and retires, followed later by one Letsgo voice at first release;
pause/resume and later hatches do not restart it. **MM_BOMB**, assign one Bomber:
Ohno at fuse end. Retry and Nuke several lemmings, including a primed Bomber:
no Ohno, with explosions/craters still occurring. Audible blast playback remains
the approval item above.
