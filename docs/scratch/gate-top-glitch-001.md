# Playtest 26 R2 · Graphics above the higher gate

In **How do we get up there!** (`H02_JC_1`, node 35), a section of green wall and
riveted steel vanished above the higher striped gate. Oblique views also showed
triangular slivers hanging from the trim. The matching crops show the restored
wall and trim, with the incoming frame on the left. [Playtest 26 R2](../playtest/2026-11-21-playtest-26.md) · [Open](../../build/gate-top-glitch-001/crops/10-initial-z2-d0-paired.png) · [Oblique](../../build/gate-top-glitch-001/crops/10-initial-z1-d16-paired.png) · [Moving](../../build/gate-top-glitch-001/crops/10-posed-moving-z2-d0-paired.png) · [Closed](../../build/gate-top-glitch-001/crops/10-posed-closed-z2-d0-paired.png)

The higher gate is id 10, at X `[128,133)` and original Y `[67,89)`. It starts
open in the recordings. Attempts 8, 11 and 14 open the lower gate, id 1, and
pass their replay checks at steps 23824, 21926 and 29175. Attempt 14 supplies
the paired views. The observer fed fractional and closed poses for id 10 to
the production mesh code, since those attempts never moved it. The six gate-10
cameras use yaw units 130.501 at zoom 0, 114.501 and 146.501 at zoom 1, and
126.501, 130.501 and 134.501 at zoom 2. The ordinary app screenshot also shows
the corrected open view. [Observed frames and replay logs](../../build/gate-top-glitch-001/after/) · [Production screenshot](../../build/gate-top-glitch-001/production-h02-upper.png)

`TubeScene::build` stamps the gate rectangle before `buildCellMeshes` reads
`grid0`. The split creates new static corners above and beside the gate. The
asset table lacks several of them, including the outer key `(128,89,1)`. The
mesh builder returned index 0 for those corners, sending them to `(0,0,60)`
at the base of the tube. The roof quad in bucket 16, band 5 had indices
`[0,286,0,0]`. The loader now supplies missing ordinary keys using the same
cylinder interpolation as terrain rebuilds, then regenerates the mesh before
`runApplicationLeg` draws the opaque tube through `TubeScene::render`. The
roof quad now uses `[600,286,603,604]`. Gate motion still uses the separate
movable vertex list. The source changes are confined to `CellMesh`, `TubeScene`
and a legacy regression case in `test_counter_gates.cpp`. [Incoming mesh trace](../../build/gate-top-glitch-001/before-detail/H02_JC_1-1795301570652508000-14/observe.log) · [Fixed mesh trace](../../build/gate-top-glitch-001/after/H02_JC_1-1795301570652508000-14/observe.log) · [Loader](../../src/app/TubeScene.cpp#L168) · [Regression case](../../src/__test__/test_counter_gates.cpp#L167)

Static READ used the registered x86 EXE, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, in Ghidra
12.1.4. The face builder at `0x463410` calls the ordinary resolver at
`0x4635BD`–`0x463624`. At `0x43A356`, a missing key enters the allocation branch,
which calls template interpolation at `0x43A3AE`. The separate gate records
are resolved at `0x4273A8` and animated by `0x426FEC`. Gathering the missing
keys and rebuilding once is the clone's implementation of that lookup. Both
static sessions closed without snapshots. Some oblique views still show
diagonal overlaps within the retracted striped face below the repaired trim.
Native depth ordering and original frame appearance remain unverified. [Resolver instructions](../../build/gate-top-glitch-001/rea-key-instructions.json) · [Face builder instructions](../../build/gate-top-glitch-001/rea-banks-instructions.json) · [Pseudocode](../../build/gate-top-glitch-001/rea-key-decompile.json)

The 102-level census found **11 affected levels**, including H02_JC_1, among
52 levels with this gate type. H02_JC_1 went from 21 unresolved lookups to zero,
and the other ten also reached zero. The **50 gate-free levels** produced
byte-identical seeded opening frames through the production app. [Key census](../../build/gate-top-glitch-001/key-census.tsv) · [Frame comparison](../../build/gate-top-glitch-001/sweep-frame-comparison.tsv)

| Active promoted recording on a gated level | Release CLI result |
| --- | --- |
| The Iron Curtain (`MM_ALL_8_003`, attempt 5) | PASS at 18207, 21/20 |
| Wheelbarrows of Doom (`M08_MG_1`, attempt 16) | PASS at 24891, 99/98 |
| Penelope Lem Stop (`M04_KF_1`, attempt 3) | PASS at 16014, 32/32 |

All three passed again in the release CLI with private LocalAppData. The lead
retired **Designed with love** (`M11_KF_1`, attempt 14) and preserved the
97/85 human win byte for byte. The accepted `builder-wall-pass-001` change
turns an inverted lemming at the stairs at step 30496. It reaches the exit
earlier. Both earlier CLI runs diverged at step 37451 with one save where
the recording expects zero. A fresh human win would restore replay coverage
for this level. [Current replay results](../../build/gate-top-glitch-001/review-followup/promoted-results.tsv) · [Retirement ledger](../playtest/replays/README.md#retired-recordings) · [Archived win](../playtest/replays/2026-11-21-playtest-25/M11_KF_1-1795249788316136900-14.replay.retired) · [Earlier gate run](../../build/gate-top-glitch-001/promoted/results.tsv) · [Builder wall run](../../build/builder-wall-pass-001/promoted-results.tsv)

The release and `smoke_tests` builds completed, and five focused smoke tests
passed. The prior supervisor gate passed 95/95 smoke tests, including the
102-level sweep, and 84/84 recovery tests. The legacy gate regression compiled
and awaits milestone sign-off. [Release build](../../build/gate-top-glitch-001/final-release-build.log) · [Smoke target build](../../build/gate-top-glitch-001/smoke-target.log) · [Focused smoke](../../build/gate-top-glitch-001/focused-smoke.log) · [Prior supervisor run](../../build/agent-runs/20261122T000521-24104-79b788cb/verification.log)

Handoff **needs_review**. Supervisor smoke for this revision is pending.  
Next action is independent re-review and the user's playtest.

**What to check.** In How do we get up there!, rotate to the narrow striped
gate beneath the highest steel ring. Zoom in and view it from both sides.
The green wall and riveted trim should meet continuously above the gate,
with clean edges as the gate moves.
