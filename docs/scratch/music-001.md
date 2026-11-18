# music-001

Status: **needs_review; mandatory smoke pending supervisor execution**. Playable v1,
asset-census-001 rank 10; difficulty routine, recommendation Sonnet, assigned Codex
override. Before: human [playtest 1 P16](../playtest/2026-11-12-playtest-1.md)
reported no music everywhere; the accepted [asset census](asset-census-001.md)
found 13 installed music WAVs and no production mixer callers. Connecting them
is the next bounded audio dependency, from the frontend/map to **node 0 /
MM_CLIMB_002 / Just Climb**, and every installed level.

Retained the sound partial implementation from interrupted attempt 1, then
completed its checks, help text and report. Production path:
`runApplication -> persistent CuePlayback/Mixer -> runApplicationLeg` calls
`playFrontendMusic` before the frontend window when no level is already selected,
and `playLevelMusic` before level presentation. The diagnostic map also starts
frontend music. `CuePlayback::playMusic -> Mixer::loadMusicFrom -> musicAssetName ->
AssetLibrary` reads the patch/base overlay and decodes/resamples each track once;
`Mixer::playMusic -> SDL audio callback -> Mixer::fill` loops its PCM independently
of simulation updates. Loading happens between scenes, before the level frame
clock starts. Successes and failures are cached across application legs.

| Transition | Result |
| --- | --- |
| Frontend, map and results pages | `lemming01`, continuing between pages. |
| Level entry, direct level, or Retry | Fresh random `lemming02..13`, restarting even if the draw repeats. Retry skips the menu-music call because selectedLevel remains set. |
| Pause, level menu, focus loss, fast-forward | Music continues at ordinary audio speed; no simulation clock or random state is consumed. |
| Return from level to map/results | Replace the level track with `lemming01` from its beginning. |
| `--no-audio` or test audio disabled | No device, music lookup, or music voice; gameplay continues. |
| Missing device/WAV | Diagnostic and silent music; a failed WAV is not repeatedly loaded. No fallback track. |
| Application exit | Mixer closes its device and releases clips. |

Music uses a **provisional 30% linear gain**, with existing SFX at full gain.
Switching or stopping music removes only music voices, preserving effect tails
and cursors. Music is excluded from the existing **32 additive SFX slots**;
effect requests still start immediately with their existing overlap policies.
Music never enters the SFX observer/counters. The independent music RNG and its
bounded uniform distribution are provisional. Cached stereo PCM is bounded by
the 13 installed tracks (about 69 MB); nothing is decoded in the callback.

Static authority: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
Used reverse-engineer-anything to examine retained attempt-1 Ghidra **12.1.4**
packets in [static/](../../build/music-001/static/), profile digest
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
No new REA session or original execution. The
[instruction cross-check](../../build/music-001/resume-20261117/static-crosscheck.txt)
retains exact Evidence IDs and operands:

- `486C53..486C5A` passes mode **0** to `498DE8`. `498E37` indexes the
  13-pointer table at **4C42FC**, whose strings at **4C4330..4C4420** are
  `music\lemming01.wav` through `music\lemming13.wav`.
- Level loader `4432C1`, at `444392..444399`, passes mode **1**. Nonzero
  mode calls `4040A0` at `498E13`, multiplies by **12.0** (`4B0B80`), adds
  **1.0** (`4B02D0`), and truncates through `49C5AC` at `498E24`.
  `4040A4..4040AF` calls CRT RNG `49C84A` and divides by **32767.0**
  (`4B03B0`). Thus ordinary indices 1..12 select tracks 02..13 rather than
  a per-level filename. The clone avoids the out-of-table endpoint when rand
  returns 32767 and keeps this draw separate from scenery/simulation randomness.

The decompiler omits x87 arithmetic operands; the instructions/constants above
settle selection. Retained `play-music.json` actually resolves to constructor
`4990B0`, so it is **not** evidence of native looping. `load-attempt.json`
traces `45C9AB -> 4432C1`; the complete native Retry/menu/pause playback path was
not instruction-verified. **Looping, pause/focus continuity, frontend/results
continuity and fresh random selection on Retry are the stated clone policies**,
with Retry following the unconditional level-loader selection. Exact native
streaming, RNG sequence, volume, loop boundary and lifecycle parity remain
unproven. Human listening and balance assessment remain pending; PCM checks
establish software output, not physical audibility or original fidelity.

