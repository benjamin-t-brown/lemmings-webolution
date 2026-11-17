# actor-occlusion-002

Status: **needs_review**. Playable v1, [playtest 10 Z1/Z2](../playtest/2026-11-16-playtest-10.md).
Difficulty difficult; recommended Opus; assigned Codex override used.
**Supervisor smoke/recovery verification and independent review are pending.**
The user's next playtest judges appearance; no original-fidelity gate is passed.

The next dependency after accepted basher-occlusion-001 was the remaining round
terrain contact and actor-versus-actor ordering. Its upright scene-occlusion
plane and the revision separating drawing from picking are retained.

Z1, installed **Lems at Loggerheads**, node 17, `MM_E1_3_SKILLS`:
the round obstacles are **sawn log-end terrain**, including the small round log
beside the large circular timber face. They are collision-derived Cylinder
geometry with the level's OUT/INN textures, not a placed obstacle model.
The parsed scene-node census contains background sand/grass/water and the
hatch/exit models, with no separate circular obstacle model.
[Level dump](../../build/actor-occlusion-002/level-dump.txt) retains that census,
the collision mask and stored surface vertices; `dump.cpp` is its static asset
reader. The ledger has no exact camera/contact coordinate, so all three natural
turn sites in the promoted recording were considered, rather than identifying
one circle solely from the prose:

| Clock ms / actor | Simulation column, file row | Drawn sample at turn |
| --- | --- | --- |
| 19,472 / 1 | 119,34 | 119,34, layer .711 |
| 29,864 / 1 | 69,34 | 69,34, layer .25 |
| 42,005 / 0 | 201,17 | 201,17, layer .75 |

Exact passive replay observations: [turns.txt](../../build/actor-occlusion-002/turns.txt).
The lower round log occupies columns **200..205 at rows 18..21**, tapering to
**202..203 at rows 16/23**. Stored key (201,17) maps to inner
`(48.8261,17,10.9531)` and outer `(58.5629,17,13.1385)`; its cells and drawn
contour agree. The existing right-facing turn convention can leave the origin
on the first obstructed column. In addition, a wide upright sprite plane can
extend into neighbouring solid cells while remaining in front of their curved
skin. A scene-Z comparison alone then admits intersecting body pixels.

`prepareInjectedWalker -> drawActorSprite -> clipBodyFootprint` now maps each
opaque body texel's upright-plane ray intersection back through the polygonal
cylinder template into collision column, row and radial layer. Pixels inside
solid/door terrain are excluded before the existing scene/pipe mask. This uses
the current `syncTerrain` grid, including skill edits. Zone/fluid/blocker flags
alone do not count as opaque terrain. SPR placement, actor anchors, simulation,
terrain geometry and hover/click rectangles are unchanged. This footprint rule
is **provisional clone presentation**, not recovered original sprite geometry.

Z2 production path:
`runApplicationLeg::renderFrame -> syncTerrain -> scheduleSceneFrame`
(complete scenery/terrain/fluid) `-> drawInjectedWalkers -> prepareInjectedWalker`
`-> clipBodyFootprint / clipActorToScene -> ActorDepth::submit -> composition`
`-> drawDeathParticles`. Previously reverse creation order overwrote opaque
actor pixels without comparing actors' depths. Now each opaque pixel competes
in **a separate actor coverage buffer**: smaller physical view Z wins; exact
ties go to the lowest stable submission index. It never changes scenery Z.
For bodies, Z is the accepted upright-plane pixel intersection. These planes
are parallel, so their horizontal camera distance gives a consistent far-to-near
submission order at every yaw, including two lanes at one column. It also keeps
the existing additive drowning blend after bodies behind it. Radius alone is
not the ordering key. Transparent texels claim no coverage; frame clear and a
new actor pass discard old coverage.

The builder's carried mesh retains its actual scratch-raster Z against other
actors; its hands/body overwrite their own attachment as before. Its existing
body-plane scene mask is retained. BITS sprites, death chips and skill debris
test/write the same actor coverage after their scene mask. The old 0.64-world-unit
chip foreground pull is removed for consistent physical depth. Bubbles also
use actor coverage. Highlight checks coverage ownership, so a selected rear
figure cannot shine through a nearer actor; `PlacementOnly` picking is untouched.

