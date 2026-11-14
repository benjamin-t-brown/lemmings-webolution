# lever-render-001: visible levers and the MM_W_012 initial containers

Status: **needs_review**. Playable-v1 step: human playtest 4, T2.
Difficulty: difficult; assigned recommendation Sonnet-class; this worker uses
the explicit Codex override. Mandatory full main/recovery verification and
independent acceptance remain the supervisor's responsibility.

Levers now draw at their installed placements and change their visible face
when the simulation throws them. The static recheck supports an empty initial
state for the lower MM_W_012 receivers. **No fluid initialization was changed.**
That conclusion remains a static/data finding awaiting the original observation
described below; it is not an observed original frame or a closure of the human
dispute by assertion.

Only source-built clone/test execution and registered-reference static analysis
were used. No original execution, copied instructions, debugger, hooks, original
capture, installation changes, subagents, commits or pushes. Queue, verifier,
controller, tool registry, shared instructions and completion gates were untouched.
The active supervisor owns the repository lease; this worker edited no protected
controls. Get-CimInstance process inspection was denied; Get-Process succeeded
and showed the existing supervisor, without a live compiler or game at entry.

## Resume and provenance

Continued the interrupted task's existing production/test/CMake changes. The
lead's copy under `build/lead/lever-render-001-attempt1/` is preserved; an additional
resume-entry copy is under `build/lever-render-001/resume-entry/`.

The authentic task-entry renderer remains under
`build/lever-render-001/frozen-render/src/app/`: BackdropScene.cpp/.h and main.cpp.
`entry-manifest.json` SHA-256 is
`9ce1f7d5ed6c123c5c769d5ef2ec39916505da7a6a2fe66f562e5c01e17429b4`.
The registered identity test checks the manifest and all three frozen sources.
`resume-baseline-audit.json` additionally confirms all three match HEAD byte for
byte. Resumed modified sources were never substituted for the baseline.

Earlier attempt artifacts include `component-1.log` (an overbroad new geometry
assertion also rejected legitimate rear-camera projection culls), `focused-2.log`
(four checks passed on an earlier source state), and `build-4.log` (interrupted
build). These are historical, not final verification. The lead's interruption
note governs the attempt's low-memory termination; it is not a gameplay failure.
This resume removed an unused sprite-dispatch extension: installed lever knobs
use the existing billboard path, and the new census admits every installed lever
without broadening the separate branch-1 sprite admission policy.

Verification-retry correction (this invocation): the prior supervisor run
`build/agent-runs/20261114T195512-47060-55025ee9/verification.log` built successfully
and passed **252/254 main tests**, including all lever-render tests, but failed
`scenebranchimpl_after` and `scenenestedimpl_after`. Recovery was not reached.
Both failed at H17_MG_1's whole-frame comparison with an old renderer that
omitted levers. This is a failed historical full verification, not a pass.
Entry files and that log are preserved in `build/lever-render-001/retry3-entry/`.
This retry changes only the two scene test drivers, their shared test helper
and this report; all resumed production changes are retained byte-for-byte.
No new static-analysis session or original execution occurred in this retry.

## Production failure, path and implementation

Before: `findLevelObjects` classified placed `lever.a3d` and
`loadRetailLevel -> linkLevelGates -> fanOutLeverLoadFaces` created functional
simulation levers. `BackdropScene::build` extracted paired scenery plus
`entities_425e50/426c6b` and fluid receivers, but omitted `tagged_entities`.
Consequently no lever root reached the level's scene draw. Retained baseline
`before/fixtures.txt` records zero admitted lever roots despite simulation counts
4, 1 and 1 on the three affected image levels.

After: `BackdropScene::build` admits tagged type-1 inline Object 09 models,
labelled `tagged-lever`, into the existing LegacyUnattributed scenery stage.
It retains each serialized root's placed position and orientation, validates
the existing mesh/billboard adapters, and reads the embedded `lever` transform
clip through the existing HatchAnimation reader. LeverPresentation binds that
clip to the serialized ID, starting face and reversal flag. Its binding storage
is application-owned, preserving BackdropScene's layout for frozen renderers.