Registered UCRT64 GCC/CMake/Ninja built `webolution`, `test_music`,
`test_work_sound` and `test_positional_sound`, eight jobs:
[build-final.log](../../build/music-001/resume-20261117/build-final.log).
Focused legacy checks **3/3 PASS, 1.22 s**:
[focused.log](../../build/music-001/resume-20261117/focused.log),
[JUnit](../../build/music-001/resume-20261117/focused.xml).
The new legacy music unit takes **0.50 s**: nonzero production-decoded PCM
from all 13 installed tracks, synthetic loop boundaries/stereo gains, immediate
full-gain SFX attack, preserved tails/cursors, 32-effect capacity, track switching,
cached restart, missing-track diagnostics, and disabled/device-free behavior.
No new route fixture or scripted play route was added. The earlier focused run
is retained as `focused-initial.*`; the final paths above include the all-track check.

All **19 active promoted human-win replays** ran individually through
`build/agent-native/main/webolution.exe --game-dir "Lemmings Revolution" --replay <file>`.
Each exits **0**, reports **PASS, won=1, completed=1**. Exact filenames, commands
and individual logs: [replays.json](../../build/music-001/resume-20261117/replays.json).

| Level | Saved/required | Final step |
| --- | --- | ---: |
| MM_BASH_002 | 10/10 | 1844 |
| MM_BASH_MINE_WLR1 | 20/10 | 2272 |
| MM_BLOCK_001_A | 24/15 | 1619 |
| MM_BLOCK_BOMB_A | 12/10 | 1546 |
| MM_BOMB | 17/10 | 954 |
| MM_BUILD | 20/10 | 1685 |
| MM_CLIMB_002 | 10/1 | 1252 |
| MM_DIG_000 | 19/10 | 1014 |
| MM_FLOAT | 10/1 | 978 |
| MM_MINE | 20/10 | 1137 |
| MM_W_012 | 16/10 | 1742 |
| MM_E1_3_SKILLS | 10/5 | 2095 |
| M06_KF_1 | 91/50 | 1544 |
| MM_E1_4SKILLS | 20/10 | 2506 |
| MM_E2_4SKILLS | 14/10 | 2142 |
| MM_BASH_DIG_001_A | 20/10 | 3241 |
| MM_BUILD_BLOCK_A | 15/10 | 2482 |
| E19_KF_2 | 24/20 | 6034 |
| E20_MG_1 | 7/5 | 12810 |

No divergence, recording change or retirement request. Replay returns before
audio startup; these are simulation regressions, not music/listening evidence.
**Smoke remains pending supervisor execution after this worker exits**; no
verifier profile or full suite was duplicated.

Changed task files: `src/audio/{CuePlayback.h,CuePlayback.cpp,Mixer.h,Mixer.cpp}`,
`src/app/main.cpp`, `src/__test__/test_music.cpp`, `CMakeLists.txt`, this report.
Authentic attempt-1 entry backups remain in [entry/](../../build/music-001/entry/);
resumed partial-source backups and retained HEAD main are under
[resume-20261117/](../../build/music-001/resume-20261117/).
The [task-only patch](../../build/music-001/resume-20261117/task-only.patch)
compares final task sources with the authentic pre-task entry, preserving the
accepted counter-gates, evil-rats and refusal-sounds edits. Text is LF and scoped
whitespace checks pass. Supervisor PID **34136** owns RepositoryLease; no
protected controls or shared instructions were edited. Get-Process found no
game/clone/build runner; detailed CIM metadata was denied. Checks use private
LocalAppData `build/music-001/resume-20261117/appdata`; units/replays open no save.
Normal and installed saves were unused. No original/oracle, hooks, debugger,
capture, installs, subagents, detached work, commit or push.

Handoff: music lifecycle integrated; all 13 tracks produce PCM, three focused checks and all 19 active promoted wins pass.
Next: supervisor smoke and independent review; retain needs_review pending those checks and human listening.

**What to check:** frontend -> map -> **node 0 / MM_CLIMB_002 / Just Climb**:
hear menu music continue into the map, then a looping level track under Letsgo,
assignment and exit effects. Pause and open the menu: music continues normally.
Retry: a newly chosen level track starts from its beginning (it may pick the
same track). Return to map: menu music restarts. Launch with `--no-audio`: both
music and effects are silent and the same navigation still works.
