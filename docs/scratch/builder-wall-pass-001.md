# Playtest 26 R1 · Builder stairs against a steel ledge

**How do we get up there!** (`H02_JC_1`, node 35), attempt 14,
`H02_JC_1-1795301570652508000-14.replay`. Lemming 23 chains Builder from
(142,57) toward the steel ledge, then turns left at (156,43) at step 21047.
Its last brick occupies (156,44) and (157,44). Follower 49 reaches (156,43)
facing right at step 26727. Coordinates are clone Y-down. [Playtest 26 R1](../playtest/2026-11-21-playtest-26.md) · [Copied recording](../../build/builder-wall-pass-001/inputs/H02_JC_1-1795301570652508000-14.replay) · [Full incoming attempt trace](../../build/builder-wall-pass-001/before/H02_JC_1-1795301570652508000-14.log)

| Recording step | Incoming clone | Fixed clone |
| --- | --- | --- |
| 26779 | Walker at (156,43) facing right | Walker at (156,43) facing right |
| 26780 | Enters (157,43) facing right. The ledge occludes the body. | Stays at (156,43) and turns left. |
| 26836 | Enters (158,43) facing right and falls. | Walks back down the stairs to (155,44). |

Column 157 has authored steel word `0x0003` at rows 39–42, air at row 43,
and the runtime brick at row 44. From the stair, the top would require a
five-cell rise. The counter gates occupy X 17–26 and 128–132. The evidence frames
use the clone scene renderer at an evidence camera and the committed actor
positions. [Incoming trace and raw cells](../../build/builder-wall-pass-001/before-detail/H02_JC_1-1795301570652508000-14.log) · [Fixed trace and raw cells](../../build/builder-wall-pass-001/h02-final/H02_JC_1-1795301570652508000-14.log) · [Incoming frames](../../build/builder-wall-pass-001/frames-before/) · [Fixed frames](../../build/builder-wall-pass-001/frames-final/)

`World::updateSegment` dispatches the walker through `stepElapsed` to
`stepWalker`. The existing step-up loop stopped immediately on the air at
(157,43) and accepted the brick as support. The walker now checks the row
immediately above its prospective feet when walking on or onto a builder
brick, and reverses before entering a one-row opening. A supported one-cell
descent can still clear the ledge. The check uses the actor's gravity and the
wrapped terrain grid. The two-cell step-up limit still governs solid feet.
[Production change](../../src/sim/World.cpp#L830) · [Legacy clearance cases](../../src/__test__/test_builder_brick.cpp#L89)

READ via REA/Ghidra 12.1.4 used the registered x86 executable, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
`0x41BEE6` calls `0x41BFFC`. A clear feet bit at `0x41C47D–0x41C482`
branches to support handling at `0x41C6EF`. Solid feet trigger two upward
probes at `0x41C48E` and `0x41C528`, then the climb or turn decision at
`0x41C5C1–0x41C679`. The Builder has a separate forward head probe at
`0x410091`. The walker's stair clearance is **provisional**. REA closed
without a snapshot. The original route remains unmeasured. [Retained walking READ](../../old/docs/scratch/phase3-round4.md) · [REA instructions, pseudocode and xrefs](../../build/builder-wall-pass-001/rea-evidence.json)

The release CLI reproduced **29 of 30** promoted wins, including **Penelope
Lem Stop** at 16014 (32/32). **Designed with love** (`M11_KF_1`, attempt 14)
diverges at **37451**, the first save. At **30496**, lemming 0 walks inverted
from (4,64) toward (3,65) on Built support at (3,64) with Earth immediately
above its feet at (3,66) in its gravity orientation. The patch turns it at
(4,64), skipping the recorded climb attempt. It reaches the same exit 399 ms
earlier in the traced slice. Both recordings remain intact. [Every promoted result](../../build/builder-wall-pass-001/promoted-results.tsv) · [M11 incoming trace](../../build/builder-wall-pass-001/m11-before/M11_KF_1-1795249788316136900-14.log) · [M11 fixed trace](../../build/builder-wall-pass-001/m11-final/M11_KF_1-1795249788316136900-14.log)

H02 attempts 4–14 replayed successfully with the final trace driver. Attempt
14 also passed the untouched release CLI check at 29175, ending on the map
with 0/50. The release build and `smoke_tests` target built, and focused smoke
tests `world`, `sharedtiming`, `levergates` and `fastforward_units` passed.
The legacy `builder_brick` target compiled and awaits milestone sign-off.
Clone launches used private LocalAppData. An older clone executable opened
a window on the replay command and was stopped. The retained World object
supplied the incoming M11 trace. [H02 CLI](../../build/builder-wall-pass-001/h02-14-cli.log) · [H02 attempt logs](../../build/builder-wall-pass-001/h02-final-all/) · [Release build](../../build/builder-wall-pass-001/release-build-final.log) · [Smoke target build](../../build/builder-wall-pass-001/smoke-target-build.log) · [Focused smoke](../../build/builder-wall-pass-001/focused-smoke.log)

Handoff **needs_review**. The supervisor smoke profile is pending.  
Next action is independent review and a fresh human replay for Designed with love.

**What to check.** In How do we get up there!, build right from the middle
ledge toward the steel overhang, leaving the staircase below its top. Let
ordinary walkers follow. They should turn on the last clear stair column
and walk back down.
