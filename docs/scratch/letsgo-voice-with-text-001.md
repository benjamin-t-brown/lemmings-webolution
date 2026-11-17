# letsgo-voice-with-text-001

Status: **needs_review; supervisor smoke pending**. Playable v1, human
[playtest 11 A12](../playtest/2026-11-16-playtest-11.md), superseding Y13.
Difficulty routine; recommendation Sonnet; assigned Codex override used.
Routes: **MM_CLIMB_002 / Just Climb**, entry/Retry; **M06_KF_1**, later hatches.
Before evidence: the human heard Letsgo at spawn after the restored entry text.
The [accepted preceding report](letsgo-text-nuke-ohno-001.md) traces that mismatch
to `afterUpdate -> firstReleasePending -> world.spawned()>0 -> LevelStart`.
Moving this audio request is the next dependency; the text is already accepted.

`runApplication -> runApplicationLeg -> materialize -> cueFrame ->
drawEntryStartCue` now consumes `entryVoicePending` and immediately requests
`CuePlayback::play(LevelStart) -> Letsgo -> Mixer::play` in the same draw branch.
The local latch resets on every new leg, including Retry, and is consumed even
when audio is disabled/unavailable. Repeated drawing, pause/menu transitions,
fast-forward passes and later hatch releases cannot request it again. The old
first-release request is removed. Frame-view audio counters include this draw's
request. Help and cue-route provenance describe the new trigger.

The existing `EntryStartCue` construction, countdown, rectangle, font, compositor
call and draw order are unchanged. `EntryStartCue.{h,cpp}` and
`test_levelreturn.cpp` have no diff against **d84e1f8**; no test file changed.
No simulation, release predicate, entry clock or mixer policy changed. Replay
returns through `replayLevel` before constructing playback/window/save state;
`--no-audio` still disables routing, with no deferred request. Standalone level
diagnostics without entry text now make no Letsgo request. Explicit test-only
frame skipping defers the request until the text is actually composed.

Retained static authority: [gameplay sounds](gameplay-sounds-001.md) and
[start-instructions.json](../../build/gameplay-sounds-001/static/start-instructions.json),
evidence `ev_bc03b8770e90cb03915c79e9d4f3273b7a949798e2b57d94d1e37c4b7b7a8ada`.
Registered EXE SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Retained Ghidra 12.1.4 instructions read this task: `425AB8` tests hatch+AE,
`425AC4` reads global `4E32A0`, `425AD0` loads Letsgo handle `4E32B4`,
`425ADB` calls playback, and `425AE3` sets the latch before opening/release
checks at `425AF7` onward. This supports pre-release, latched playback; it does
**not** establish original text/audio alignment. Synchronizing to the clone's
draw is the user-requested, **provisional** presentation choice. No new REA
session or original execution occurred; physical output latency and human
listening remain unverified. Original timing/fidelity gates stay unchanged.

Registered UCRT64 GCC/CMake/Ninja build, `-O2 -g`, assertions, eight jobs:
[build.log](../../build/letsgo-voice-with-text-001/build.log).
[Focused checks](../../build/letsgo-voice-with-text-001/focused.log): **3/3 PASS**,
41.62 s: entrystartcue 0.01 s, unchanged levelreturn 41.41 s, cueplayback 0.19 s.
[JUnit](../../build/letsgo-voice-with-text-001/focused.xml).
These check existing text/Retry and audio routing, not audible synchronization.
No new tests, replay fixtures or scripted routes; mandatory smoke is left to
the supervisor after worker exit.

All **17 active promoted recordings PASS**, exit 0, won=1, completed=1, using
`webolution --game-dir "Lemmings Revolution" --replay <file>` sequentially.
[Every exact file, command, result and individual log](../../build/letsgo-voice-with-text-001/replays.json):

| Level | Result; saved/needed |
| --- | --- |
| MM_BASH_002 | PASS; 10/10 |
| MM_BASH_MINE_WLR1 | PASS; 20/10 |
| MM_BLOCK_001_A | PASS; 24/15 |
| MM_BLOCK_BOMB_A | PASS; 12/10 |
| MM_BOMB | PASS; 17/10 |
| MM_BUILD | PASS; 20/10 |
| MM_CLIMB_002 | PASS; 10/1 |
| MM_DIG_000 | PASS; 19/10 |
| MM_FLOAT | PASS; 10/1 |
| MM_MINE | PASS; 20/10 |
| MM_W_012 | PASS; 16/10 |
| MM_E1_3_SKILLS | PASS; 10/5 |
| M06_KF_1 (playtest 11) | PASS; 91/50 |
| MM_E1_4SKILLS | PASS; 20/10 |
| MM_E2_4SKILLS (playtest 11) | PASS; 14/10 |
| MM_BASH_DIG_001_A | PASS; 20/10 |
| MM_BUILD_BLOCK_A | PASS; 15/10 |

No recording changed or diverged. These silent headless runs verify simulation
regression only, not text/audio synchronization or human playability.

Changed: `src/app/main.cpp`, `src/audio/CuePlayback.cpp`, this report. Incoming
hooks-log/queue/status edits preserved; worker edited no protected controls.
Get-Process checked live runners (CIM metadata access denied); supervisor PID
15712 owns RepositoryLease. Backups, HEAD main and entry status are retained in
[the task directory](../../build/letsgo-voice-with-text-001/). All child runs used
private `build/letsgo-voice-with-text-001/appdata`; levelreturn injects its own
test save, and replay opens no save. Installed/normal saves were not written.
No original/oracle, debugger, hooks, capture, installs, subagents, concurrent
writers, commits or pushes. Changed files use LF; scoped diff check passes.

Handoff: voice follows the first text draw once per attempt; three focused checks and all 17 active replay wins pass.
Handoff: needs_review; supervisor smoke and independent review next, then human listening/playtest confirmation.

What to check: **MM_CLIMB_002 / Just Climb**, enter and Retry: hear Letsgo as the
existing oversized Let's Go text first appears, once per attempt. Pause/resume
and toggle fast-forward before/during/after the text; no repeat and no voice at
first release. **M06_KF_1**, continue through later hatch releases: no repeat.
With `--no-audio`, entry and Retry remain silent with the same text.
