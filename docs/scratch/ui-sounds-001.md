# ui-sounds-001 - needs_review

Playable v1, [playtest 9 Y12/Y13](../playtest/2026-11-16-playtest-9.md).
Difficulty: routine; recommended model: Sonnet; assigned Codex override used.
Before evidence is the human ledger: silent buttons/assignments and Letsgo at
level load. Routes: **MM_CLIMB_002 / Just Climb**, **MM_BUILD**, **M06_KF_1**.
This revision fixes the review's remaining text/sound mismatch. Refreshed
**supervisor smoke is pending**; original fidelity and human listening remain unproved.

## Production call path and revision

`runApplication -> CuePlayback::start -> Mixer::loadFrom -> soundAssetName ->
AssetLibrary` loads Action and Changeop through the installed patch/base overlay.
`runApplicationLeg -> playUiClick -> Mixer::play` supplies Changeop for stocked
skill/Nuke selection, pause/resume, speed activation and hatch-rate clicks/repeats.
`World::assign -> takeCues -> CuePlayback::play -> Mixer::play` maps successful
Assign to Action; refused assignments and empty-space clicks stay silent.
SkipPlaying discards overlapping requests for the same Action/Changeop clip.
Held rate changes request at most one click per frame, only if the rate changes;
initial clamped presses provide feedback. Skill holds/key repeat add no requests.

**Correction to the first attempt:** moving sound alone to first spawn left text
on a clock that had to expire before release. The review rejected that mismatch.
The same **3.5 s entry / 1.5 s Retry** countdown, strict expiry below **-1 s**,
frame-time/pause/focus gate and full door duration now live in `entryDelay`.
`LevelStartSequence::releaseEnabled(entryDelay.active())` preserves the previous
release predicate; no simulation/release clock or recorder input changed.

`RuntimeSimState::advanceElapsed -> afterUpdate` consumes `firstReleasePending`
on `world.spawned() > 0`. In that **same update** it requests LevelStart/Letsgo
and creates `EntryStartCue::atFirstRelease()` for the existing text compositor.
The text starts at **0.5 s**, the readable **(160,180)-(480,260)** rectangle;
there is no text before first spawn. Its separate frame clock keeps the existing
source-over/additive phases, retires below -1 s, freezes while paused/menu/focus
suspended, and is never multiplied by fast-forward or fed back into release.
One per-leg latch prevents repeats after pause, speed changes and later hatches;
Retry/new levels reset it. Audio-disabled operation still creates text. Direct
diagnostics retain their previous absence of entry text; headless replay is audio-independent.

**Provisional:** immediate base-size text omits the former pre-spawn shrinking
phase. Pause, speed and rate feedback use Changeop without an established original
play site. Nuke's two-click behavior is unchanged. Frontend/confirmation/results,
map node/footer and unsupported Options buttons remain silent: no sample was
established by the bounded static search. Refusals have an evidenced silent branch.
Same-sample overlap suppression is provisional; physical gain/latency/mixing and
original sound/text phase parity are unverified.

## Retained static evidence

