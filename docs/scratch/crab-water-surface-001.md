# crab-water-surface-001

Status: **needs_review**; acceptance is blocked on **lead-owned replay retirement**.
Prior supervisor smoke passed; verification after this revision is pending.
Playable v1, [playtest 10 Z7](../playtest/2026-11-16-playtest-10.md),
**Kriss Kross**, node 19 / `E19_KF_2`. Difficulty routine; recommendation
Sonnet; assigned Codex override used. The next dependency is correcting the
integer surface/contact adapter introduced by accepted [crab-hazard-001](crab-hazard-001.md).

## Recorded failure

The existing human recordings were inspected with the production replay parser
and a diagnostic death observer, using the task-entry simulation objects. These
are clone observations, not original comparisons or generated gameplay routes.
Full logs: [recording-4-deaths.log](../../build/crab-water-surface-001/recording-4-deaths.log)
and [recording-1-deaths.log](../../build/crab-water-surface-001/recording-1-deaths.log).

| Recording under build/playtest/replays/ | Zero-based step / clock ms | Actor / tribe | Death cell (X,Y-down) / original Y |
| --- | --- | --- | --- |
| E19_KF_2-1794876874744373400-4.replay | 525 / 19,637 | 0 / water (1) | (156,50) / 45 |
| same | 704 / 26,000 | 2 / water (1) | (156,50) / 45 |
| E19_KF_2-1794877146847683900-1.replay | ends at step 260 / 10,004 | no deaths | attempt closes before crab contact |

The 96-row level's crab model is at **(148,38,0.3)**, floor cell **(148,57)**.
Its stored contact origin is **(142,38,0)** and relative maximum **(14,7,1)**:
closed volume **X 142..156, original Y 38..45, Z 0..1**. The contacted water
segment, id **9**, spans **X [152,160)**, bottom **38**, live height **7.5**,
surface **45.5** at both deaths. Adjacent segments are equally full. The clone
puts matching feet at `floor(45.5)=45`, mistakenly inside the crab's upper edge.
The longer recording also has five later splats at (171,91), steps 1093, 1261,
1433, 1603 and 1774; these are distinct from the two crab deaths.

