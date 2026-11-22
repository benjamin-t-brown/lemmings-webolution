# Current run status (generated)

Derived from tasks.json and the latest loop result; do not edit manually.
Task acceptance is not project completion or proof of gameplay fidelity.
Read IMPLEMENTATION_PLAN.md for priorities and docs/STATUS.md for the fidelity ledger.

## Queue

- oracle-001: **blocked** — Propose a bounded original-game comparison
- native-006: **blocked** — Add expected-result coverage for texture pixel-format enumeration
- multi-hatch-release-001: **blocked** — Release lemmings per hatch on multi-hatch levels
- rat-hit-sound-002: **blocked** — A lemming punched off by a rat plays the original's sound, not Ohno

## Recently completed queue entries

- particle-tuning-001: Fewer brown explosion particles and longer dig and mine debris throws (playtest 27 S3, S4)
  Accepted: 2026-11-22T09:20:55.250696+00:00
  Verification evidence: `build/agent-runs/20261122T090708-14116-034aec37/verification.log`
  Verification evidence: `build\agent-runs\20261122T090708-14116-034aec37\verification.log`
- load-missing-keys-001: Load-time cell mesh creates missing key vertices on every level, as 0x43A31D does (playtest 27 S2 follow-up)
  Accepted: 2026-11-22T17:05:22.111893+00:00
  Verification evidence: `build/agent-runs/20261122T092055-14116-b5dfb4f8/verification.log`
  Verification evidence: `build\agent-runs\20261122T092055-14116-b5dfb4f8\verification.log`
- level-music-start-001: Level music starts at once on Try again; the original waits for the title cue (playtest 28 T1)
  Accepted: 2026-11-22T17:22:21.793823+00:00
  Verification evidence: `build/agent-runs/20261122T170522-46400-840a02ad/verification.log`
  Verification evidence: `build\agent-runs\20261122T170522-46400-840a02ad\verification.log`
- rate-hold-speed-001: Holding a hatch changes the release rate far slower than the original (playtest 28 T2)
  Accepted: 2026-11-22T17:36:54.633656+00:00
  Verification evidence: `build/agent-runs/20261122T172512-46400-c72b5974/verification.log`
  Verification evidence: `build\agent-runs\20261122T172512-46400-c72b5974\verification.log`
- release-exe-name-001: Release downloads ship the game as lemmings_webolution(.exe)
  Accepted: 2026-11-22T17:50:52.249810+00:00
  Verification evidence: `build/agent-runs/20261122T174609-46400-a3e654c4/verification.log`
  Verification evidence: `build\agent-runs\20261122T174609-46400-a3e654c4\verification.log`

## Latest loop

- Result: `build/agent-runs/loop-3e04d2db3b4c4bee82cc01a22f993f60/result.json`
- Status: **blocked**
- Duration: 0h 50m 07s
- Completed in this invocation: load-missing-keys-001, level-music-start-001, rate-hold-speed-001, release-exe-name-001
- Reason: The queue has 210 done tasks, four blocked records, and no planning-required placeholder. Both Playtest 28 defects have accepted fixes, and the recorded Playtest 26 and 27 fixes on How do we get up there! await human confirmation. The next playable-v1 dependency is a human playtest of that level from the ordinary map, through the lower striped gate to results and, after a win, a restart to confirm that the map retains completion. The retained gate evidence establishes restored actor visibility, 

## Documentation handoff

At each accepted milestone or priority change, reconcile IMPLEMENTATION_PLAN.md and
docs/STATUS.md with the accepted review and its verification log. Keep detailed
per-run reports historical; append corrections rather than replacing their evidence.
A generated ledger does not update narrative conclusions or historical fidelity gates.