`runApplicationLeg -> renderFrame` supplies live `(serializedId, state)` faces
from World to `setLeverFaces` before scene submission. The level renderer draws
Primary, LegacyUnattributed (including levers), tube/actors, FluidGate and
Secondary with their shared scene depth buffer. No simulation timing, release,
skill, fluid-height or scene-depth policy was changed. The lever meshes and knob
billboards use the same existing Less/write-on paths as other placed scenery.

The `lever` clip has duration 0.666667 seconds and animates child nodes 2 and 3
in the sampled models; the installed census checks no lever track replaces its
placed root. The exact source offsets, root matrices, keys and quaternion values
are in `{before,after}/measure.txt`. A present unsupported lever model or malformed
clip produces a diagnostic/error rather than a guessed replacement.

Static face contract, checked against instructions:

- `0x436E6A` writes state +1/-1, selects the `lever` clip and direction
  `(face == -1) XOR [+0xA0]`, disables looping, then sends the matching list.
- `0x43703F` applies stored `[+0xDC]`, then calls model vt+0x24.
- Vtable `0x4B1688` resolves +0x24 to **0x45130A**, +0x30 to **0x451509**,
  +0x3C to **0x451649**, +0x40 to **0x4510BF**. `0x465A69` maps reversal
  byte 0 to direction +1, nonzero to -1. `0x45130A` samples duration for
  forward and zero for reverse. This establishes the initial settled endpoint.

**Provisional runtime presentation:** a throw immediately samples its new
settled endpoint. The original advances the selected transform clip; that
interpolation is not connected here. This choice is labelled in production code,
changes no simulation clock, and exposes the correct current face to a player.
It is not original animation-timing fidelity.

## Retained clone images and interaction

All paths in this section are under `build/lever-render-001/`.
Matched before/after scenery frames use the production scene/tube stages and
the same camera, first elapsed update and wave phase. Natural application images
use the ordinary `runApplication --level MM_W_012 --sim --native` path, dummy
SDL presentation and a private save sink.

| Installed level | Before/after image in each side directory | Visible lever pixels changed vs before | Face-pair pixels changed |
| --- | --- | ---: | ---: |
| MM_W_012, Introducing - Water Lemmings, node 8 | `MM_W_012/initial-yaw-160.png` | 356 | lever 2: 106 |
| E16_KF_1, Wood you believe it?, node 31 | `E16_KF_1/initial-yaw-32.png` | 202 | lever 0: 100 |
| MM_ALL_8_001, One Way Ticket, node 22 | `MM_ALL_8_001/initial-yaw-64.png` | 194 | lever 0: 109 |

Each has all eight yaw frames. I visually inspected these three after frames,
the MM_W_012 baseline at yaw 160, and its lever-2 before/thrown pair. The grey
base, angled stem and knob stand on the path; changing face moves the stem/knob
across the base without moving the placed root.

`after/<level>/lever-<index>-{before,thrown}.png` holds **placed-contact fixtures**:
an actor is placed in each real lever AABB, faces the accepted pulling direction,
and advances the actual World contact path. This is not a natural route claim.
MM_W_012's four boxes are (79,60), (72,34), (183,60), (189,35); initial faces
are -1,+1,+1,-1. All four throws change pixels (106,111,106,111 respectively),
and each tested contact changes the actual simulation face.

`app-{before,after}/natural-1-{before,thrown}.png` is the natural hatch-walker
sequence, without actor placement or skills. `world.csv` records the ordinary
route's 610 frames: every actor's position/state, faces, saved/lost counts and
every receiver's request/height compare exactly between frozen and current
applications. At 61 seconds, saved=4 and lost=0; this is regression evidence,
not a complete human playtest or a new claim that the level was won. The initial
and final application images show the initially empty lower tank and its filled
state after the ordinary lever-2 contact. Gate ids 5/6 start at 0 and finish at
24.5; assertions cover those values in both applications.

