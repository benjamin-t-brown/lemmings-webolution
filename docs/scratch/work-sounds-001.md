# work-sounds-001

**needs_review; revised work overlap; supervisor smoke pending for this revision.** Playable v1,
[playtest 11 A2/A8/A11](../playtest/2026-11-16-playtest-11.md).
Difficulty routine; recommendation Sonnet; assigned Codex override used.
Before: Bomber/Nuke blasts, miners on **Sergeant Bash, node 20 / MM_E1_4SKILLS**,
and diggers on **The Crowded House, node 21 / E20_MG_1** were silent.
The missing cue routing/emission is the next dependency; no simulation behavior
or timing changes. Human listening remains the acceptance check.

## Production path and limits

`World::stepMiner/stepDigger -> clearSkillBox -> cue(Work)` now requests one
work sound only if the existing stroke removed terrain. Miner accumulates the
existing clear calls' return counts; their footprint/order, steel refusal,
movement and transitions are unchanged. Digger uses its existing removed count.
`Work = Bash` preserves every existing enum value and basher expectation while
sharing its existing `Mine` route and overlap policy. No additional asset load
or duplicate route is needed for the same sample.

Bomber: `assign -> fuse expiry -> stepExploder -> explode -> cue(Explode)`.
Nuke: `nuke -> stepNuke -> nukeExploding -> stepExploder -> explode`, the same
existing cue. The adapter now routes it to `Explode`; the accepted Nuke Ohno
guard and existing per-buffer explosion coalescing are untouched.

Both paths: `RuntimeSimState::advanceElapsed -> afterUpdate -> drainCues ->
CuePlayback::drain -> World::takeCues -> play -> Mixer::play`. Startup uses
`CuePlayback::start -> Mixer::loadFrom -> soundAssetName -> AssetLibrary`
patch/base overlay and the production WAV decoder/resampler. Help is updated.

**Provisional policies (revised after review):** one Mine request per successful
hybrid stroke, using `RestartPlaying` for bash/mine/dig. Every request removes
the previous Mine tail under the device lock and starts the sample at frame zero;
there is **at most one Mine voice**, with no skipped request or delayed queue.
Fast-forward/other workers can truncate a tail; requests within one audio callback
interval share the latest fresh attack. Explode uses `RestartPlaying`:
`Mixer::play` removes voices of
that clip under the device lock before adding its new attack, so **at most one
explosion voice** exists. A same-frame group therefore cannot sum explosion
tails; later blasts restart rather than queue. At fast-forward a tail may be
cut short. No new requests occur after work stops or during pause; a one-shot
already playing can finish. Other effects retain their existing mixing.
These are player-facing clone choices, not recovered retail mixing/cadence.
The observer records requests, including overlap-suppressed requests; it does
not measure audible output. No physical listening or waveform measurement ran.

## Static evidence and installed samples

Read first: accepted [gameplay audio](gameplay-sounds-001.md),
[Nuke Ohno](letsgo-text-nuke-ohno-001.md), [hatch](hatch-click-rate-001.md),
and retained [Scrape research](../../old/docs/scratch/phase3-round25-scrape.md).
Only target: `C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
Used reverse-engineer-anything and static REA/Ghidra **12.1.4**, profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Full returned packets, Evidence IDs and limitations: [static/](../../build/work-sounds-001/static/).
Pseudocode was checked against [instructions](../../build/work-sounds-001/static/crosscheck.txt):

- `45E570..45E582`: Explode string `4C0C94` -> receiver+`1C8B8`;
  `4C6A08 + 1C8B8 = 4E32C0`. Helper `4081FC` loads it at `408205`,
  then plays through `44535B` at `40821C`, without an Ohno/Nuke guard.
  The existing clone blast-completion cue is the provisional integration point;
  this bounded read did not trace the original animation callback registration.
- `45E660..45E672`: Mine string `4C0CE8` -> receiver+`1C8E0` = `4E32E8`.
  Miner `4188F6` skips zero/protected cells, clears at `418B29`, and on
  subpass **-1** loads Mine at `418D21`, playing at `418D39` via `44535B`.
- Digger `415A43` refuses protected cells at `415B1B/415B2A`, clears at
  `415B84`, and after its successful row/effects loads Mine at `415D69`,
  playing at `415D81`. `415B84` is the cell store, not the sound call.
  Existing refusal Scrape/MineMetal sounds are outside this task.

Closed analysis with empty arguments, without snapshots; no original execution.
Production decoder output in [focused.log](../../build/work-sounds-001/focused.log):

| Installed entry (base lemmings.box; no patch override) | Bytes | Mono rate | Frames | Seconds |
| --- | ---: | ---: | ---: | ---: |
| `SOUNDS\EXPLODE.WAV` | 1,196 | 11,127 Hz | 1,152 | 0.103532 |
| `SOUNDS\MINE.WAV` | 27,436 | 22,050 Hz | 13,696 | 0.621134 |

## Authorized smoke expectation correction

Only `test_cueplayback.cpp` smoke expectations that excluded Explode changed.
The route-count change is the direct consequence of routing that eighth cue.
Before and after assertion lines (the unrelated Chink absence is retained):

```cpp
// Before
check(routes.size() == 7, "seven cues have retained play-site evidence (playtest 9 Y3-Y5/Y12)");
for (sim::SoundCue unevidenced : {sim::SoundCue::Explode})
  check(!audio::routedCueName(unevidenced).has_value(), "no retained play site => unrouted");
