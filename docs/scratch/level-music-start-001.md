# level-music-start-001

**needs_review.** Playtest 28 T1 reported immediate music on **Try again** in
the clone and about 3.3 s from that click to music in the original, followed by
the first lemmings at about 4.3 s. The checks use **Just Climb**, node 0
(`MM_CLIMB_002`). The early call was in `runApplicationLeg`, before the level
frame loop. [Playtest 28](../playtest/2026-11-22-playtest-28.md)

## Static evidence

Ghidra 12.1.4 via static REA on the registered x86 executable, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The [evidence packet](../../build/level-music-start-001/static/) contains raw
instruction reads, pseudocode and xrefs. REA closed without a snapshot.

- Loader `0x4432C1` passes mode 1 at `0x444392` and calls `0x498DE8` at
  `0x444399`. At `0x498E2F` it calls `0x49905A`, which calls `0x498D9A` at
  `0x499074` when `+0x1F1` is set. `0x498DC8` calls DirectSound `Stop`
  (vtable `+0x48`). `0x49909E` releases the old buffer (vtable `+0x08`) if
  `+0x1F0` is set, then clears that flag. The loader fills the new buffer and
  sets `+0x1F0` at `0x49904D`, leaving it stopped. The ordinary entry interval
  after this load is music-free. [Loader call site](../../build/music-001/resume-20261117/static-crosscheck.txt)
- `0x45C9AB` seeds the title clock at `0x45CB21..0x45CB39` with `2.0 + 1.0 + 0.5`
  and subtracts `2.0` without the intro at `0x45CBA6`. `0x45CC09` checks the
  active byte `+0x26024` and playing UI state `+0x572C == 0` at
  `0x45CE7C..0x45CE93`. It subtracts frame time `[+0x1C968]` from `[+0x25FF8]` at
  `0x45CEA1..0x45CEB3`. Below `-1.0`, it clears the active byte at `0x45CEDA`
  and calls `0x498D04` at `0x45CEE6`. That is about **2.5 s of playing time on
  Retry** and **4.5 s after map entry**, measured from the loaded level clock.
- `0x498D04` checks loaded/stopped flags, sets the playback position to zero at
  `0x498D43` (vtable `+0x3C`), starts looping at `0x498D61` (vtable `+0x30`,
  flags 1), sets `+0x1F1`, and applies volume through `0x498993` at `0x498D91`.
  Three other call sites use the same start routine. `0x450B26` is registered
  at `0x4507C4` and calls the reset/stop path `0x450ADA` before `0x450B3C`
  starts the loaded buffer. `0x45CFF9` resumes UI state 0 on the `+0x55A1`
  transition at `0x45D7F4..0x45D864`, then calls it at `0x45D864`.
  `0x486BBA` calls it at `0x486BC2` after frontend mode 0 loads at `0x486C5A`
  and invokes the wrapper at `0x486C5F`. The first callback's external trigger
  and the audible effect of exceptional original resume/reset paths are unverified.

## Clone change and checks

`runApplicationLeg` now calls `CuePlayback::prepareLevelMusic` before the frame
loop. It stops the previous music voice, preserves SFX tails, and caches the
usual random `lemming02..13` selection. The `EntryStartCue` active-to-expired
transition calls `playLevelMusic` once, using the existing pause, menu and
focus rules. Frontend and results continue through `playFrontendMusic` with
`lemming01`. The mixer and the connected hatch-release gate use their existing
paths. Direct `--sim` provisionally uses the short cue clock for audio in its
diagnostic view. [Application call path](../../src/app/main.cpp) ·
[Audio routing](../../src/audio/CuePlayback.cpp)

The clone probe reused existing entry and Retry input sequences with dummy SDL
audio, a private save location, and scratch-only mixer logging. The times below
are frame-clock samples of a loaded mixer voice starting. Physical audibility
and original click-to-music latency require human observation.

| Route | Observed timing | Log |
| --- | --- | --- |
| Map to Just Climb and `MM_BOMB` | Music starts once at 4.5000 s active, 4.9000 s elapsed. The four paused frames hold the clock at 2.6000 and have no music. | [map.log](../../build/level-music-start-001/map.log) |
| Just Climb loss results → Try again | At 2.5000 s the cue is active at `-1.0000` and music is silent. At 2.5625 s the cue retires and the loaded track starts. Results music resumes at the next results screen. | [results-retry.log](../../build/level-music-start-001/results-retry.log) |
| Just Climb in-level menu → Retry | The open menu freezes the map-entry clock. After Retry, music starts at 2.5000 s and the first lemming appears at 4.2 s in the existing 100 ms route. | [menu-retry.log](../../build/level-music-start-001/menu-retry.log) · [release samples](../../build/level-music-start-001/menu-retry-frames/summary.txt) |
| Direct Just Climb with `--sim` | Music starts once at 2.5000 s. | [direct.log](../../build/level-music-start-001/direct.log) |

Registered UCRT64 tools built `smoke_tests`. Seven smoke-labelled tests covering
cue timing, audio routing, frontend, level presentation and results/Retry passed.
The edited legacy `music` test compiled and is deferred to milestone sign-off.
All **29** promoted replays returned `PASS won=1 completed=1`, including
`M02_MG_1` attempt 36 at step 28031. The replay ledger has each exact input,
command, result and log. [Smoke build](../../build/level-music-start-001/build-smoke-tests.log) ·
[Focused smoke](../../build/level-music-start-001/focused-smoke.log) ·
[Legacy music build](../../build/level-music-start-001/build-music-test.log) ·
[Replay results](../../build/level-music-start-001/replays.json)

Changed `src/app/main.cpp`, `src/audio/CuePlayback.cpp`, `src/audio/CuePlayback.h`,
`src/__test__/test_music.cpp` and this report. The legacy test now checks music
silence during preparation, SFX continuity and one start per prepared track.

Handoff — Level music starts at the title cue's expiry on map entry and Retry.  
Next — Supervisor smoke profile pending, followed by independent review and human playtest.

**What to check** — In **Just Climb**, lose and choose **Try again**. Pause
while **Let's Go!** is visible and resume. The level music should wait until
the cue clears after about 2.5 s of unpaused play. Lemmings should appear at
their usual time. Enter from the map and expect about 4.5 s of unpaused play
before music starts.