Depth evidence in `after/fixtures.txt`: every accepted lever fragment assigns a
strictly nearer scene depth. Across eight views, visible/occluded lever-fragment
counts are **2773/1500** for MM_W_012, **735/387** for E16_KF_1 and **582/432**
for MM_ALL_8_001. The tests require both visible and occluded pixels for each
case. For E16_KF_1 yaw 128, all 109 recorded lever pixels are hidden by later
scenery. This supplements inspection of foreground scenery covering the rear
levers; it is clone depth evidence, not original-device-state evidence.

MM_CLIMB_002 and MM_FLOAT have no levers: all **16** matched frames are exactly
pixel-identical. The installed census covers **102 levels / 130 levers**;
every lever model, clip and scene binding is admitted. That census is asset/
admission coverage, not all-level gameplay or all-level visual acceptance.

## Re-derived MM_W_012 container state

Identification: map node 8, third column second down, map centre (275,203), title
**Introducing - Water Lemmings**, archive member `COMPILED\MM_W_012.LVL`.
Accepted asset identity: lemmings.box offset **0x4132306** (68363014), member size
**846980**, FNV-1a64 **0x378dfb7c2f0fe1c0**; patch.box has no replacement.
The new read-only measurement independently decodes this member and the public
loader, and logs all serialized receiver fields and face lists.

Do not equate every 8-cell receiver with a separately built tank: lower groups
span X=[16,64), [96,112), [152,168), [200,248); the upper group spans [120,144).
The per-receiver table below makes the camera-dependent phrase “bottom two
containers” unambiguous without assuming which two visible tanks the human meant.
Every lower group starts empty by the same static rule.

All rows are **kind 0, water**. Q is the member-relative file offset of the
request word. At **Q+4** is kind; **Q+0x28** high; **Q+0x30** serialized current
height; **Q+0x38** low. For lower rows: request=1, high=stored height=24.5,
low=0, bottom Y=4, stored max Y=28.5, loaded height=0. For upper rows 8..10:
request=0, high=stored height=22.5, low=0, bottom Y=59, stored max Y=81.5,
loaded height=22.5. Every X range below is half-open and 8 cells wide.

| Stream index / id | X range | Q | Request | Initial height | Lever id / box / raising face |
| --- | --- | --- | ---: | ---: | --- |
| 0 / 22 | 16..24 | 0xB5EAC | 1 | 0 | 1 / (72,34) / -1 |
| 1 / 21 | 24..32 | 0xB689B | 1 | 0 | 1 / (72,34) / -1 |
| 2 / 20 | 32..40 | 0xB728A | 1 | 0 | 1 / (72,34) / -1 |
| 3 / 19 | 40..48 | 0xB7C79 | 1 | 0 | 1 / (72,34) / -1 |
| 4 / 18 | 48..56 | 0xB8668 | 1 | 0 | 1 / (72,34) / -1 |
| 5 / 9 | 56..64 | 0xB9057 | 1 | 0 | 1 / (72,34) / -1 |
| 6 / 8 | 96..104 | 0xBADCF | 1 | 0 | 0 / (79,60) / +1 |
| 7 / 7 | 104..112 | 0xBB7BE | 1 | 0 | 0 / (79,60) / +1 |
| 8 / 12 | 120..128 | 0xBC1AD | 0 | 22.5 | none |
| 9 / 11 | 128..136 | 0xBCB9C | 0 | 22.5 | none |
| 10 / 10 | 136..144 | 0xBD58B | 0 | 22.5 | none |
| 11 / 6 | 152..160 | 0xBF303 | 1 | 0 | 2 / (183,60) / -1 |
| 12 / 5 | 160..168 | 0xBFCF2 | 1 | 0 | 2 / (183,60) / -1 |
| 13 / 4 | 200..208 | 0xC1A6A | 1 | 0 | 3 / (189,35) / +1 |
| 14 / 17 | 208..216 | 0xC2459 | 1 | 0 | 3 / (189,35) / +1 |
| 15 / 16 | 216..224 | 0xC2E48 | 1 | 0 | 3 / (189,35) / +1 |
| 16 / 15 | 224..232 | 0xC3837 | 1 | 0 | 3 / (189,35) / +1 |
| 17 / 14 | 232..240 | 0xC4226 | 1 | 0 | 3 / (189,35) / +1 |
| 18 / 13 | 240..248 | 0xC4C15 | 1 | 0 | 3 / (189,35) / +1 |

