# Native console output

Playtest 19 J2 reported a flood of routine messages. The existing Windows
Release clone reproduced it during 120 frames of **Now You're Stalking**
(`E22_KF_1`, node 27) and **Escape to Victory** (`E11_KF_1`, node 28). Each
run printed 121 backdrop counters, including the initial render before the
frame loop. `MM_DIG_000` also printed the first sprite draw's RNG state.
[Playtest 19](../playtest/2026-11-19-playtest-19.md) · [Baseline commands and streams](../../build/native-log-quiet-001/baseline.md)

## Message inventory

`WEBOLUTION_NATIVE_DIAGNOSTICS=1` restores the routine native messages below.
In PowerShell, set `$env:WEBOLUTION_NATIVE_DIAGNOSTICS = '1'` before launch.

| Before-change line | Source and frequency | Treatment |
| --- | --- | --- |
| `backdrop: enabled=… pixels-depth-written=…` | `runApplicationLeg → renderFrame`, 121 per 120 frames | Opt-in during play. Observer-driven runs keep the counters for the sweep child log. The counters include normal culls and projection rejections. |
| `sim: tick=…` | `runApplicationLeg`, once per simulated level | Opt-in during play. The existing observer-driven diagnostics and explicit `--sim-ticks` still emit it. |
| `level=… mesh=… yaw=… selector=…` | `runApplicationLeg`, once after the initial render | Opt-in during play. Observer-driven diagnostics still emit it. |
| `presenter: … flags=…` | `runApplicationLeg`, once per level before the SDL loop | Opt-in during play. Frame profiling and observer-driven diagnostics still emit it. |
| `backdrop: sprite scenery generator state at first draw = …` | `renderFrame`, once on `MM_DIG_000` | Opt-in during play. Observer-driven diagnostics still emit it. |
| `frontend: entering … from map (SOURCE SDL adapter)` | Source trace of connected level entry and retry | Opt-in during play. The level-selection observer still gets the exact text. |
| `sim: lemming …` | Source trace of seeded simulation setup, one line per actor | Same condition as the simulation summary. |
| `click-to-assign: skill …` | Source trace of the SDL keyboard skill-selection handler | Opt-in during play. Observer-driven diagnostics still emit it. |
| `record: …` and `record: saved …` | Source trace of successful attempt creation and finalization, including Play-Clone's automatic recording | Opt-in during play. Recording failures still print. |
| `texture-cache: entries=…` | `TextureCacheScope` destruction, once per leg if profiling | Kept under the existing `WEBOLUTION_FRAME_PROFILE`. |

`runApplication` prints `Lemmings Webolution (native)` once on native play
startup. The level path through `runApplicationLeg → renderFrame` and the SDL
loop gates the routine messages at their emitters. Missing or unsupported
assets, raster errors, audio faults, save failures and recorder failures remain
visible. Snapshot confirmations and explicit HUD and caption diagnostics
keep their output. [Native entry and level path](../../src/app/main.cpp#L417) · [Recorder path](../../src/app/Replay.hpp#L70)

The consumer search covered `tools/` (including `verify.py`), `cmake/`,
`scripts/`, `src/__test__/` and the relevant tool documentation. All identified
consumers retain their messages. The level-selection test reads the map-entry
line. The sweep's level observer enables `logLevelDetails`, which also gates
the backdrop counters. The sweep parent captures both streams in `child.log`.
`interestingLog` skips `sim: tick=` and keeps the last six other lines for
successful and failed runs. `runChunk` writes that tail to `parent.txt` as
`child_log_tail`, and `runReport` publishes it in `matrix.csv` as
`p_child_log_tail`. Replay returns before the startup line.
[Consumer search](../../build/native-log-quiet-001/consumer-search.txt) · [Sweep log capture and publication](../../src/__test__/test_levelsweep.cpp#L504)

## Evidence and checks

The retained Web performance fix excluded the backdrop stats from the
Emscripten path. For the sprite-seed message, Ghidra static analysis of
`Lemmings Revolution/Lemmings Revolution.exe` (x86, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`)
found calls at `0x0045D06C` and `0x004451E5` to `0x00447580`. Instructions
load and scale the argument at `0x00447583..0x00447586`, convert it at
`0x0044758C`, and pass it to the RNG seeder at `0x00447592`. Pseudocode
agrees on conversion and dispatch. This establishes the seed path only.
Console policy is a provisional clone choice. Original runtime logging
was unobserved. REA closed without a snapshot.
[Retained seed analysis](scene-branch-research-002.md) · [Static REA results](../../build/native-log-quiet-001/rea-seed.json) · [Web change in Git](../../build/native-log-quiet-001/git-show-web-perf-main.patch)

- Registered UCRT64 GCC 16.1.0 rebuilt `test_levelsweep.exe` and
  `build/native-log-quiet-001/native-release/webolution.exe`. Ten console
  checks matched using private save paths. Three default 120-frame level
  runs and the frontend startup each gave one stdout line and zero stderr
  lines. The opt-in `MM_DIG_000` run restored all 121 counters, the seed,
  and the setup diagnostics. The profiler and seeded screenshot still
  emitted their explicit diagnostics. A corrupt private save raised one
  warning and kept its bytes. An unknown level exited 1 with its error.
  [Configure](../../build/native-log-quiet-001/revision/configure.log) · [Build](../../build/native-log-quiet-001/revision/build.log) · [Console runs](../../build/native-log-quiet-001/revision/checks.tsv)

- The targeted `test_levelsweep chunk 4` run exited 0 with six child records.
  With the environment opt-in unset, Now You're Stalking emitted three
  backdrop counters. The node 27 parent tail contained all three, up from
  zero in the prior smoke run, and matched the historical tail exactly.
  The supervisor's sweep report will exercise the final CSV publication.
  [Tail check](../../build/native-log-quiet-001/revision/sweep-tail-check.txt) · [Child log](../../build/native-log-quiet-001/revision/sweep/chunk-004/node-027/child.log) · [Parent record](../../build/native-log-quiet-001/revision/sweep/chunk-004/node-027/parent.txt)
- All 23 promoted recordings reran and returned 0 with `PASS`, `won=1` and
  `completed=1` via `webolution --game-dir "Lemmings Revolution" --replay <file>`.
  Now You're Stalking finished at step 24,402, with 49/49 saved. The table
  lists each recording, final step, result and stderr capture prefix.
  [Replay results](../../build/native-log-quiet-001/revision/replays.tsv)

- **Handoff** — `needs_review`. Supervisor `verify.py --profile smoke`, including the level sweep and recovery suite, is pending.
- **Next action** — Review the logging call paths and smoke result, then confirm the console during desktop play.

**What to check** — Enter Now You're Stalking (node 27), rotate the camera,
select a skill and retry. Expect one startup line for the process and no
routine messages during the level or retry. Start again with
`WEBOLUTION_NATIVE_DIAGNOSTICS=1` to see the diagnostic lines.
