# exit-yippee-sound-001: saved lemmings play Yippee

Status: **needs_review**. Playable v1, [playtest 6 V4](../playtest/2026-11-15-playtest-6.md).
The human observed silence when lemmings reached the balloon. This closes that
audio dependency through the already accepted cue path. Difficulty: routine;
recommended model: Sonnet; this worker uses the explicit Codex override.
Mandatory supervisor **smoke verification is pending**, followed by independent
review and the user's next playtest. No original execution occurred.

## Before and production change

The actual installed route for confirmation is **MM_CLIMB_002 / Just Climb**:
ordinary frontend -> map node 0 -> assign Climbers -> balloon -> Results.
The V4 ledger is the before evidence. Source inspection found that
`World::step` already sets `State::Exited`, increments `saved_`, and appends
`SoundCue::Exit` together. `World::cue` does not coalesce Exit events, and
`Lemming::alive()` prevents an exited actor from being counted again. However,
`audio::CuePlayback` deliberately left Exit unrouted. That adapter omission,
rather than missing simulation events, caused the silence.

After: `runApplication` owns one `CuePlayback` across frontend/gameplay legs;
its idempotent `start` preloads the new Yippee route through
`Mixer::loadFrom` -> `soundAssetName` -> patch/base `AssetLibrary` ->
`loadSound` / `decodeWav` -> existing load-time `resampleTo`.
`RuntimeSimState::advanceElapsed` calls the application's `afterUpdate` after
each World update, including fast-forward passes. `drainCues` ->
`CuePlayback::drain` -> `World::takeCues` -> `CuePlayback::play` now maps each
Exit to Yippee -> `Mixer::play`. Each save requests its own voice and cursor;
play only enqueues under the existing short SDL device lock. SDL's callback
renders the samples; the frame loop never waits for the 0.45-second cheer.
No simulation emission, exit predicate, release, skill or clock code changed.

Only Exit opts into a provisional averaging bus: sum live cheer samples in
double precision, divide by live voice count, then attenuate that bus to the
headroom left by existing sounds. A single cheer retains its gain; coincident
cheers cannot multiply their peak into clipping. Finished voices stop counting
on the exact sample frame, so their tails do not attenuate later samples.
Exit voices are separate from the existing additive 32-voice limit: a group
arrival cannot cut off earlier cheers or steal other SFX slots. Their lifetime
is the clip duration, and their population is bounded by the finite saved
actors. Existing additive effects/music retain their prior mixing and cap.
The callback allocates no new sample buffers. No music or other cue was added.

## Static evidence and installed sample

Read first: accepted [native-022 cue routing](native-022-sound-cue-routing.md),
[exit presentation](exit-balloon-path-001.md),
[window lifecycle](frontend-window-quit-001.md), and retained exit-state research
[round 11](../../old/docs/scratch/phase3-round11-exit.md). The older native-022
finding that Yippee lacked a retained play site was correct for that task;
the new targeted evidence resolves that gap without rewriting its history.