The table's exact raw offsets and decoded triples are retained in
`{before,after}/measure.txt`; `{before,after}/fixtures.txt` asserts and records
the loaded heights after load-face fan-out. The stored 24.5 is real data, not
uninitialized memory, but it is explicitly overwritten at load:

1. Reader **0x42ACAC**, instructions **0x42AE4F..0x42AEEA**, reads request to
   `[+0x18C]`, kind to `[+0x128]`, high to `[+0x98]`, current to `[+0x1A8]`,
   low to `[+0xA0]`. At **0x42AEF6** it compares request with 1.
2. Request=1 branch **0x42AEFF..0x42AF1D** copies low `[+0xA0/+0xA4]` to
   current `[+0x1A8/+0x1AC]`. Kind 0 then clears water-zone bit 0x200 via
   **0x42AF32..0x42AF3D -> 0x43127E**. Otherwise **0x42AF65 onward** copies
   high and sets the zone via **0x42AF98..0x42AFA3 -> 0x4311D0**.
3. The load-face hook runs after receivers exist:
   **0x444417 -> 0x43703F -> 0x436E6A**. The four stored faces
   **-1,+1,+1,-1** each select an **empty** list. The opposite lists send
   only type 4 to the receiver IDs in the table; no MM_W_012 lever sends type 5.
   Thus there is no initial fan-out that fills a lower receiver.
4. Newly re-read receiver post-load hook **0x42B0B8..0x42B1F6** builds spatial
   queries and links another object; it does not replace height or request.
   Its only receiver write in the final branch is `[+0x13C]` at **0x42B1ED**.
5. Accepted initial-update/draw trace in fluid-surface-research-001 section 7.2:
   `0x4423D2 -> 0x42B24D` updates vertices even for dt=0, then
   `0x441845 -> 0x42C5C9` flips the zero-height normal sign. The serialized
   raised mesh therefore does not establish a raised first gameplay frame.

Lever-2's stored face is +1 at member **0xCC43B**; its selected +1 list is empty
at **0xCC463**. The opposite -1 list count is at **0xCC43F**, with type-4
messages to ids 5/6 at **0xCC443 / 0xCC453**. Original receiver message handlers
**0x42C8C0 / 0x42C8E0** write request 0 / 1 respectively. The clone preserves
the accepted initialization and request path:
`loadRetailLevel -> linkLevelGates -> FluidGate::initialise ->
fanOutLeverLoadFaces`; later `World::updateSegment -> stepLeverContacts ->
activateLeverFace -> dispatchLeverMessages -> FluidGate::step` fills it.

Correction/qualification to fluid-kinds-001: its stored-state conclusion is
supported by this independent recheck, but it did not observe the original and
must not be treated as settling the human T2 report. Invisibility of the lever
was a separate confirmed production omission and is repaired here. No speculative
full-at-start override was introduced.

**Observation that would settle the remaining dispute:** in an authorized
interactive original session, open this exact installed MM_W_012 member, inspect
all four lower X groups before any lemming reaches a lever, then observe lever 2
at (183,60) and its [152,168) pool after the first left-facing pull. Record the
initial lever faces, lower-pool surfaces and elapsed point of the first view.
If those lower pools are visibly full before any pull, the static path above is
missing an effective runtime write/dispatch; compare it at that boundary instead
of treating the serialized baked mesh as the answer. No such original run was
performed or scheduled by this worker.

