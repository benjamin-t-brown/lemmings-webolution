# native-022: gameplay sound cue routing

Status: **needs_review**, pending separate read-only acceptance. The worker did
not edit the queue.

Milestone step: play a level in the connected application, where it was silent.
Scope: **cue routing only.** This does not establish native sample timing,
volume, mixing or resampling fidelity, music, FMV, full-frame or audio
acceptance. The `docs/STATUS.md` audio row remains open and was not edited.

Exe SHA-256 of the retained static reads:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. No
original executable, copied instruction, oracle, hook, debugger, capture or
launcher was run. No new static-export analysis was added; only existing notes
were read.

Accepted native-017/native-018 startup/selection and save-codec-001 completion
and restart integrations precede this task. Gameplay audio is the next assigned
unparked play-step dependency; the older STATUS descriptions are not a reason
to redo those integrations.

## Source-traced failure (not an observed retail failure)

`World::cue()` records `SoundCue` values and `World::takeCues()`
(`src/sim/World.cpp:1057`) was called only by `src/__test__/test_world.cpp`.
`src/audio/Mixer.cpp` was not in `cmake/webolution_sources.cmake` and nothing in
`src/app` opened an audio device, so gameplay produced no sound. The sim's
`SoundCue::Splat` also named a retail cue (`Splat`) that the executable never
plays.

## Call path (after)

`main()` (`options.enableAudio = true`) -> `runApplication` constructs one
`audio::CuePlayback` (one `Mixer`), applies `--no-audio` / the test observer, and
for the frontend route calls `playback.start()` once after the library and
session exist -> every `runApplicationLeg` (frontend window, then level) shares
that pointer; direct-level runs call the same idempotent `start()` -> in the level
loop `RuntimeSimState::advance(..., afterTick=drainCues)` drains
`World::takeCues()` after **each** `World::tick()`, and `drainCues()` runs again
right after `World::assign()` -> `CuePlayback::play()` -> the single
`kRoutes` table -> `Mixer::play()` (and the optional recording observer).

Device lifetime: each window leg used to end in `SDL_Quit()`, which also
force-stops the audio subsystem. `app/SdlShutdown.hpp::quitWindowSubsystems()`
(used by `FrontendWindow.cpp` and the leg in `main.cpp`) calls
`SDL_QuitSubSystem(VIDEO|EVENTS)` only when audio is initialised, and plain
`SDL_Quit()` otherwise, so non-audio runs keep their previous shutdown exactly.
The error-path `SDL_Quit()` calls in `main.cpp` are unchanged (they end the run).
`Mixer` now stops only the audio subsystem it started itself.

## Cue evidence table

| Sim cue | Retail cue | Address / source note | Status |
|---|---|---|---|
| `OhNo` | `Ohno` | `0x00414420` (helper step `0x00414370`, guard byte `[0x004CBB40]`==0): `old/docs/scratch/phase3-round7.md` "When Ohno plays"; gate byte measured `0x00` and play at fuse end in `phase3-round7-live.md` | **routed** |
| `Splat` | `FallSplat` | played once at `0x0040B4B6` inside splat enter `FUN_0040b3ec` (type `0x1a`) after a long fall; `old/docs/scratch/phase3-round28-die.md`. Retail `Splat` (`DAT_004E32D4`) has 0 play references | **routed, corrected from Splat** |
| `Assign` | `Chink` (`0x004C0C8C`) | string registered only; no retained play site in `old/docs`, `docs`, `docs/scratch`, `decomp-lemmings/docs` | **unknown, unrouted** |
| `Exit` | `Yippee` (`0x004C0C84`) | `old/docs/ROUND11_BRIEF.md` says only that the sim fires it; no play site | **unknown, unrouted** |
| `Explode` | `Explode` (`0x004C0C94`) | `phase3-round7-live.md` describes the sim's own blast cue, not a retail play site | **unknown, unrouted** |
| (entry) | `Letsgo` (`0x004C0C7C`) | string only; no retained play site | **unknown, unrouted** |
| menu/frontend sound, music | - | no evidence | **open, unrouted** |

`Die` is evidenced (`0x004042CF`, fluid `+0x128==1`) but the sim has no `Die`
cue, and sim emission was not changed, so it is not routed here.

## Installed WAV availability (executed, read-only)

The registered `cueplayback` CTest read the repository-local base/patch overlay
through `AssetLibrary`, resolved `soundAssetName()`, read the payload bytes and
decoded each WAV with the production decoder. Both resolve to the base archive;
the patch does not override either entry.

| Cue / archive entry | Overlay source | WAV bytes | Decoded rate / channels | Frames / samples | Seconds |
|---|---|---:|---|---|---:|
| `Ohno` / `SOUNDS\OHNO.WAV` | lemmings.box | 5,412 | 10,638 Hz / mono | 5,368 / 5,368 | 0.504606 |
| `FallSplat` / `SOUNDS\FALLSPLAT.WAV` | lemmings.box | 26,668 | 22,050 Hz / mono | 13,312 / 13,312 | 0.603719 |