The only analysis target was the registered
`C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The project static REA profile used Ghidra 12.1.4. Full returned records,
including limitations, are under [static/](../../build/exit-yippee-sound-001/static/).
Pseudocode is decompiler output, not original source; the decisive statements
were checked against complete instruction reads:

| Instructions | Finding | Retained record |
| --- | --- | --- |
| `0x450890..0x450895` | Level receiver `0x4C6A08` calls registration `0x45E51F`. | `registration-caller-instructions.json` |
| `0x45E540..0x45E552` | Push Yippee string `0x4C0C84`, register via `0x41D7D0`, store returned handle at receiver + `0x1C8B0`. Arithmetic: `0x4C6A08 + 0x1C8B0 = 0x4E32B8`. | `registration-instructions.json` |
| `0x418257..0x4182A1` | Save-state step `0x41818A`, after timer >= 1, increments per-type and total saved words (`0x4CBAA4`). | `exit-step-instructions.json` |
| `0x4182D3..0x4182EA` | Push flag 0, load handle `[0x4E32B8]`, push mapped actor position, call `0x445274` at `0x4182E5`, return completed status 2. | `exit-step-instructions.json` |
| `0x4452D5..0x445344` | Wrapper resolves the handle through sound manager `0x58CB90`, records the positional sound and calls sample play `0x497F39` for flag 0. | `play-wrapper-instructions.json` |

Key Evidence IDs: registration
`ev_2d5c7c112fd8d70aaf5dda0d099e4c73e105ca312b1b2a9743e78ebf611a0552`,
save step `ev_55f9a1ff10eb6c8f6c17136d49c67b9dce98f56de5d66d46ef7e7c41bfef6966`,
wrapper `ev_cbc3af0ea3ad6ec0c435dce9db205dc7ea8506f9fae13b903b9bbe1d3c66172c`.
The handle xref finds that save-site read. The sample-play instructions also
show a rotating five-buffer index; this does not establish retail gain or
overlap loudness. Analysis was closed with `close_binary({})`, without snapshots.

The read-only installed archive inventory and production WAV decode agree:
**SOUNDS\YIPPEE.WAV**, base `lemmings.box`, offset **46,220,251**, **4,592 bytes**,
8-bit unsigned PCM, mono, **10,101 Hz**, **4,548 frames**, **0.450252 s**.
No patch override exists. Payload SHA-256:
`8966d72ede344402d110c61e16df45e2f493dabb0fdabac1752d8e989d33ee98`.
[Asset record](../../build/exit-yippee-sound-001/installed-sample.json) and
[production decode log](../../build/exit-yippee-sound-001/targeted-tests-final.log).
The sample identity is traced, not a provisional choice among candidates.

## Checks, images and limits

- Registered UCRT64 GCC/CMake build of `webolution`, `test_exit_sound`, and
  `test_cueplayback` passed with assertions, no warnings in the final build:
  [build-final-clean.log](../../build/exit-yippee-sound-001/build-final-clean.log).
- Targeted CTest **2/2 passed, 0.22 seconds**:
  [targeted-tests-final.log](../../build/exit-yippee-sound-001/targeted-tests-final.log).
  New legacy `exit_sound` checks per-update save/cue equality, buffered event
  multiplicity, repeated-drain silence, and bounded mono/stereo mixing through
  100 simultaneous loud samples, cancellation, volume and headroom. It never
  initializes SDL or opens a device. These small synthetic units protect logic
  that could silently regress; they do not establish natural gameplay.
- Existing smoke `cueplayback` passed using SDL dummy audio, including its
  missing-backend, missing-WAV, no-opt-in and disabled-audio cases. Its original
  assertion that Exit must be silent encoded V4; only that expectation, the
  resulting route counts and its archive fixture were updated for the newly
  evidenced route. No smoke assertion was weakened, no timeout changed, and no
  test was removed. Smoke membership stays **95**; the new test is legacy.
- `git diff --check` passed on changed tracked files; all task text files use LF.
  No full verification profile was run by this worker; the supervisor owns it.

Retained clone route images inspected for context: Just Climb's
[balloon after 10 saves](../../build/exit-balloon-path-001/after/pending-0.png)
and [Results, saved 10](../../build/exit-balloon-path-001/after/results.png),
from the accepted earlier exit task. These are historical clone images, not
new before/after audio evidence. Images cannot establish whether a cheer played.
Per the current verification tiers, no replay fixture, frozen build, per-task
scripted route or new image capture was added. The after behavior is supported
by the production path and unit checks; human listening is still pending.

The averaging/headroom policy is explicitly provisional in code. Static
evidence establishes the retail cue and its relation to counting a save,
not exact volume, attenuation, pan or overlap mixing. A future authorized
original observation of isolated and closely spaced exits would settle those
audio details. The original counts after its save-state timer; the clone's
existing save moment is immediate exit recognition. This task attaches audio
to that accepted clone moment and does not introduce the original delay or
claim timing/audio fidelity. All parks and full-project gates are unchanged.

## Preservation and handoff

Incoming edits were preserved; the parent supervisor holds RepositoryLease
(PID **49132**). The process check found no live clone/build/test runner before
edits. CIM process inspection was denied; `Get-Process` supplied the process
check instead. No protected control was edited. Backups are under
[before-source/](../../build/exit-yippee-sound-001/before-source/), with the incoming
diff/status and [task-only.diff](../../build/exit-yippee-sound-001/task-only.diff)
beside them. No original, oracle, debugger, hooks, install tools, capture,
subagents, concurrent writers, commits or background worker was launched.
Targeted runs set private LocalAppData under
`build/exit-yippee-sound-001/appdata`; neither normal nor installed save was
read/imported/reset/written by this task.

Changed files: `src/audio/CuePlayback.cpp`, `src/audio/Mixer.{h,cpp}`, new
`src/audio/OverlapMix.h`, `src/app/main.cpp` (help text only),
`src/__test__/test_cueplayback.cpp`, new `src/__test__/test_exit_sound.cpp`,
five CMake registration lines, and this report.

Handoff: ready for supervisor `verify.py --profile smoke` and independent diff/call-path review.
Handoff: V4 acceptance is the user's next playtest; retail mixing and timing fidelity remain unproven.

What to check: **MM_CLIMB_002 / Just Climb**, assign Climbers and let them reach
the balloon: each saved-counter increment produces one Yippee, including close
arrivals without distorted or cut-off cheers; Results still reports the same
saved total. With audio disabled or unavailable, the same play remains usable.