## Static evidence identity and retention

Registered reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
Ghidra 12.1.4 through the project static profile. Both sessions were closed
without snapshots; Ghidra/Java analysis did not execute the target.

Complete returned evidence lives in `build/lever-render-001/static/`:

- Reader `0x42acac-pseudo.json`, `0x0042ACAC-instructions.json`:
  pseudocode ID `ev_93a35b57f9569035d30c03e7be87836f8083eb6797f6908b009ce95e317b8b71`.
- Lever `0x436e6a-{pseudo,instructions}.json`, `0x43703f-pseudo.json`,
  `0x0043703F-instructions.json`, `strings.json`. Load hook ID
  `ev_9e992b29589df7dd11923889767ad22bc87dc1f99f33851f154e0385f612e32f`.
- Receiver hook `0x42b0b8-{pseudo,instructions}.json`: IDs
  `ev_89ee3fbd861ee3853c79ddf43945bcb34ee4336df27710649742ce7465650e42` /
  `ev_f1cca4ac76b8784530f5c8886cbe9b277c47ab4adc5fb3eb2c5a16c5004854b3`.
- Model vtable `model-vtable-0x4b1688.json`; complete pseudocode and instructions
  for `0x004510BF`, `0x0045130A`, `0x00451509`, `0x00465A69`.
  Endpoint pseudocode ID
  `ev_75ef4da2eda72e459dfdb797547a1b6d82f7e408959c875f6f5d65cb2547bc28`.

Accepted evidence reused: lever-links-research-001 (member identity, exact lever
field/list offsets), lever-init-research-001 (reader ordering and exclusive
zone rectangle), lever-initial-face-impl-001 (state plus load fan-out),
lever-gates-impl-001, fluid-surface-research-001 sections 6.2/7.2,
fluid-lever-assessment-001 (earlier natural route), and fluid-kinds-001 (map
identification/kinds). These reports remain unchanged; their historical
limitations and corrections still apply.

## Checks, saves and changed files

CTest registration: `lever_render_identity`, `lever_render_before/after`,
`lever_render_app_before/after` in `cmake/lever_render.cmake`. The measurement,
all-level asset census, image demonstrations, contact fixtures, natural
application route, depth assertions and save comparisons run in those tests.
The after tests depend on their before counterparts; identity precedes baseline
execution. Baseline comparisons are optional only when frozen sources are absent
on another checkout; those sources and comparisons are present in this run.

Existing assertion adjustment, justified by the demonstrated defect:
fluid-kinds-001 required MM_W_012 initial PNGs to equal its frozen renderer,
which omitted every lever. Drawing levers must change those images. The revised
test compares RGBA exactly outside **actual accepted depth-tested writes whose
source offsets belong to the new installed lever model extents**. It does not
mask a projected box, change fluid-height expectations or waive general scenery
differences. The frozen fluid renderer has no lever extents; its baseline stays
authentic. Before RGBA is regenerated through that frozen renderer, alongside
its unchanged PNG evidence. No timeout or existing semantic assertion was
relaxed to hide an unexplained failure.

The same demonstrated defect invalidated two earlier scene tests' whole-frame
hash assertions. Their frozen renderer sources, manifests, counts, admission
checks, sprite/nested dispatch checks and timeouts are unchanged. The drivers
now retain full RGB/depth bytes for all ten compared levels and all three
modes, plus a pixel mask populated **only by actual accepted Less/write-on
stores within `tagged-lever` source extents**. A differing hash requires exact
RGB and depth bytes outside that mask. Levels/modes without lever stores still
require whole-frame equality. The shared implementation is
`src/__test__/scene_lever_baseline.hpp`; no production test switch was added.

