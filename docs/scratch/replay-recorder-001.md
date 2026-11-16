# replay-recorder-001

Status: **needs_review**. Playable-v1 smoke-gate plan step 2. Difficulty:
difficult; recommended model Opus; this worker used the requested Codex override.
Supervisor smoke verification is **pending**. No original execution or new REA
analysis was needed; the retained [shared-dt contract](shared-timing-impl-001.md)
and [command research](gameplay-commands-research-001.md) supplied the relevant
timing/input context. Historical fidelity gates and parks are unchanged.

Before this task, the clone had no input-recording CLI or headless consumer.
The next dependency was retaining a played attempt without writing another
scripted route. Installed routes checked here include MM_CLIMB_002, MM_FLOAT,
and MM_BLOCK_001_A, through the ordinary frontend and production SDL adapter.

Changed files: `src/app/main.cpp`, new `src/app/Replay.hpp`,
`scripts/Play-Clone.ps1`, `CMakeLists.txt`, new
`src/__test__/test_playtest_replays.cpp`, `docs/playtest/replays/README.md`, and
this report. Pre-existing release-rate and other edits were preserved; task-entry
copies are in `build/replay-recorder-001/entry/`. No queue/controller/verifier,
shared instructions, existing test assertions, or timeouts were edited.

Production path: `runApplication` parses `--record DIR`; `runApplicationLeg`
creates a fresh recorder for each loaded attempt. After each admitted input
event/held-rate repeat it records effective selection, pause, fast-forward,
release interval/rate, and Nuke changes. The assignment branch records the
resolved actor index, skill, pre-assignment pose/state and acceptance, before
its existing auto-resume. Camera/pixel picking is already resolved at that
boundary. After `RuntimeSimState::advanceElapsed`, the recorder retains the
integer budget, hatch gate, counts, clock and completion phase. Completion,
Retry, map abandonment and close publish separate files with final
saved/required/won/completed fields. A successful result uses the existing
latched `LevelEndTransition` outcome.

**Provisional clone format**, not original replay compatibility: a step is one
admitted `advanceElapsed` call, including its fast-forward passes. The hybrid
simulation is NOT determined by skill commands plus the old 5-Hz tick number:
integer-dt partitioning, zero-ms updates, and the presentation's release gate
matter. Recording preserves those actual budgets/gates without retuning clocks.
`--replay FILE` branches before frontend, SDL devices and SaveStore construction,
loads the same retail setup, applies the same ordered commands, and calls the
same `advanceElapsed` path with those integer budgets. Build identity is a
cached FNV-1a-64 label of the running clone image plus compiler version, computed
before gameplay clocks start. A different build is reported and still checked.

**Source-built checks**, not human playtests or original comparisons:

| Recorded attempt | Assignments | Reproduced result |
| --- | ---: | --- |
| MM_CLIMB_002 | 10 Climber | saved 10, required 1, won |
| MM_FLOAT | 9 Floater | saved 9, required 1, won |
| MM_BLOCK_001_A, existing matched placements | 5 Blocker | saved 23, required 15, won |
| MM_W_012, existing matched placements | 3 Blocker | saved 0, required 10, lost |

Existing drivers were reused with a build-only adapter that appends `--record`;
no new route, baseline build, replay fixture or identity-test registration was
added. All generated recordings remain under ignored `build/`.

- [Existing two-level runs](../../build/replay-recorder-001/existing-checks.txt):
  play took 20.752/14.139 s; initial headless checks took 0.031/0.028 s.
- [Matched-route checks](../../build/replay-recorder-001/matched-checks.txt):
  the existing three-route driver passed in 92.242 s; all five attempt files
  reproduced, including the five-assignment win in 0.095 s. The existing adaptive
  Blocker driver instead lost three attempts; those losses also reproduced
  ([retained results](../../build/replay-recorder-001/blocker-checks.txt)).
- [Control checks](../../build/replay-recorder-001/control-corpus-final.txt):
  pause/resume, fast-forward toggles, Nuke completion, retries and map exits
  reproduced. The first ad-hoc menu invocation lacked its required output-dir
  environment variable; the corrected invocation passed. Both logs are retained.
- [Final checks](../../build/replay-recorder-001/final-checks-latest.txt), after the
  [final build](../../build/replay-recorder-001/build-latest.log): 23 retained
  attempts passed the corpus consumer with invalid SDL drivers; a fresh CLI
  recording round-tripped. Altered saved result, assignment and count failed at
  steps 429, 126 and 0 respectively; truncation and missing CLI paths also failed.
  Replay created no private appdata/save. `playtest_replays` passed empty in
  0.01 s and is labeled **legacy**; the 95-test smoke list is unchanged.

Bookkeeping uses small deque blocks and constant-time scalar snapshots, without
per-frame disk I/O or actor scans. The [consumed-buffer measurement](../../build/replay-recorder-001/recorder-cost-consumed.txt)
measured 23.9–27.0 ns/update across three 500,000-update runs, including allocation;
serialization ran afterward. Whole-frame samples were noisy: [first series](../../build/replay-recorder-001/performance-summary.txt)
-2.10%, [second series](../../build/replay-recorder-001/performance-final/performance-summary.txt)
+1.30%. Those samples do not isolate recorder cost or establish a speedup.

Limits: memory grows with admitted updates (about 64 bytes/record plus container
overhead). Records are buffered until the attempt ends; a crash leaves an
incomplete `.partial`, and I/O errors are reported without blocking ordinary
gameplay/progress. Abandoned/direct diagnostic attempts can pass as reproduced
prefixes (`completed=0`); they are not completed wins. Diagnostics identify the
first divergent recorded count/phase or assignment, not every internal state
change. Picking/UI behavior, rendering and cross-platform floating-point parity
are not covered. Release-rate recording is source-traced at both press and hold
boundaries; repeats without an intervening update coalesce to their final
effective interval, preserving the running countdown. No new human playtest was
performed, and mandatory smoke verification remains for the supervisor.

`Play-Clone.ps1` now records by default into `build/playtest/replays/`, outside
version control. Use `-Rebuild` for the next playtest. After a human win, run
`webolution --game-dir "Lemmings Revolution" --replay <file>` on that recording;
require `PASS`, `won=1`, `completed=1`, then copy the unchanged `.replay` into
`docs/playtest/replays/` with a descriptive level filename. The legacy CTest
recursively checks every promoted `.replay`; no files have been promoted here.

Two-line handoff:
Recorder, headless consumer, default launch flag and legacy corpus check are implemented; retained multi-assignment wins and negative checks reproduce correctly.
Supervisor: run the smoke gate, then independent review; the user's next playtest confirms ordinary recording and promotion.

What to check: **MM_CLIMB_002** (Climber) and **MM_BLOCK_001_A** (multiple Blockers),
launched with `scripts/Play-Clone.ps1 -Rebuild`. Win, locate that attempt's
`build/playtest/replays/*.replay`, and run `--replay` on it: expect no window,
exit 0, and identical saved/required/won/completed values; Retry must create a
different attempt file.
