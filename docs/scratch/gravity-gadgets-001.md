**gravity-gadgets-001: implementation complete; needs_review.**

Playable v1, human playtest 4 T5(a,c), on map node **11**, `m06_kf_1`,
resolved `COMPILED\M06_KF_1.LVL`, **Sometimes up..sometimes down.**
The next dependency was showing the placed gravity gadgets and terminating the
roof escape recorded by the accepted [presentation work](antigrav-presentation-001.md).
That work's acceptance cites 260 main / 84 recovery passes in
`build/agent-runs/loop-74fa1376ca404209855704db4e38cf31/0001-verify-0dcf13/verification.log`;
its earlier pending/failed reports remain historical. This task preserves its
inverted sprite presentation. Difficulty: difficult; recommendation: Sonnet-class,
as assigned; this worker used the explicit Codex override.

These are static reference reads, source inference, source tests and source-built
clone images. No original execution, copied-byte oracle, debugger, hook, original
capture, installation change, subagent or background job was launched. Full
registered main/recovery verification is **pending supervisor execution**.
No acceptance, fidelity gate, queue, verifier, controller, permissions, tool
registry, shared instruction, simulation speed, release rate or upright fall rule
was changed. Pre-existing edits were preserved.

**Reproduction and before/after evidence.**

The route directly loads the installed level, rather than claiming a frontend
unlock or human win. `runApplication --level m06_kf_1 --sim --native --no-audio`
runs for 240 frames at the existing owned 100 ms test clock. Its application
observer reads state without mutating the World. Separate real-World traces use
`advanceElapsedMs(20)` for 24 seconds; these two sampling cadences have slightly
different cell-crossing samples and are labelled separately.

Task-entry source was copied **before production changes**, including the dirty
source already present, to `build/gravity-gadgets-001/before-source/`. The
[SHA-256 manifest](../../build/gravity-gadgets-001/before-sha256.json) covers every
copied source and the entry CMake file. The registered identity test checks it.
The before executable overrides entry `World.cpp`, `BackdropScene.cpp` and
`main.cpp`, using entry headers; unchanged support objects are shared. Neither
the authentic baseline nor historical demonstrations were refreshed from edited
sources. Added presentation types are application-owned; BackdropScene's data
layout is unchanged.

Final paired artifacts and logs are archived in
[`development-evidence/`](../../build/gravity-gadgets-001/development-evidence/),
with [100 file hashes](../../build/gravity-gadgets-001/development-evidence/sha256.json).
Later registered runs can regenerate the separate `before/` and `after/` folders
without replacing that archive.

| Actual clone framebuffer | Before | After |
| --- | --- | --- |
| Upper flipper, placed native `(25,89)` | [absent](../../build/gravity-gadgets-001/development-evidence/before/pad-0-0.png) | [visible](../../build/gravity-gadgets-001/development-evidence/after/pad-0-0.png) |
| Lower flipper, placed native `(69,8)` | [absent](../../build/gravity-gadgets-001/development-evidence/before/pad-1-0.png) | [visible](../../build/gravity-gadgets-001/development-evidence/after/pad-1-0.png) |
| Production application at 24 seconds | [roof escape](../../build/gravity-gadgets-001/development-evidence/before/m06_kf_1/frame-24000.png) | [loss counted](../../build/gravity-gadgets-001/development-evidence/after/m06_kf_1/frame-24000.png) |

I inspected these full frames and the changed-coil sample images. The upper
model hangs below its ledge; the lower model stands on its pad. The two real
models are tagged type 3, IDs 6/7, with `anti_grav.a3d` typed roots at file offsets
837208 / 864351. The terrain retains six gravity cells, Y-down `(24,6)`, `(25,6)`,
`(24,7)`, `(25,7)`, `(68,88)`, `(69,88)`, consistent with the accepted presentation
census. The regression asserts each placed model neighbours its gravity cells.