Both are present, readable and decodable, rather than merely present in the
archive name block. These are asset measurements, not native playback evidence.
Full output: `build/native-022/ctest-main.log`, test `cueplayback`.

## Authentic source-built before/after demonstration

The permitted command was run verbatim from the repository root:

```text
git --no-pager show --no-ext-diff --no-textconv HEAD:src/app/main.cpp
```

Its output was retained using the file tool as
`build/native-022/frozen-main.cpp` before further edits in this revision:
55,104 UTF-8 bytes, SHA-256
`d4a3546c79d3f4b227fd1c5e0920a673887a82103efab83698e5b7bfcfc870b4`.
A subsequent read compared equal to the captured git-show output, character for
character. This is authentic HEAD source, not a hand-written reconstruction.

`test_audiodemo.cpp` builds separate `test_audiodemo_before.exe` and
`test_audiodemo_after.exe` under `build/agent-native/main`. It includes the
frozen/current main.cpp unchanged and substitutes only the level loader in that
test translation unit. Both link the same current shared source dependencies.
This isolates main.cpp's application wiring; it is not a whole historical
checkout or original-game comparison. A clean checkout without the ignored
frozen source still registers the current-side test.

Both drivers run the application's direct simulated level path with dummy
video/audio, `--native --scene-off --frames 80` and the existing 0.2-second
test clock. The synthetic setup retains the installed MM_CLIMB_002 collision
terrain and flags (256x96). It supplies one lemming, one bomber, no exits, an
untimed level and integer releases every tick. Bomber entrance (32,53) is the
installed entrance; SDL key 3 plus an actor click performs the production
assignment. The fall fixture selects an existing uninterrupted air gap, entrance
(55,20), from that same terrain. Neither driver changes World emission/timing.
No observer mutates the application's World: pending-cue inspection drains a
copy only.

Executed CTest results under `SDL_AUDIODRIVER=dummy`:

| Application source | Scenario | Tick | Dummy device ready | App cues consumed | Cues pending in copy | Literal observer output |
|---|---|---:|---|---:|---:|---|
| frozen HEAD | bomber fuse | 26 | no | 0 | 2 (Assign, OhNo) | none |
| current | bomber fuse | 26 | yes | 2 | 0 | Ohno |
| frozen HEAD | long fall | 11 | no | 0 | 1 (Splat) | none |
| current | long fall | 11 | yes | 1 | 0 | FallSplat |

All four scenarios returned 0 and exited via observer-driven SDL_QUIT.
Expectations are literal retail names, not another recovered invocation's
answers. The frozen app leaves its cues pending; the current app drains them
through the real production adapter. Reports:
`build/native-022/before.txt` and `build/native-022/after.txt`.
The adapter observer records a request; it does not measure audible output,
sample timing or the mixer's waveform.

## Ordinary frontend application demonstration

`audioapp` calls public runApplication with only `--game-dir`, default
frontend routing, audio opt-in and the production wall clock: no explicit
`--frontend`, frame-limit argument, seeded ticks, injected setup, saved flags
or World mutation. The frontend observer queues RETURN, RETURN and a click on
the existing unlocked node1, whose production map mapping is MM_FLOAT.

Executed result:

```text
ordinary Menu3 -> Confirm4 -> Map5 -> MM_FLOAT: result=0 frontendFrames=3 levelFrames=204 tick=104 device=1 drained=1 cueQuit=1 observer=FallSplat
```

The unchanged installed setup naturally produces a long-fall splat. Its cue
observer receives literal FallSplat and queues SDL_QUIT; the level observer
confirms a death and drained cue. Frame counts depend on the production clock
and machine speed and are not expectations. Tick 108 and CTest's existing
120-second timeout are failure guards only. Success requires the cue and the
observer-driven quit, not expiration of either guard. This case passed in
26.30 seconds including the existing audio-on/default/--no-audio scenarios.
The connected route uses no persistence store and touches no installed save.

The pre-existing three audioapp scenarios still verify the device survives
frontend window teardown, default test options open no device, and --no-audio
opens none and suppresses observation. CuePlayback tests cover the synthetic
archive table, Splat-to-FallSplat correction, missing WAV and disabled audio.
The absent-backend case now requires failure plus a diagnostic and continuing
routing; it no longer permits an informational "not exercised" pass.

## Changed files

Production routing from the preceding implementation attempts was retained:

- `src/app/main.cpp`: --no-audio/help, one adapter across legs, assignment
  and per-tick drains.