Static authority was reread from accepted research, not rerun:
registered `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Retained Ghidra 12.1.4 packets are copied unchanged under
[static/](../../build/actor-occlusion-002/static/). Instructions cross-check
pseudocode at `43CC94/43CC9A` (origin sign), `43CCB5` (subtract 0.0003),
`43CCCA` (depth slot), `43CD4E` (sprite submit). Instruction evidence
`ev_9a8209aba409c9745a27529fbccfc174c42f9a83172b48c6c3544c73701ba1e3`.
The retained material-state report does not establish inherited hardware Z
state. No new REA session, original execution, debugger, hooks or capture ran.

Checks: registered UCRT64 GCC 16.1.0/CMake/Ninja, `-O2 -g`, assertions kept,
eight build jobs. [build-3.log](../../build/actor-occlusion-002/build-3.log)
succeeds. [focused-3.log](../../build/actor-occlusion-002/focused-3.log) and
[JUnit](../../build/actor-occlusion-002/focused-3.xml): **5/5 pass**, 6.03 seconds:
unchanged `actorcompose`, `clicktoassign`, `levelvisibility`, `builder_brick`,
and new legacy `actor_depth` (1.78 seconds). The latter checks inverse rays,
7,680 independent polygonal cell-membership samples, zone exclusions, depth
ties/reset, **96 views / 2,873 overlapping pixels** with three actors and both
lanes, list reversal, carried-mesh order, and particle rejection by actor coverage.
These are supplemental logic/pose checks, not new gameplay routes.

Legacy removal: `actor_occlusion_plane` required a terrain-intersecting body to
be pixel-identical to an unmasked sprite against an artificial rear plane;
the new footprint intentionally rejects those pixels. Its failed initial run
is retained in `focused-initial.log`; registration and source were removed under
Verification tiers, without changing its assertions. Smoke registrations remain
**95**; no smoke assertion, timeout, tier list or verifier changed.

All 16 promoted human recordings were run sequentially with
`webolution --game-dir "Lemmings Revolution" --replay <file>` on the final build.
[replays-final/results.json](../../build/actor-occlusion-002/replays-final/results.json)
retains exact filenames, arguments, exits and individual logs:

| Level | Result |
| --- | --- |
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
| MM_BASH_DIG_001_A | PASS 20/10 |
| MM_BUILD_BLOCK_A | PASS 15/10 |
| M06_KF_1-1794811410542247000-12 | DIVERGED step 175 |
| MM_FLOAT_CLIMB2-1794812527275449000-2 | DIVERGED step 163 |

The 14 passes have exit 0, won=1, completed=1. The two exit-1 divergences are
unchanged from accepted [multi-hatch-simultaneous-001](multi-hatch-simultaneous-001.md):
first simultaneous release at clock 5,836 ms expects one spawn, observes two.
They precede this task and are permitted by the assigned replay policy. No
recording or replay check was edited, renamed or weakened; the lead owns retirement.

Limitations: original effective Z state, exact original actor volume and device
bias remain unresolved. The footprint is an exclusion mask, not a displacement
of a simulation origin that enters an obstruction: a fully covered figure can
disappear behind the round log from some views. Supplemental final clone crops
(`contact-x*.png`, 54 views at three contacts/two lanes) retain that behaviour;
the user's playtest must judge whether contact remains readable. No blanket
visibility or full-frame fidelity claim is made. Additive effects keep the
existing provisional art/blend policy; this is not general transparency sorting.
Full smoke/recovery is pending supervisor execution; no complete profile ran here.

Changed: `src/app/{ActorDepth.h,ActorOcclusion.h,Raster.h,Raster.cpp,TubeScene.h,TubeScene.cpp}`,
new `src/__test__/test_actor_depth.cpp`, removed `test_actor_occlusion_plane.cpp`,
the corresponding legacy CMake registration, and this report. Incoming edits
are preserved; [entry/](../../build/actor-occlusion-002/entry/) and
[task-only.patch](../../build/actor-occlusion-002/task-only.patch) separate this task.
The existing supervisor PID **40668** owns RepositoryLease. Get-Process found
no clone/compiler/test runner at entry or handoff; CIM metadata was denied and
was not retried. No protected controls/shared instructions, saves, commits or
pushes changed. Clone commands used private `build/actor-occlusion-002/appdata`;
replays/asset readers create no SaveStore. Task text is LF; scoped diff checks pass.

Handoff: Z1 solid-footprint exclusion and Z2 shared actor/effect coverage are implemented; 5 focused checks pass, 14 replays pass, 2 previously accepted divergences persist; needs_review.
Handoff: supervisor smoke/recovery and independent review next, then human contact/rotation playtest; original depth fidelity remains open.

What to check: **Lems at Loggerheads (node 17)** — let walkers turn at the round
log ends and large circular timber face; rotate through both sides, including
the lower log contact: bodies must not paint inside the timber, exposed parts
remain visible, and hover/click boxes remain usable. Watch overlapping inner/
outer-lane groups while rotating and paused: the nearer figure wins without
flicker. **Just Build / Just Bash / Just Bomb** — overlap a builder's carried
brick, excavation debris and death pieces with walkers; nearer coverage wins
and hidden highlights do not show through the front actor.