The completed-scene pixel observer identifies writes by each model's serialized
record offsets, then checks that their RGB survives subsequent terrain/scenery
passes. It finds **1,142** upper and **1,410** lower flipper pixels, at inclusive
screen bounds `(299,66)..(341,102)` and `(298,371)..(338,415)` in the respective
640x480 camera views. Before has zero model submissions/writes. Changed UV samples
change the image; paused samples are identical. Accepted depth writes must move
nearer, and a nearer prefilled scene depth rejects every gadget write. These
checks exercise the existing clone scene-depth policy, not original D3D fidelity.
See [render.csv](../../build/gravity-gadgets-001/development-evidence/after/render.csv).

The failure is not a naturally unsupported ceiling landing: the first actor
already flips, rises and walks under real solid terrain. At 9000 ms it is
`(73,77)`, gravity -1, Walker. Later it falls upward through an open roof column.
`Terrain::at(y<0)` returns Empty, `stepFaller` lacked bounds termination, and the
render adapter clamps the escaped sample to the top cylinder ring. The apparent
stuck actor is a perpetually falling simulation actor.

| Trace | Before | After |
| --- | --- | --- |
| World at 18,940 ms | `(104,-1)`, Faller, distance 34 | `(104,0)`, Splatter, distance 34, loss counted |
| World at 24,000 ms | `(104,-102)`, Faller, still active | `(104,0)`, Dead |
| Application at 19,000 ms | `(104,-1)`, Faller | `(104,0)`, Splatter |
| Application at 20,600 ms | `(104,-33)`, Faller | `(104,0)`, Dead |

The [World trace](../../build/gravity-gadgets-001/development-evidence/after/natural.csv)
records every actor at every sample; the [application trace](../../build/gravity-gadgets-001/development-evidence/after/m06_kf_1/positions.csv)
records actor 0 through the production render loop. All released actors satisfy
`spawned == active + saved + dead`; every active inverted actor stays in bounds.
At 24 seconds the World has four losses instead of the baseline's two, reflecting
two formerly escaped actors. Losses are counted on entering Splatter and are not
counted again when that state becomes Dead.

**Production path and original evidence.**

`runApplicationLeg` loads the real archive through `AssetLibrary`,
`loadRetailLevel` creates terrain/World, and `BackdropScene::build` parses the
serialized gadget tree. Previously extraction included primary/secondary scenery,
hatches, exits, fluids and type-1 levers, omitting type-3 gravity gadgets. It now
adds their placed Object 09 trees to the existing `LegacyUnattributed` stage.
`scheduleSceneFrame` draws that stage with ordinary scenery depth, followed by
tube/fluid/secondary passes and actor composition. No replacement gizmo geometry
or screen overlay was invented.

Each model's retained `anti_grav` auxiliary payload is 8,093 bytes: no transform
tracks, two 100-key **UV** tracks, no events, duration `3.3333332538604736` seconds.
They target material 0 of nodes `(2,4)` and `(1,5)`. They are not colour tracks;
the hatch-only decoder correctly rejects this different form. The new bounded
reader binds original corner UVs once. `advanceScenery` retains its existing
paused/capped clock, then `animateGravity` samples the stored key and adds its
UV offset to those original values. Repeated draws do not accumulate offsets.
The value regression independently checks a V advance of approximately
`0.03028485` at 100 ms. No simulation or gadget-effect clock was added.

