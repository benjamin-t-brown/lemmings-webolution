# miner-axe-001 — installed Axe child

**needs_review; coordinator scope decision required.** Playable v1, asset-census-001 rank 8;
difficult / recommended Opus / assigned Codex override. Route: **node 5,
MM_MINE / Just Mine**, mining into metal. Before evidence is the accepted
[asset census](asset-census-001.md): the separate Axe resource had no production
consumer. The playtest ledgers establish skill-presentation priorities; this
particular missing child is source/asset-traced, not a new human observation.
Read the accepted census, [hatch dependency](hatch-click-rate-001.md),
[refusal sounds](refusal-sounds-001.md) and gadget presentation reports first.

**Scope correction for review:** the supplied construction address is on
**refusal**, not every successful mining stroke. The installed `mine` images
already contain the held pickaxe. This implementation adds the traced thrown
child on the existing steel refusal; it does **not** add a second looping axe
over ordinary strokes. Acceptance item 1's ordinary-stroke wording is therefore
**not claimed verbatim** and needs lead/reviewer reconciliation with this new
positive branch evidence. Queue/acceptance text was not edited by this worker.

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Static REA/Ghidra 12.1.4, profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`;
used reverse-engineer-anything. Complete returned packets and limitations are
under [static/](../../build/miner-axe-001/static/); no target instructions ran.

- `418AC0..418ADF` detects protected terrain and sets refusal local byte `-28`.
  `418D75..418D7F` tests that byte and jumps past construction when clear.
  `418E53/418E5B` passes count **1** to `4194F0`; `419506 -> 496E39`, whose
  `496E40/496E49` loads **4E331C** and binds the Axe. Verified against complete
  [miner instructions](../../build/miner-axe-001/static/ins4188F6.json) and
  [binding instructions](../../build/miner-axe-001/static/ins496E39.json).
- [Vtable bytes](../../build/miner-axe-001/static/vtable.json) give
  `4B0B88+4 = 4196C0`. Its
  [instructions](../../build/miner-axe-001/static/ins4196C0.json) test live child
  `+30 > 0`, then `419760 -> 493761`; that helper positions the child and
  dispatches virtual **+4**. This corrects the historical UI-only attribution.
- `418EC9..418EDA -> 419590` sets base life **4.5**, variation **0**.
  `419B70` transfers that life to child `+30`; `419C50` subtracts elapsed
  seconds there and advances position/velocity. Its complete
  [instructions](../../build/miner-axe-001/static/instructions419C50.json)
  cross-check these offsets. Callback selector **11** resolves through
  `4C428C+44` to `49356B`: randomized local components `(6*r,12+5*r,6*r)`
  transformed by the emitter basis, not clone terrain coordinates.
  `419780 -> 496C84` advances child animation/controller rotation.

Production: `runApplication` installs `World::setMinerRefusalObserver` before
simulation. `World::stepMiner` sends that presentation notification at its
existing MineMetal branch **before turning**; state, cuts, refusal conditions,
sound and timing are unchanged. `TubeScene::emitMinerAxe` snapshots position,
facing and gravity into the existing bounded particle storage. The existing
`advanceDeathParticles(levelClockMs)` and `drawDeathParticles` calls own its
life and rendering. Retry rebuilds the scene; pause holds its clock and
fast-forward uses the existing level-time updates. It never enters picking.

The draw selects the installed **8 temporal × 5 directional** Axe images using
the shared view chooser, mirroring, light map, gravity flip and
`actorSpritePlacement` zoom. It tests the same upright physical plane against
scenery and actor coverage, applies the existing terrain-footprint mask, and
does not write scenery depth. **16/40** installed Axe records retain pixel-valued
UV doubles; **24/40** are normalized. Only the raw records are converted from
their exact stored rectangles using the decoded **256×111** atlas. The global
SPR parser/sampler is unchanged. Earlier development renders exposed this UV
issue; final rendering checks use the corrected path.

Provisional: deterministic clone-local launch offset/velocity/gravity,
60-ms age-to-strip phase and spin adaptation. Native random distributions,
emitter basis and discrete integration are not reproduced. Base lifetime 4.5
is traced, not tuned. Only the clone's existing **steel** refusal emits; its
known missing bash/mine one-way refusal behavior remains unchanged. The normal
`mine` pose and all gameplay outcomes remain untouched. Synthetic render checks
are supplemental, not a natural Just Mine playthrough or fidelity evidence.

Registered UCRT64 GCC build: [build.log](../../build/miner-axe-001/build.log).
New **legacy** `miner_axe`: **PASS, 2.27 s**;
[CTest](../../build/miner-axe-001/focused-ctest.log),
[JUnit](../../build/miner-axe-001/focused-ctest.xml). It checks one pre-turn
notification, unchanged outcomes, no notification on success, all installed
clip entries, **320** yaw/facing/gravity/zoom/phase samples, scenery/actor
occlusion, unchanged scenery depth and expiry at 4500 ms. No existing test,
assertion, timeout or smoke membership was changed; smoke count remains 95.

All **19 promoted human-win recordings PASS**, exit 0, `won=1`, `completed=1`:
[exact files, commands and individual results](../../build/miner-axe-001/replays.json).
Final replay build: `exe-fnv1a64:8798f3d74a48da36`, GCC 16.1.0.
No divergence or retirement request; recordings/replay checks were untouched.

| Level | Step | Saved/needed |
| --- | ---: | ---: |
| MM_BASH_002 | 1844 | 10/10 |
| MM_BASH_MINE_WLR1 | 2272 | 20/10 |
| MM_BLOCK_001_A | 1619 | 24/15 |
| MM_BLOCK_BOMB_A | 1546 | 12/10 |
| MM_BOMB | 954 | 17/10 |
| MM_BUILD | 1685 | 20/10 |
| MM_CLIMB_002 | 1252 | 10/1 |
| MM_DIG_000 | 1014 | 19/10 |
| MM_FLOAT | 978 | 10/1 |
| MM_MINE | 1137 | 20/10 |
| MM_W_012 | 1742 | 16/10 |
| MM_E1_3_SKILLS | 2095 | 10/5 |
| M06_KF_1 | 1544 | 91/50 |
| MM_E1_4SKILLS | 2506 | 20/10 |
| MM_E2_4SKILLS | 2142 | 14/10 |
| MM_BASH_DIG_001_A | 3241 | 20/10 |
| MM_BUILD_BLOCK_A | 2482 | 15/10 |
| E19_KF_2 | 6034 | 24/20 |
| E20_MG_1 | 12810 | 7/5 |

Changed: `src/app/TubeScene.h/.cpp`, `DeathParticles.h`, `main.cpp`,
`src/sim/World.h/.cpp` (observer and corrected Axe comment only),
`src/__test__/test_miner_axe.cpp`, its legacy registration in `CMakeLists.txt`,
and this report. Entry backups and task-only diff are under ignored
`build/miner-axe-001/`. Incoming edits were preserved. Supervisor **PID 34136**
holds RepositoryLease; no protected-control/shared-instruction edit was made.
CIM metadata access was denied; Get-Process found no clone/build runner at
entry or handoff. Checks use private `build/miner-axe-001/private-appdata`;
replays never open saves. Installed/normal progress was not used or changed.
No original, oracle, debugger, hooks, capture, install, background work,
subagent, commit or push ran. Both static analyses were closed without snapshots.
**Mandatory smoke remains pending supervisor execution after worker exit.**

Handoff: traced refusal Axe presentation implemented; needs_review, with ordinary-stroke wording explicitly unresolved.
Next: supervisor smoke and independent review reconcile item 1 with the refusal trace, then human playtest; no fidelity gate is closed.

Revision follow-up, **20261118T022755-34136-91175e7c**: implementation is blocked
on a concrete acceptance conflict, not on another rendering fix. The independent
review supplied with this run accepts criteria 2-4 and rejects criterion 1. It
explicitly requires coordinator reconciliation and rejects adding a duplicate
looping axe merely to satisfy its wording. This worker cannot edit acceptance
criteria or substitute a different trigger for the positively traced refusal.

Re-read the retained instruction packet: `418ADF` sets refusal byte `-28`;
`418D75..418D7F` tests it and jumps to `419005` when clear, bypassing the
`418E53` count-one argument and `418E5B -> 4194F0` child construction. The
production notification remains inside `World::stepMiner`'s Steel branch,
before its turn/Walker transition; `runApplication -> emitMinerAxe ->
advanceDeathParticles/drawDeathParticles` remains the presentation path.
The existing legacy unit also explicitly checks no notification on successful
mining. No contradictory retained evidence justifies a new ordinary-stroke
emitter. Exact original held-tool motion and human confirmation remain unknown.

Correction to the earlier pending-verification statements: the prior supervisor
run **20261118T015121-34136-29804968** completed successfully. Its
[verification.log](../../build/agent-runs/20261118T015121-34136-29804968/verification.log)
records **95/95 main smoke and 84/84 recovery passes**, and its
[result.json](../../build/agent-runs/20261118T015121-34136-29804968/result.json)
records verification exit 0. The 19 replay results above belong to that earlier
implementation run. No build, test, replay or static-analysis session was
launched in this follow-up; no new execution result is claimed. Any mandatory
verification for this invocation remains the supervisor's responsibility.

Changed in this follow-up: **this report only**. Its entry copy is retained at
`build/miner-axe-001/revision-20261118T022755/miner-axe-001.entry.md`.
Source, tests, recordings, controls and other incoming edits were preserved.
The live lease still belongs to supervisor PID 34136; Get-Process found no
clone/build runner. No save was accessed and no original execution occurred.

Handoff: existing refusal Axe implementation remains needs_review; criterion 1 is unresolved, and further implementation is blocked on scope.
Next: coordinator must reconcile criterion 1 with the refusal-only trace before redispatch; this worker changed neither the criterion nor the trigger.

**What to check:** **Just Mine / node 5 / MM_MINE**: let a miner reach metal.
On the refused stroke, one installed axe should tumble away while the lemming
turns and walks as before, with the same MineMetal clang and unchanged terrain.
Rotate around the cylinder and use both zooms; scenery/other actors should
occlude the tool. Pause freezes it, fast-forward advances it, and Retry clears
it. Successful mining retains its existing held-pickaxe strokes, without a
second looping tool. Human confirmation of this presentation remains pending.
