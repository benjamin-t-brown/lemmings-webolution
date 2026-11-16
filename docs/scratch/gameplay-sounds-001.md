# gameplay-sounds-001 — needs_review

Playable v1, [playtest 9 Y3/Y4/Y5](../playtest/2026-11-16-playtest-9.md):
missing bashing, level-start and last-three-bricks sounds. Difficulty: routine;
recommended model: Sonnet; assigned Codex override used. **needs_review;
coordinator reconciliation is required for the inherited replay exceptions.**
The first supervisor smoke passed; independent review requested revision only
for the unqualified all-replays criterion. Verification after this report update
is pending supervisor execution. Acceptance is the next human listening playtest.
This is audio integration, not original-fidelity acceptance.

The ledger is the before evidence. Installed confirmation routes are node 0
**MM_CLIMB_002 / Just Climb** (entry), node 6 **MM_BASH_002** (bashing), and node 4
**MM_BUILD** (builder warnings). Before, World emitted none of these skill
events and application entry requested no sound; the adapter routed only Ohno,
FallSplat and Yippee. Adding events and routes is the next dependency; skill,
release, animation and simulation timing are unchanged.

## Production path and provisional choices

`runApplication` owns one `CuePlayback` across frontend/level legs. Its existing
`start -> Mixer::loadFrom -> soundAssetName -> AssetLibrary` patch/base overlay
loads and decodes/resamples all six routes once. `runApplicationLeg` now requests
`LevelStart -> Letsgo` on its first focused, unpaused gameplay frame. A per-leg
latch sits outside fast-forward passes; retries/new level legs get a fresh latch.
Menus and focus suspension defer it, and audio-disabled/unavailable entry cannot
accumulate a later backlog. **Provisional point:** this precedes the clone's
existing title/hatch-opening sequence; exact retail phase and the original global
startup guard are not reconstructed. Direct simulated level entry also uses it;
headless recorded replay remains silent by design.

`World::stepBasher` now emits `Bash` after a successful terrain-clearing bite;
empty grace swings and steel refusal do not emit it. `World::stepBuilder` emits
`BuilderWarning` after the accepted advance, before decrementing counts 3, 2, 1.
`RuntimeSimState::advanceElapsed -> afterUpdate -> drainCues -> CuePlayback::drain
-> World::takeCues -> play -> Mixer::play` handles every event, including each
fast-forward pass. No new clock or skill-state mutation was added.

**Provisional cadence/mixing:** one Mine request per successful hybrid bash bite,
rather than retail cell/subpass callbacks. Mine uses `SkipPlaying`: finish the
current clip, discard overlapping requests. Chink uses `RestartPlaying`: each
brick starts its attack, replacing any still-playing Chink tail. Thus all three
warnings play in full at normal speed; fast-forward/group arrivals can truncate
tails. Both are additive, non-looping one-shots under the existing mixer lock and
voice cap; neither queues requests. Other SFX retain their existing policies.
Pause produces no skill updates/events; an already-playing one-shot may finish.
The observer records routed requests, including ones suppressed by overlap;
observer counts are not audible-output measurements.

## Static evidence and installed samples

Read first: [native-022](native-022-sound-cue-routing.md),
[accepted exit audio](exit-yippee-sound-001.md), [basher](basher-fix-001.md),
[builder round 6](../../old/docs/scratch/phase3-round6.md), and the accepted
[shrug task](builder-shrug-assign-001.md). Their historical unknowns are retained;
the following new targeted reads resolve these three sample/trigger questions.