## Static authority and corrected rule

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`, x86,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Used reverse-engineer-anything with REA Ghidra **12.1.4**, static only; closed
without snapshot arguments. No original execution, debugger, hooks or capture.

- `430F93..430FA1` passes **actor+8** directly to `430C9C`. The fresh
  [collector instructions](../../build/crab-water-surface-001/static/collect_asm.json),
  evidence `ev_f5c76f0f8a7a4071706cee767d511b692178ebed01a27a99f06f0a584adf72ff`,
  corroborate the [pseudocode](../../build/crab-water-surface-001/static/collect.json).
- `430C9C` compares three **double** coordinates to translated relative boxes.
  In particular `430D15..430D23` and `430D8C..430D9A` test both Y edges
  inclusively. Both actor-facing branches use the same bounds. Neither the
  collector nor this predicate checks tribe (+4C). Fresh [instructions](../../build/crab-water-surface-001/static/contact_asm.json),
  evidence `ev_d8430e175eb86b18424bdc697fc38e20fb011dfacd2258c0219f4e87bcd23045`,
  cross-check [pseudocode](../../build/crab-water-surface-001/static/contact.json).
- The accepted [crab census](../../build/crab-hazard-001/census.txt) establishes
  floor placement and boxes. Accepted [fluid research](fluid-hazard-001.md)
  instruction-checks `41C321..41C36F`: water/sheet 1 and acid/sheet 2 match;
  exact original fluid attachment and immersion remain unresolved.

Production path: `runApplicationLeg` (or headless `replayLevel`) ->
`RuntimeSimState::advanceElapsed` -> `World::advanceElapsedMs` -> `updateSegment`
-> `stepElapsed/attachFluidHazard` -> receiver updates -> `stepCrabs`.
`attachFluidHazard` continues to quantize matching feet to the mean surface.
Only the crab contact point now recovers **minY + live height** when an upright
matching lemming is horizontally inside that fluid and its feet equal the
quantized surface row. The stored box and closed comparisons remain intact.
Thus **45.5 > 45** survives; an actual surface on/below the upper edge still
contacts. Height zero disables the adapter; a point down at the crab remains
lethal for every tribe. Attack/death accounting, rendering and clocks are unchanged.

**Provisional:** reconstructing the mean surface fraction from integer feet is
a clone adapter, including acid consistency. Original exact fractional actor
placement, waves and radial coordinates are not established here. The radial
lane remains 0. This is not an original-fidelity or timing pass.

## Checks and limits

Registered UCRT64 build: [final-build.log](../../build/crab-water-surface-001/final-build.log).
The existing **legacy** `crab` test passes in **0.31 s**:
[passing-crab-ctest.log](../../build/crab-water-surface-001/passing-crab-ctest.log).
Added only small logic checks for fractional/integer surface edges, direct floor
contact, drainage, horizontal exclusion, other tribes and inverted gravity;
the existing closed-volume/animation/14-crab installed-data checks still pass.
The direct-floor sample is stationary because an upright matching walker placed
underwater is first lifted by the existing fluid code. Seafood Sarnie's drained
contact remains the same closed **[110,124] x [9,16] x [0,1]** check.

The longer Kriss Kross recording now intentionally **DIVERGES at step 525**:
expected dead=1, actual dead=0. [Contact snapshot](../../build/crab-water-surface-001/corrected-contact-diagnostic.log)
shows actor 0 alive at (156,50). No count checks were relaxed. The short recording
still **PASSes**, saved=0/20, won=0, completed=0. The failed attempt is not a win.

All 17 promoted `.replay` files ran individually through
`webolution --game-dir "Lemmings Revolution" --replay <file>`; exact filenames,
exits and individual log links/names are retained in
[replay-results.tsv](../../build/crab-water-surface-001/replay-results.tsv).

| Level | Strict replay result, saved/required |
| --- | --- |
| M06_KF_1 | PASS 93/50 |
| MM_BASH_002 | PASS 10/10 |
| MM_BASH_MINE_WLR1 | PASS 20/10 |
| MM_BLOCK_001_A | PASS 24/15 |
| MM_BLOCK_BOMB_A | PASS 12/10 |
| MM_BOMB | PASS 17/10 |
| MM_BUILD | PASS 20/10 |
| MM_CLIMB_002 | PASS 10/1 |
| MM_DIG_000 | PASS 19/10 |
| MM_FLOAT | PASS 10/1 |
| MM_MINE | PASS 20/10 |
| MM_W_012 | PASS 16/10 |
| MM_E1_3_SKILLS | PASS 10/5 |
| MM_E2_4SKILLS | Existing step-1195 divergence, dead 0 -> 1 |
| MM_BASH_DIG_001_A | PASS 20/10 |
| MM_BUILD_BLOCK_A | PASS 15/10 |
| MM_FLOAT_CLIMB2 | PASS 10/6 |

Every PASS has won=1/completed=1/exit 0. The Abyss exception is the already
accepted [fall-out-death-001](fall-out-death-001.md) behavior; that level has no
crab. Consequently this is **16/17 strict PASS**, not a claim that all promoted
recordings pass literally. No new promoted-replay divergence was introduced.
Seafood Sarnie's winning recording remains retired for its earlier shrug-click
defect; neither it nor any other recording was edited. Natural Seafood Sarnie
contact and the corrected Kriss Kross crossing await the user's playtest.

Changed: `src/sim/World.cpp`, `src/__test__/test_crab.cpp`, this report.
Entry backups and [task-only source diff](../../build/crab-water-surface-001/task-source.diff)
preserve all incoming edits. Supervisor PID **4628** owns RepositoryLease;
Get-Process found no live clone/build before edits (CIM metadata was denied).
No protected controls were edited. Runs used private LocalAppData under
`build/crab-water-surface-001/appdata`; replay constructs no SaveStore.
Normal/installed saves were unused. No new route fixture, frozen build,
identity gate, assertion/timeout change, agents, commit or push. Text is LF.
At the original worker exit, smoke/recovery and independent review were pending.

## Review revision: lead-owned replay retirement

The supervisor subsequently passed **95/95 main smoke and 84/84 recovery** in
[20261117T020843-4628-7d6dfe2b/verification.log](../../build/agent-runs/20261117T020843-4628-7d6dfe2b/verification.log).
The supplied independent review supports the contact correction, recorded death
cells, direct-contact behavior and smoke result. It requests revision solely
because The Abyss recording is still promoted. These are completed prior-run
checks, not a new verification of this revision.

The [retirement policy](../playtest/replays/README.md#retired-recordings) says a
recording dependent on an intentionally fixed defect is renamed
`*.replay.retired` **by the lead**, with a reason in that README. This invocation
is an implementation worker; it cannot perform that lead-owned action or edit
shared instructions. The accepted [fall-out-death-001](fall-out-death-001.md)
correction must remain: actor 0 falls through The Abyss's open bottom at
**(99,95)**, step **1195**, clock **42,098 ms**, and dies instead of walking on
the artificial floor. Strict replay therefore expects dead=0 but observes
dead=1. The level has no crab; no crab behavior change can reconcile it.

Prepared [lead-retirement-proposal.patch](../../build/crab-water-surface-001/review-revision/lead-retirement-proposal.patch)
contains the exact rename, policy reason and node-18 replay-status correction
for the lead to review and apply at an idle boundary under RepositoryLease.
It preserves the recording bytes, its human-win provenance and all replay
assertions; it does not claim a replacement win. The original replay,
README and level table have **not** been changed by this worker. Entry backups,
the unmodified report and the proposed handoff are retained in
[review-revision/](../../build/crab-water-surface-001/review-revision/).

The actual promoted set remains **16/17 strict PASS** in the retained replay
run. Applying the proposal would leave 16 promoted recordings, all of which
passed in that run; this is a conditional statement, not a new 16/16 run or a
completed retirement. The lead should apply the retirement, run the promoted
replays and reconcile this report before independent acceptance. New worker
checks were limited to evidence review and scoped diff/format/preservation
checks; no new clone, build, smoke suite or original analysis ran. Production
source/tests remain as reviewed. Post-exit smoke belongs to the supervisor.

Handoff: fractional fluid surface contact now keeps water lemmings above the crab safe; closed direct contact remains lethal.
Handoff: needs_review; lead must reconcile The Abyss replay with the prepared retirement proposal, then replay/supervisor verification and independent review.

What to check: **Kriss Kross (node 19)**, send water lemmings across the full
tank over the crab at column 148: they survive without triggering its attack.
**Seafood Sarnie (node 15)**, drain the pool and walk into the crab around
column 116/original Y 9: the lemming dies once with the existing particles and
the crab attacks. A lemming that actually reaches the crab remains vulnerable.