Read first: [gameplay sounds](gameplay-sounds-001.md),
[native-022](native-022-sound-cue-routing.md), [hatch input](hatch-click-rate-001.md),
[level text](level-entry-start-cue.md), and retained Changeop research.
Reference: `C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
First-attempt project static REA/Ghidra 12.1.4 evidence was reused; no new REA session
or original execution. Full IDs, instruction/pseudocode packets, sample metadata,
search limits and earlier findings are preserved in
[the detailed record](../../build/ui-sounds-001/revision-2/details.md) and
[static/](../../build/ui-sounds-001/static/).

| Site | Instruction-supported finding |
| --- | --- |
| Assignment | `4556A2` refusal bypasses playback; `4556AA` loads Action handle `4E32FC`, `4556B5` plays it. `assignment-instructions.json`. |
| Selection | Stock check `45ACF0`; mode-2 branch loads Changeop handle `4E32D0` at `45AD05`, plays at `45AD11`. `selection-instructions-2.json`. Original mode meaning remains incomplete. |
| Speed/rate | `455AFF` plate 8 and `425DE3` rate clamp/store establish no playback; chosen feedback is provisional. |
| Letsgo | Accepted `build/gameplay-sounds-001/static/start-instructions.json`: play `425ADB`, latch `425AE3`, before opening/release checks. Requested clone first-spawn point is provisional. |

Installed base samples `SOUNDS\ACTION.WAV` (0.168345 s) and
`SOUNDS\CHANGEOP.WAV` (0.148027 s) decoded through the production loader:
[prior focused.log](../../build/ui-sounds-001/focused.log); no patch override.

## Checks and preservation

Revision [build](../../build/ui-sounds-001/revision-2/build.log) and
[final build](../../build/ui-sounds-001/revision-2/final-build.log) passed with registered
GCC 16.1.0, `-O2 -g`, assertions and eight jobs. Existing initializer warnings remain.
[Focused run](../../build/ui-sounds-001/revision-2/focused.log): **5 pass / 5 obsolete
legacy failures**, 79.69 s. Passed: entrystartcue, cueplayback, gameplay_sounds,
hatch_timing_units, and smoke levelreturn (44.19 s). Its two first-Retry-frame
expectations encoded Y13 (additive text before spawning); they now require
spawned=0, no text, presentation clock=0. Simulation/save assertions and timeout stay intact.

Removed these failing legacy registrations under the explicit Verification tiers
rule, without repairing their assertions or removing any smoke registration:

- `entrycuewindow`: requires the old level-load clock and pre-spawn text.
- `levelmenuwindow`: requires that clock to advance before spawning and early Retry text.
- `hatch_timing_after`: requires visible text to retire before first spawn.
- `hatch_timing_fast`: requires the same obsolete ordering in fast-forward.
- `hatch_timing_retry`: requires the old Retry text seed and retirement before release.

Unit/assets/winning hatch routes remain. [Inventory](../../build/ui-sounds-001/revision-2/inventory.json):
**95 smoke / 232 total**. No new tests, replay fixtures or scripted routes added.
Existing entry-route [CSV](../../build/ui-sounds-001/revision-2/legacy-output/build/hatch-timing-001/after/MM_CLIMB_002/frames.csv)
shows no text until frame 62 / world 5900 ms: spawned=1, cue-visible=1, cue=0.5.
Its [first-release image](../../build/ui-sounds-001/revision-2/legacy-output/build/hatch-timing-001/after/MM_CLIMB_002/first-release.png)
was inspected: readable Let's Go over the first lemming. This is bounded clone
observation from a legacy test with obsolete failing ordering assertions;
same-update audio is source-traced, not a dummy-device listening measurement.

All **15 active promoted human replays** were rerun with the revised
`webolution --game-dir "Lemmings Revolution" --replay <file>`:
[exact filenames, commands, exits and per-file logs](../../build/ui-sounds-001/revision-2/replays.json).
Every file returned exit 0, PASS, won=1, completed=1:

| Level | Saved/needed |
| --- | ---: |
| M06_KF_1 | 93/50 |
| MM_BASH_002 | 10/10 |
| MM_BASH_MINE_WLR1 | 20/10 |
| MM_BLOCK_001_A | 24/15 |
| MM_BLOCK_BOMB_A | 12/10 |
| MM_BOMB | 17/10 |
| MM_BUILD | 20/10 |
| MM_CLIMB_002 | 10/1 |
| MM_DIG_000 | 19/10 |
| MM_FLOAT | 10/1 |
| MM_MINE | 20/10 |
| MM_W_012 | 16/10 |
| MM_BASH_DIG_001_A | 20/10 |
| MM_BUILD_BLOCK_A | 15/10 |
| MM_FLOAT_CLIMB2 | 10/6 |

The two retired recordings remain excluded per [policy](../playtest/replays/README.md);
no recording changed. Prior-content supervisor [verification](../../build/agent-runs/20261116T203507-1620-0f37d269/verification.log)
passed 95 main smoke / 84 recovery; it does not verify this revision. Refreshed
smoke is pending supervisor execution; no full suite was duplicated.

Revision files: `main.cpp`, `EntryStartCue.{h,cpp}`, comments in `AppEntry.h` and
`LevelStartSequence.h`, `test_levelreturn.cpp`, `CMakeLists.txt`, this report.
First-attempt audio/test edits are retained. Incoming work/legacy output backed up
under [revision-2/before/](../../build/ui-sounds-001/revision-2/before/).
Get-Process checked runners; supervisor PID 1620 owns RepositoryLease, with no
competing lease or protected-control edits. Tests used private
`build/ui-sounds-001/revision-2/appdata`; replay never opens SaveStore. Installed/normal
saves and unrelated edits are preserved. Files use LF; git diff --check passes.
No original execution, agents, detached work, installation, commits or pushes.

Handoff: first-spawn text and sound share one per-attempt latch; 5 focused checks and 15/15 active replay wins; five obsolete legacy registrations retired.
Handoff: needs_review; refreshed supervisor smoke, independent review and the user's listening/playtest are next. Fidelity gates and parks are unchanged.

What to check: **MM_CLIMB_002 / Just Climb**, enter/Retry: no text or voice at load;
first spawn shows readable Let's Go and plays Letsgo together once. Pause/resume
and speed changes do not restart them. **M06_KF_1**, later hatches do not repeat it.
Select Climber and assign it: Changeop selection, Action success, silent duplicate
refusal. **MM_BUILD**, skill/pause/speed/Nuke/hatch buttons give short non-stacking
feedback; held hatch clicks stop at limits. Frontend/map remain silent pending evidence.