- `src/audio/CuePlayback.{h,cpp}`: the evidence table, loading and routing.
- `src/audio/Mixer.{h,cpp}`: SDL boundary and audio-subsystem ownership.
- `src/app/AppEntry.h`: audio opt-in/observer and read-only frame counters.
- `src/app/RuntimeHud.{h,cpp}`: optional callback after each World tick.
- `src/app/SdlShutdown.hpp`, `src/app/FrontendWindow.cpp`: keep the mixer
  alive across frontend/game window legs.
- `cmake/webolution_sources.cmake`, `CMakeLists.txt`: production audio
  through webolution_objects and SDL2, without the recovery subproject.
- `README.md`: routed/unrouted cues and fidelity qualifications.

This revision changed `CMakeLists.txt`, `src/__test__/test_audioapp.cpp`,
`src/__test__/test_cueplayback.cpp`, added
`src/__test__/test_audiodemo.cpp`, and updated this note plus ignored
`build/native-022` evidence. The existing production wiring needed no further
change. The added CTests are audiodemo_before/after; the existing audio tests
gain the missing scenarios and measurements. No sim source changed.

## Executed verification and retained failures

`python tools/agent_control/verify.py --profile all` completed with exit 0:
registered `C:/progs/msys2/ucrt64/bin/g++.exe` targeting
x86_64-w64-mingw32, source-built SDL dummy tests, repository-local asset reads.

- Main **78/78 passed**, 425.20 s; the prior 76 registrations remain.
- Recovery **84/84 passed**, 30.57 s; unchanged registrations.
- levelreturn passed in **235.53 s**, retaining natural success/failure,
  completion and clone-restart checks.
- cueplayback **0.22 s**, audioapp **26.30 s**, audiodemo_after **3.41 s**,
  audiodemo_before **2.98 s**.
- Reference SHA-256 was verified by reading bytes only, matching the identity
  above. Recovery keeps an empty LEMMINGS_REFERENCE_EXE and native_int_array
  as its sole native_-prefixed test.
- The verifier's final git diff --check passed. Changed source text has LF
  endings. No existing timeout was raised; the pre-existing levelreturn 360 s
  limit and its historical save-codec-001 explanation remain.

Retained final logs/JUnit: `build/native-022/ctest-main.log`,
`ctest-recovery.log`, `main-results.xml`, `recovery-results.xml`,
and `verifier-tail.log`. Original verifier outputs/inventories remain in
`build/agent-native/{main,recovery}`. The final post-build CMake edit only
corrected a comment to say dummy device rather than no device.

The post-report git diff --check also returned 0. Git warned about pre-existing
CRLF text in installed configuration/backup files; those installation files
were left untouched. The task's changed sources and notes use LF.

History is not erased: earlier workers lacked a shell and delivered no
before/after run or connected cue observation; their supervisor reported
76/76 main and 84/84 recovery passes. This revision first encountered a demo
link error (OBJECT dependencies required direct webolution_objects linkage).
The next invocation passed 76/78 main tests but failed the two new demo
fixtures: unlimited -1 stock is displayed as zero by the existing UI adapter,
and the original gap search considered only row0. Fixed with positive bomber
stock and searching all empty runs; no production behavior was changed for
these fixture errors. See `failed-verification-summary.txt`,
`failed-ctest-main.log`, `failed-main-results.xml` and
`failed-fixture-{before,after}.txt` under build/native-022. Recovery was not
reached on that failing invocation.

## Preservation, limits and next action

The initial worktree was already extensively staged/dirty, including the moved
recovery tree, production integrations and an installed hooks.log change. Those
edits were retained. This worker did not edit the queue, controls, verifier,
tool registry, shared instructions, STATUS gate rows, recovery PROGRESS or
FUNCTIONS, native-006 deferral, oracle-001, the 372 / 6,186 baseline or the
D1/D3/digger parks. No commit, reset, clean, stash, push, original execution,
original-byte oracle, hook, debugger, capture, launcher or installation write
was performed. The installed save/reference remain read-only; existing save
tests also check the installed save remains unchanged.

Ordinary startup, selection, play, finish and completion surviving clone restart
have source-built support from the accepted tasks and the passing levelreturn
scenario. This task adds only evidenced gameplay cue routing and its connected
demonstration. It does not establish native sample timing, volume, mixing,
resampling, music, FMV, full-frame/audio acceptance or full playable-game
fidelity. The STATUS audio row remains open.

Unknown/unrouted: Chink (Assign), Yippee (Exit), Explode, Letsgo, menu/frontend
sound and music. Die has a retained play site but no current sim cue. No new
emission was invented. The observer also sees routed requests when a WAV/device
is unavailable; it alone is not evidence that samples were mixed.

Next action: separate read-only acceptance review of native-022 and the retained
executed evidence, then supervisor-controlled needs_review -> done if every
criterion passes. No subsequent task is started here. Future routing requires
retained play-site evidence; native audio fidelity and media remain separate
unfinished dependencies.