Before/after justification, independently recorded by both drivers: **24/30**
RGB/depth hash pairs remain exactly equal. The six changed pairs are scene-on
and scene-only for H17_MG_1, MM_W_012 and VH22_JC. Scene-on RGB/depth changed
pixel counts are respectively **382/382, 251/251, 118/118**; scene-only counts
are **480/500, 463/470, 221/225**. Every changed pixel belongs to an accepted
lever store; **zero unexpected pixels** occur. Their baseline images show the
missing lever geometry; current images restore it. Detailed images, raw bytes,
masks, hashes and `measurements.txt` are retained in
`build/lever-render-001/retry3-scene/{branch,nested}/{before,after}/`.
These are source-renderer comparisons; no original-fidelity claim follows.

Builds use registered UCRT64 GCC 16.1.0, C++23, `-O2 -g`, assertions enabled.
This resume bounded focused build concurrency to four after the documented host
memory interruption; registry/default jobs were not edited.
`resume-build-1.log`, `resume-build-final.log` and `resume-build-final-2.log`
retain the commands' output. The final build adds no warnings in task code;
test_backdropscene's existing narrowing warning is retained.

`resume-focused-1.log`: **8/8 passed**, 140.80 seconds, before the final
additional initial-state/depth/receiver-route assertions.
`resume-focused-final.log`: **12/12 passed**, **152.70 seconds**, on the previous resume's final code:
the five lever-render cases, four fluid-kinds cases, backdropscene, levergates
and leverinitface. Both applications recorded 610 frames, one natural throw,
matching actor/receiver histories and zero failures. I also inspected both
natural-throw frames and the final filled-container application image.
Full registered main/recovery verification is **pending supervisor execution**,
not claimed here.

This retry's registered UCRT64 targeted build is retained as `retry3-build.log`.
It rebuilds only the four affected scene-driver executables with four jobs;
existing unused-function/variable warnings remain. The matching six CTest cases
(two source-identity checks and both before/after driver pairs) pass **6/6**,
**26.68 seconds**, in `retry3-scene-focused.log`. This addresses both failures
from the preceding full main run without repeating the full suite.
`retry3-lever-focused.log`: **12/12 passed**, **153.08 seconds**, covering the
five lever-render tests, four fluid-kinds tests, backdropscene, levergates and
leverinitface. Both ordinary applications again recorded 610 frames, one natural
throw, matching actor/receiver histories and zero failures. All four lever test
`checks.txt` files record unchanged installed/normal saves. Together this retry
completed **18/18 focused tests**; full registered main/recovery verification
and independent acceptance remain pending supervisor execution.
`retry3-production-audit.json` confirms all eight resumed production, CMake,
lever/fluid test files are byte-identical to retry entry; this retry's final
task file identities are in `retry3-final-source-hashes.json`. Earlier manifests
and checks remain historical, with no baseline source replacement.

Every new component test creates no save sink. Application tests inject only
`build/lever-render-001/app-{before,after}/SavedGame.dat`; complete installed and
normal clone save bytes/existence are compared before and after. Each side's
`checks.txt` records both unchanged. The existing fluid application test likewise
uses its private `build/fluid-kinds-001/app/SavedGame.dat`. No import/reset of
normal progress or installed save write was performed.

Task files: `src/app/BackdropScene.{h,cpp}`, `src/app/main.cpp`,
`src/__test__/test_fluid_kinds.cpp`, new `src/__test__/test_lever_render.cpp`,
`src/__test__/test_scenebranch_impl.cpp`, `test_scenenested_impl.cpp`, new
`src/__test__/scene_lever_baseline.hpp`,
new `cmake/lever_render{,_identity}.cmake`, CMakeLists.txt (one include), and
this report. Final source identities are retained separately under the task build
directory. Pre-existing control/log changes remain preserved.

Unknowns: original lower-pool observation remains outstanding; runtime pull
interpolation is provisional; original effective depth/device state, broader
timing, all-level playability and historical fidelity gates remain unchanged.

Handoff: visible placed levers, face updates and the explicitly static MM_W_012 container finding are ready for independent review; task remains needs_review.
Next: supervisor runs full registered main/recovery verification, then reviewer checks the images, scoped fluid/scene baseline corrections and unresolved original observation.