check(audio::unroutedCues().size() == 1, "Explode remains unknown");
check(std::none_of(heard.begin(), heard.end(),
                   [](const std::string& n) { return n == "Explode" || n == "Chink"; }),
      "Explode and Chink are not played");
// After
check(routes.size() == 8, "eight cues have retained play-site evidence (including Explode, playtest 11 A2)");
check(audio::routedCueName(sim::SoundCue::Explode) == std::optional<std::string_view>("Explode"),
      "Explode routes to its explosion sample (playtest 11 A2)");
check(audio::unroutedCues().empty(), "Explode is now evidenced and routed");
check(std::count(heard.begin(), heard.end(), "Explode") == 1,
      "the bomber blast plays Explode once");
check(std::none_of(heard.begin(), heard.end(),
                   [](const std::string& n) { return n == "Chink"; }),
      "Chink is not played");
```

The synthetic archive gains EXPLODE.WAV so unchanged per-route assertions also
exercise it. All other smoke assertions and every timeout/registration remain
untouched. No smoke test was added or removed.

## Checks and preservation

First-attempt checks (retained history): registered UCRT64 GCC/CMake/Ninja,
`-O2 -g`, assertions, eight build jobs:
[build.log](../../build/work-sounds-001/build.log), exit 0. Existing
RetailLevel.cpp Lever initializer warnings remain. Focused **2/2 PASS, 0.25 s**:
[log](../../build/work-sounds-001/focused.log), [JUnit](../../build/work-sounds-001/focused.xml).
Existing smoke cueplayback checks installed decoding, normal Bomber routing,
disabled/missing-device/missing-sample paths. Expanded existing **legacy**
gameplay_sounds guards exact stroke/event coupling, empty/steel refusal,
pause/fast-forward, repeat drains, and normal/prelit Nuke explosion routing
without Ohno. No replay fixture, frozen build or scripted route was added.
That attempt's later supervisor [verification](../../build/agent-runs/20261117T045930-15712-4a3becdc/verification.log)
passed **95/95 main smoke and 84/84 recovery**. It does not verify this revision.

All **17 active promoted human-win replays PASS**, exit 0, won=1, completed=1.
Exact filenames, commands, steps and individual output logs:
[replays.json](../../build/work-sounds-001/replays.json). Saved/required:
MM_BASH_002 10/10; MM_BASH_MINE_WLR1 20/10; MM_BLOCK_001_A 24/15;
MM_BLOCK_BOMB_A 12/10; MM_BOMB 17/10; MM_BUILD 20/10; MM_CLIMB_002 10/1;
MM_DIG_000 19/10; MM_FLOAT 10/1; MM_MINE 20/10; MM_W_012 16/10;
MM_E1_3_SKILLS 10/5; M06_KF_1 91/50; MM_E1_4SKILLS 20/10;
MM_E2_4SKILLS 14/10; MM_BASH_DIG_001_A 20/10; MM_BUILD_BLOCK_A 15/10.
No divergence/retirement request; recordings/checks are unchanged. Replays
bypass playback and establish simulation regression results, not audio output.

Changed: `src/sim/World.{h,cpp}`, `src/audio/CuePlayback.cpp`, help in
`src/app/main.cpp`, `src/__test__/test_cueplayback.cpp`, legacy
`src/__test__/test_gameplay_sounds.cpp`, and this report. Incoming edits retained:
[backups](../../build/work-sounds-001/entry/),
[task-only.patch](../../build/work-sounds-001/task-only.patch).
Get-Process found no clone/build/test runner at entry; CIM command-line access
was denied. Supervisor **PID 15712** owns RepositoryLease; no protected controls
were edited. Runs used private `build/work-sounds-001/appdata`; units/replays
open no SaveStore. Normal/installed saves were unused. Changed text is LF;
scoped `git diff --check` passes. No original/oracle, debugger, hooks, capture,
installation, subagents, detached workers, commits or pushes. Original timing,
positional gain/pan, mixing fidelity and historical gates remain unproved.

## Review correction and revision checks

Independent review of that attempt rejected the work-sound criterion: the
original `SkipPlaying` policy discarded overlapping requests even though the
observer counted them. The first report explicitly acknowledged those drops;
calling the policy provisional did not satisfy every-successful-stroke playback.
The other four criteria passed. This revision changes only Work's overlap policy
to the existing `RestartPlaying` path; simulation, triggers, assets and explosion
policy are unchanged. No new static analysis or original execution was needed.

New **legacy** `work_sound` is a subsecond PCM unit, guarding a failure the
observer cannot detect. It pauses a dummy SDL device and steps the production
`CuePlayback::play -> Mixer::play -> Mixer::fill` path, comparing exact output
to sample-zero PCM after each request. Both synthetic audio and installed Mine
pass at short overlapping-worker and fast-forward intervals (3780/4725 frames),
with an unrelated SFX retained. Four bursts of 100 requests each retain one
fresh attack without summed tails; the last one-shot finishes and stays silent.
Two narrow friend declarations allow deterministic private mixer access; no
production mixer logic or public playback API changed. This is output-buffer
evidence, not physical listening or original mixing fidelity.

Revision [build](../../build/work-sounds-001/revision/build.log) exit 0;
[focused log](../../build/work-sounds-001/revision/focused.log) and
[JUnit](../../build/work-sounds-001/revision/focused.xml): **3/3 PASS, 0.55 s**
(cueplayback, gameplay_sounds, work_sound). New unit is labelled **legacy**;
no existing smoke assertion, registration or timeout changed in this revision.
The authorized first-attempt assertion corrections quoted above are retained.
All **17 active promoted recordings again PASS**, exit 0, won=1, completed=1:
[each filename, command and result](../../build/work-sounds-001/revision/replays.json).
No divergence, recording change or retirement request. Mandatory smoke for the
revised content is **pending supervisor execution**.

Revision files: `CuePlayback.cpp`, test-access friends in `CuePlayback.h` and
`Mixer.h` (plus its comment), `test_work_sound.cpp`, its legacy registration in
`CMakeLists.txt`, and this report. Earlier task changes and unrelated edits remain.
Revision-entry [backups](../../build/work-sounds-001/revision/entry/) and diff
are separate from the authentic first-attempt entry backups. Supervisor lease
PID 15712 remains in place; no protected controls edited. CIM process-command
access was denied; Get-Process found no clone/build/test runner at entry.
Private revision LocalAppData is `build/work-sounds-001/revision/appdata`;
tests/replays use no normal or installed save. Physical latency/listening,
truncated-tail preference and original overlap fidelity remain unverified.

Handoff: fresh work attacks replace dropped overlaps; PCM checks and all 17 promoted replays pass.
Handoff: needs_review; supervisor smoke and independent review next, followed by human listening.

What to check: **MM_BOMB / Bomber Tutorial**, assign one Bomber: Ohno at fuse end,
then a blast sound at the explosion. Nuke a group, including a primed Bomber:
blast attacks without Ohno or loud stacked explosion tails, also at fast-forward.
**Sergeant Bash, node 20 / MM_E1_4SKILLS**, mine through earth: one work sound per
successful stroke at normal speed and fast-forward; work a second miner nearby
and check new hits restart the attack without loud stacked tails or a delayed
backlog, with no repeats after stopping. **The Crowded House, node 21 /
E20_MG_1**, dig the upper snow shelf: repeated work strokes, silence on steel
refusal or after stopping; pause adds no new attacks.