The registered reference throughout is `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
x86, image base `0x400000`. REA used Ghidra 12.1.4, profile digest
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Analysis was closed without snapshot arguments. Complete returned Evidence,
including provider limitations, is retained as `build/gravity-gadgets-001/static-*.json`.
Relevant accepted/historical context is
[anti-gravity](../../old/docs/scratch/phase3-round9-antigrav.md),
[inverted steppers](../../old/docs/scratch/phase3-round30-inverted.md),
[fall/splat evidence](../../old/docs/scratch/phase3-round10-steppers.md), and
[the serialized reader](../../decomp-lemmings/docs/evidence/object_base_prefix.md).
No historical execution was repeated.

Static claims cross-checked against instructions:

- `433DD9` dispatches tagged type 3 to constructor `41D2B0`; `41D730` reads its
  gravity/facing fields and obtains `AntiGrav`. The placed model comes from the
  shared serialized base reader. [Dispatch instructions](../../build/gravity-gadgets-001/static-0x433dd9-ins.json),
  [reader instructions](../../build/gravity-gadgets-001/static-0x41d730-ins.json).
- Fall advance `4160BD` subtracts gravity `actor+44` times movement from native
  Y (`416263..41627F`), increments the fall accumulator (`416315..416321`), and
  calls shared landing `416433`. [Instructions](../../build/gravity-gadgets-001/static-0x4160bd-extra.json).
- `416465..41646B` takes the support sample using gravity. `4164CE..41653A`
  checks raw vertical bounds before trampoline and solid support and attaches
  `40B3A0`. `4165FC..41660A` compares fall-state `+18` to the double at `4B10A8`,
  whose retained bytes decode to **30.0**, with a strict greater-than lethal
  branch. Floater byte `actor+49` and matching `0x200`/type 1 or `0x4000`/type 2
  support zones exempt a landing. There is no inverted-gravity exemption.
  [Pseudocode](../../build/gravity-gadgets-001/static-0x416433.json),
  [instructions](../../build/gravity-gadgets-001/static-fall-ins.json),
  [threshold bytes](../../build/gravity-gadgets-001/static-splat-threshold.json).
  Principal IDs: `ev_34a2e470c1ee596182b4c7d27791325753b1b2ea3382c17949c2042b0043f691`
  and `ev_b879fd6905f9c1bc4d2161f2fb124977043543deb48fdc7179e51e908e553e6d`.
- `40B3A0` installs vtable `4B0668`, type `0x1A`; its entry `40B3EC` clears
  actor render-enabled `+28` at `40B4AF` and performs the loss/effect path.
  This is the same state used for an excessive fall. [Constructor instructions](../../build/gravity-gadgets-001/static-0x40b3a0-ins.json),
  [entry instructions](../../build/gravity-gadgets-001/static-splat-enter-ins.json).
- `47CF5C` reads each 40-byte key as a time and two two-component values;
  `476FB0` selects its end-time interval, and `476A80` evaluates base plus
  fraction times delta. `4764C4` copies that result to targeted material fields.
  [Key instructions](../../build/gravity-gadgets-001/static-0x47cf5c-ins.json),
  [selection](../../build/gravity-gadgets-001/static-0x476fb0-ins.json),
  [evaluation](../../build/gravity-gadgets-001/static-0x476a80-ins.json),
  [material stores](../../build/gravity-gadgets-001/static-0x4764c4-ins.json).

`World::stepFaller` retains the shared ceiling landing, strict >30 splat rule,
floater/zone exemptions and budgets. It now terminates an inverted actor leaving
the grid through the existing Splatter/loss path. A narrowly scoped contact
adapter also prevents an inverted Faller supplied already adjacent to a ceiling
from moving through that support. Upright movement/check order is unchanged.

**Provisional choices and limits.**

The clone uses Y-down feet coordinates; the original uses raw native Y with
gravity-dependent `+368` bias. Retained `40DF50` pseudocode also clamps raw Y to
`[0,height-1]`, so the bounds guard alone does not establish the exact original
upper-edge contact sequence. The clone deliberately loses the actor at its first
out-of-grid feet cell, clamps the effect to the edge, and uses its existing splat
lifetime/counter/cue. Immediate inverted support contact is likewise a provisional
feet-grid adapter. These choices are labelled in code; exact native coordinate
conversion and loss-effect duration are not fidelity claims.

Animation uses a shared zero phase, repeats on the existing scenery clock, and
maps serialized node IDs to placed-tree preorder, following the existing hatch
adapter convention. Material UV translation is expanded into corner UVs. Native
per-object start phase, contact-driven sound/activation and exact node binding
have not been observed; the visible, placed, scrolling coils are the bounded
player-facing result. Other animation forms produce a diagnostic. An authorized
original observation of inverted short/long falls and an open roof, recording raw
Y, `+368`, support samples, state/loss/render flags, plus model clock/node/material
offsets on these two pads, would settle these adapters. No such run was made.

Placed-actor regression uses unmodified real terrain, with fixture release/exits
disabled. It finds the actual ceiling at `(0,27)` and tests contact at feet
`(0,28)`, inverted falls of 30/31 cells, floaters, and open roof column 104.
Thirty cells lands; 31 splats unless floating; each lethal case reaches Dead,
counts one loss, and permits `finished()`. This supplements the natural route;
it is not a natural win or full original comparison.

**Checks and preservation.**

- Registered UCRT64 GCC/CMake/CTest; existing `-O2 -g`, assertions retained;
  at most eight compile jobs. [Focused build](../../build/gravity-gadgets-001/development-evidence/final-build.log)
  and [CTest](../../build/gravity-gadgets-001/development-evidence/final-ctest.log):
  **13/13 passed**, 161.75 seconds, covering world, terrain, retaillevel,
  simclock, lever rendering/application, accepted anti-gravity presentation,
  and the new identity/before/after tests.
- The expanded driver adds the independent UV sample and pre-escape physics
  comparison. [Driver CTest](../../build/gravity-gadgets-001/development-evidence/final-driver-ctest.log):
  **3/3 passed**, 53.38 seconds. The subsequent
  [final rebuild](../../build/gravity-gadgets-001/development-evidence/final-rebuild.log)
  passes without warnings after only a comment/whitespace cleanup. No full
  verifier profile was duplicated; supervisor verification remains pending.
- [Comparison](../../build/gravity-gadgets-001/development-evidence/comparison.json):
  `MM_CLIMB_002` and `MM_BASH_002` each retain **160/160 identical whole frames
  and simulation records**, including counts/outcomes. The gravity application
  retains its first **189** simulation frames, through 18.9 seconds, exactly;
  the next frame implements the demonstrated loss. Gadget pixels change from
  the first frame, as intended. Existing assertions/timeouts are unchanged.
- Each test preserves installed and normal clone save presence/full bytes and
  uses its own private `FileSaveSink`; direct routes do not write even that
  private save. Both archived summaries record preservation and zero failures.
  `git diff --check` passes for task files; task text uses LF. Entry-source
  comparison identifies only CMake, BackdropScene cpp/header, main.cpp and
  World.cpp as modified existing files. New files are the task driver, its two
  CMake files and this report. No commit/push or protected control edit was made;
  the parent supervisor lease remained active.

Handoff: gravity-gadgets-001 is needs_review; visible animated flippers and terminating, counted inverted falls are implemented with retained clone evidence and passing focused checks.
Next action: supervisor full registered main/recovery verification and independent acceptance review; human playability and the documented original-coordinate/animation adapters remain unconfirmed.

**Retry 002: correct frozen expectations for the demonstrated invisible gadgets.**

The first supervisor run is now historical **failed verification**, not a pending
or passing run: [verification.log](../../build/agent-runs/20261115T060609-22780-87a907a3/verification.log)
reports **278/280 main tests passed**. Only `scenebranchimpl_after` and
`scenenestedimpl_after` failed, both first reporting `H17_MG_1:scene-on`.
That run does not establish a recovery pass. Full registered main/recovery
verification of this retry remains **pending supervisor execution**.

Both tests compared their authentic frozen scenes with current scenes outside
the accepted lever stores. The new flippers correctly change those scenes too:
`H17_MG_1` has three installed `anti_grav.a3d` models and ten anti-gravity cells,
per [the retained level census](../../build/level-sweep-001/census.md).
The failed view contained 862 changed RGB/depth pixels outside its lever mask.
The same frozen comparisons also cover the flippers on `M06_KF_1` and
`M08_KF_1`; the first failing assertion had prevented reaching them.

Under AGENTS.md's Project lead direction, this retry corrects the **two existing
comparison expectations that encoded invisible flippers**. This is an explicit
exception to the earlier report's statement that existing assertions were
unchanged. `scene_lever_baseline.hpp` retains the existing lever mask and adds a
separate `.gravity-mask`. A gravity pixel is eligible only after an actual
Less/write-on store whose source lies inside a placed `tagged-gravity` model's
serialized tree extent. The two callers require exact RGB and depth equality
outside the union of those two provenance masks. They exclude no level or screen
rectangle. Missing masks fail; scene-off frames still compare exactly. Frozen
sources, expected hashes, source identities, test counts and timeouts are unchanged.
The dedicated gravity driver still requires both installed flippers to produce
visible pixels, animate, pause, and obey occlusion.

The registered build with at most eight jobs succeeded, with only the existing
unused-function/variable warnings in the historical scene drivers:
[build.log](../../build/gravity-gadgets-001/retry-002/build.log).
The [focused CTest run](../../build/gravity-gadgets-001/retry-002/ctest.log)
passed **9/9 in 81.83 seconds**: scene branch/nested identity and before/after
pairs, plus gravity identity and before/after. Each historical scene pair
compares 30 views: **21 match whole-frame RGB/depth exactly**; the remaining
nine differ only at recorded lever/flipper stores, with zero unexpected pixels.
These tests also rebuild scenery for all 102 installed levels and complete their
existing bounded application routes. No full verifier profile was duplicated.

An independent byte audit and its
[preservation/comparison record](../../build/gravity-gadgets-001/retry-002/preservation-and-comparison.json)
confirm that **both frozen and current render hashes are unchanged from the
failed run**. This retry changes the defective expectations, not the rendered
output or production behavior. The new H17 scene-on comparison records 2,681
accepted gravity pixels and 500 accepted lever pixels; all 1,244 changed pixels
are within their union. The natural gravity route again lands on a ceiling,
then loses the escaping actor at 18,940 ms and reaches Dead. Installed and normal
clone save bytes/presence remain unchanged; the non-gravity application routes
still compare all 160 frames and simulation records exactly against task entry.

Retry-entry source backups, the entry diff, and the failed evidence are retained
under [retry-002/](../../build/gravity-gadgets-001/retry-002/). Final paired
gravity outputs and the affected historical scene images/raw buffers/masks are
archived in [evidence/](../../build/gravity-gadgets-001/retry-002/evidence/), with
[SHA-256 hashes](../../build/gravity-gadgets-001/retry-002/evidence-sha256.json).
The retained and regenerated flipper images were inspected: the upper coil hangs
below its ledge and the lower coil stands on its pad. Authentic task-entry sources
still match their original manifest. Production files match retry entry exactly;
this invocation changes only the two scene test drivers, their shared comparison
helper and this report. The parent's RepositoryLease (PID 22780) remained in place.
The audit confirms the queue patch is unchanged from retry entry and the other
protected controls match supervisor entry. No original execution or new static
analysis session was needed; retained evidence and all provisional limits above
remain applicable.

Complete task file set: `CMakeLists.txt`, `src/app/BackdropScene.{cpp,h}`,
`src/app/main.cpp`, `src/sim/World.cpp`, `src/__test__/test_gravity_gadgets.cpp`,
`src/__test__/scene_lever_baseline.hpp`, `src/__test__/test_scenebranch_impl.cpp`,
`src/__test__/test_scenenested_impl.cpp`, `cmake/gravity_gadgets.cmake`,
`cmake/gravity_gadgets_identity.cmake`, and this report. Pre-existing changes in
those files are preserved. Native coordinate conversion, native animation phase
and human playability are still unconfirmed; this retry adds no fidelity claim.

Handoff: gravity-gadgets-001 is needs_review; production fixes and retained evidence stand, and both frozen comparison failures are addressed with 9/9 focused checks passing.
Next action: supervisor full registered main/recovery verification, then independent acceptance review; retain the documented provisional adapters and human-playability limits.