Only target: `C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
Used reverse-engineer-anything and project static REA, Ghidra **12.1.4**, profile
**3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece**.
Complete returned evidence, IDs and limitations: [static/](../../build/gameplay-sounds-001/static/).
Analysis was closed with empty arguments, without snapshots. No original execution.
Pseudocode is decompiler output; decisive branches/handles/calls were checked
against full function instruction reads:

| Record | Instruction-supported finding |
| --- | --- |
| [registration](../../build/gameplay-sounds-001/static/registration-instructions.json) | `45E528..45E53A`: Letsgo -> receiver+1C8AC; `45E558..45E56A`: Chink -> +1C8B4; `45E660..45E672`: Mine -> +1C8E0. With receiver `4C6A08`, handles are `4E32B4`, `4E32BC`, `4E32E8`. [String bytes](../../build/gameplay-sounds-001/static/cue-strings.json) confirm Mine at `4C0CE8`. |
| [hatch](../../build/gameplay-sounds-001/static/start-instructions.json) | `425AB6..425AE3`: test hatch+AE and global `4E32A0`, play Letsgo once at `425ADB`, then set +AE=1; opening/arming checks follow at `425AF7..425B3B`. This is before release, not the first actor's spawn. |
| [bash](../../build/gameplay-sounds-001/static/bash-sound-instructions.json) | `40C0AF` refuses protected cells using Scrape (`40C139..40C148`). Destructible nonzero cells are cleared; subpasses 0/3 enter the effects arm, load Mine at `40C317`, play at `40C32F` through `44535B`. Bite `40C357` calls this helper at `40C570/40C61A`. |
| [builder](../../build/gameplay-sounds-001/static/builder-instructions.json) | `40FF54` compares pre-decrement count with 4, skips sound if >=4; `40FF5C` loads Chink, `40FF73` plays it, `40FF93` stores count-1. This is the advance/decrement callback, after the separate brick stamp. Clone retains its existing combined brick/advance moment. |

Production decoder results in [targeted-tests-final.log](../../build/gameplay-sounds-001/targeted-tests-final.log):
all three entries resolve to **base lemmings.box**, with no patch override.

| Entry | Bytes | Mono rate | Frames | Duration |
| --- | ---: | ---: | ---: | ---: |
| `SOUNDS\LETSGO.WAV` | 6,390 | 7,092 Hz | 6,346 | 0.894811 s |
| `SOUNDS\MINE.WAV` | 27,436 | 22,050 Hz | 13,696 | 0.621134 s |
| `SOUNDS\CHINK.WAV` | 1,194 | 8,403 Hz | 1,150 | 0.136856 s |

Other evidenced sounds still silent, **not implemented** in this task (bounded
inventory from retained research and this trace, not a complete audio audit):

| Event/sample | Static authority |
| --- | --- |
| Successful Digger / Mine; Digger or Basher refusal / Scrape; Miner refusal / MineMetal | [round 25](../../old/docs/scratch/phase3-round25-scrape.md): `415B84`, `415B33`, `40C139`, `4188F6`. Mine is newly routed only for Bash. |
| Fluid-death step / Bubble; lava death enter / Die | [round 24](../../old/docs/scratch/phase3-round24-bubble.md), [round 28](../../old/docs/scratch/phase3-round28-die.md): `4046DC`, `4042E5`. |
| Lever / Lever; speed-pad activation / SpeedPad | [round 21](../../old/docs/scratch/phase3-round21-lever.md), [round 17](../../old/docs/scratch/phase3-round17-speedpad.md): `436E29`, `4326D1/4326FE`. |
| Object timed state commit / Rumble | [round 26](../../old/docs/scratch/phase3-round26-rumble.md): `423823`, guarded by prior/desired states and timer. |
| Skill-panel selection and SFX-volume feedback / Changeop | [round 27](../../old/docs/scratch/phase3-round27-changeop.md): `45AD11`, `48EB0B`. Selection is distinct from skill assignment. |
| E10_KF_1 pause easter egg / Witch | [round 20](../../old/docs/scratch/phase3-round20-witch.md): `45D20C`. |

Explode remains unrouted without a retained play-site trace in the evidence read
here. Chink's builder site does not justify playing it for every assignment.
Registered-only Balloon/Fire/ForceF/Splat are not established missing events.

## Checks, replay results and preservation

Registered UCRT64 GCC 16.1.0 incremental build passed:
[build-final.log](../../build/gameplay-sounds-001/build-final.log).
Earlier [build.log](../../build/gameplay-sounds-001/build.log) includes existing
RetailLevel.cpp missing-field-initializer warnings; no warning was introduced by
these changed files. Focused CTest **2/2 passed, 0.21 s** in
[targeted-tests-final.log](../../build/gameplay-sounds-001/targeted-tests-final.log).
The new **legacy** unit covers exact last-three event multiplicity, buffering,
repeat drains, pause/fast-forward and successful/empty/steel bash cases. Existing
smoke cueplayback now requires six routes and their literal names/installed
decodes: its old three-route expectation encoded Y3-Y5; assertions were expanded,
not weakened. Smoke membership remains **95**, with unchanged timeouts.

Removed legacy `audioapp` registration/source: its expected silence at level entry
and sole-FallSplat observation conflict with the intended Y4 Letsgo addition.

Ran `build/agent-native/main/webolution.exe --game-dir "Lemmings Revolution"
--replay <file>` on **all 17** promoted human recordings. Exact filenames,
return codes and outputs: [replays-final.json](../../build/gameplay-sounds-001/replays-final.json),
plus individual `final-replay-*.log` files in that directory. Every PASS below
has **won=1, completed=1**:

| Level | Result, saved/needed |
| --- | --- |
| M06_KF_1 | PASS 93/50 |
| MM_BASH_002 | PASS 10/10 |
| MM_BASH_MINE_WLR1 | PASS 20/10 |
| MM_BLOCK_001_A | PASS 24/15 |
| MM_BLOCK_BOMB_A | PASS 12/10 |
| MM_BOMB | PASS 17/10 |
| MM_BUILD | PASS 20/10 |
| MM_CLIMB_002 | PASS 10/1 |
| MM_DIG_000 | PASS 19/10 |
| MM_FLOAT | PASS 10/1 |
| MM_MINE | PASS 20/10 |
| MM_W_012 | PASS 16/10 |
| MM_BASH_DIG_001_A | PASS 20/10 |
| MM_BUILD_BLOCK_A | PASS 15/10 |
| MM_FLOAT_CLIMB2 | PASS 10/6 |
| MM_BLOCK_BUILD_BASH | DIVERGED step 2850, actor 3 assignment acceptance changed |
| MM_E4_4_SKILLS | DIVERGED step 2075, actor 2 assignment acceptance changed |

The last two are the **pre-existing, accepted shrug-fix divergences**, exactly
matching [builder-shrug-assign-001](builder-shrug-assign-001.md): recorded refused
Builder on Shrugger (198,63), and refused Basher on Shrugger (169,74), now succeed.
Audio introduces no new divergence. Later outcomes are unverified; the recordings
are unchanged. Earlier pre-final-mixer checks are retained in `build.log`,
`targeted-tests.log`, `replays.json` and `replay-*.log` with the same outcomes.

## Review revision: coordinator dependency

The [accepted shrug review](../../build/agent-runs/loop-981e969dfbe149c6bfa22cd1bafb5fbc/0004-review-b017fc/decision.json)
allowed changed replays when listed with their reason; it explicitly accepted
these two divergences. The [audio review](../../build/agent-runs/loop-981e969dfbe149c6bfa22cd1bafb5fbc/0006-review-41b04d/decision.json)
instead checks an unqualified all-replays criterion and says:
"The coordinator must explicitly reconcile those inherited exceptions before
this criterion can pass; preserve the recordings, replay validation and accepted
shrug behavior."

That criterion remains **unmet**, rather than implicitly waived by the earlier
acceptance. The exact conflicting commands are Seafood Sarnie's
`a 2850 3 4 0 198 63 7 0 0 0` (Builder refused during Shrugger), and A Long and
Lonely Road's `a 2075 2 5 0 169 74 7 0 0 0` (Basher refused during Shrugger).
The accepted Y6 fix now accepts both commands. The earlier shrug results and
audio results agree on all fifteen wins and both divergence steps/actors.
Neither stopped replay establishes a later win with the accepted behavior.

The coordinator must resolve how those two inherited exceptions apply to this
task's acceptance before another acceptance review. This worker cannot change
the queue or acceptance criteria, rewrite recordings, weaken replay validation,
or undo the accepted shrug fix to produce seventeen passes. This resume changes
only this report; no production or test revision is justified by the review.

Completed first supervisor run:
[verification.log](../../build/agent-runs/20261116T173911-11176-b6712ac5/verification.log),
**95/95 main smoke and 84/84 recovery passed**, followed by `git diff --check`;
[result.json](../../build/agent-runs/20261116T173911-11176-b6712ac5/result.json)
records verifier exit 0. These are prior-run results, not new executions.
No build, test, replay or original analysis was rerun in this report-only resume.
The incoming report is backed up under
[resume-c3a24e69/report-before.md](../../build/gameplay-sounds-001/resume-c3a24e69/report-before.md).
Get-Process found no clone/build/test runner; supervisor PID 11176 retains the
RepositoryLease. No shared control, recording or prior evidence was modified.

Changed: `src/sim/World.{h,cpp}`, `src/audio/CuePlayback.cpp`,
`src/audio/Mixer.{h,cpp}`, `src/app/main.cpp` (entry/help),
`src/__test__/test_cueplayback.cpp`, new `src/__test__/test_gameplay_sounds.cpp`,
deleted legacy `src/__test__/test_audioapp.cpp`, `CMakeLists.txt`, and this report.
Incoming edits are preserved; supervisor **PID 11176** holds RepositoryLease.
No protected controls were edited. Initial Get-Process check found no live
clone/build/test runner; CIM command-line access was denied, so Get-Process was
used. Backups/incoming diff and task-only patch are under
[build/gameplay-sounds-001/](../../build/gameplay-sounds-001/).
Runs used private `build/gameplay-sounds-001/appdata`; replay/unit paths open no
SaveStore. Normal and installed saves were unused. No original/oracle, debugger,
hooks, capture, installation, agents, detached work, commits or pushes occurred.
Physical listening, exact startup phase, positional gain/pan, retail mixing and
sample timing remain unproved. Full-project gates and parks remain unchanged.

Handoff: three sounds implemented; prior smoke passed 95/95 main and 84/84 recovery; fifteen replay wins and two inherited divergences remain.
Handoff: needs_review with coordinator dependency: explicitly reconcile those exceptions before acceptance; supervisor verification and human listening remain next.

What to check: **Just Climb / MM_CLIMB_002**, enter/retry a level: one Letsgo before
the hatch releases, with no repeats on pause/resume or fast-forward. **MM_BASH_002**,
bash a pillar: repeated digging sound during successful bites, no persistent loop
after walking resumes. **MM_BUILD**, let a full bridge finish: one Chink on each
of its last three bricks; pause stops new attacks, and fast-forward has distinct
warning attacks without stacked tails.
