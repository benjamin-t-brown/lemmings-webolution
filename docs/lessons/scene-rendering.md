# Scene rendering and integration findings

> Dated historical research excerpts, preserved in their original wording and order within each source range.
> Earlier conclusions may be superseded by later entries; this is not current project status or
> authorization. See [STATUS](../STATUS.md) for current scope and [LESSONS](../LESSONS.md)
> for essential startup guidance. Some excerpts refer to findings that appeared earlier in the
> former combined file; use their linked evidence and the source-range comments below.

<!-- Source lines 754-923 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
**A7a (offline MEASURED, [check](../scratch/phase-a7-wider-mask-localize.md)):** current
tube-only production wider-mask score reproduces A4d: **3031085/137520225 =
0.022041012512886742**, still >0.02. Mask and error reference come from the
durable A2 baseline/suppression pair, not A1 `original-0.png`. Predeclared 16×16
top five cells carry only **25.93%** of positive cell excess; four are in the
lower y=432 band. This falsifies five-cell concentration, not a connected-region
or causal claim. No missing-model attribution or production change. Only initial
has an identified matching wider mask in the checked retained evidence; do not
reuse it for other cameras. Independent full/cell histogram checks, fresh PNG
identity and 24/24 CTest pass. A7 remains open; no live allowance renewed.

**A7b (offline MEASURED, [check](../scratch/phase-a7-lower-band-characterize.md)):**
lower band `[0,432,640,448)` has masked E302804/N3491 (9.99% of global error).
Both original restorations equal baseline there; both suppression images match
each other. The discrepancy is stable in those pairs, but its native cause is
unresolved. Paused-caption composition is a visual lead, not a native ownership
or alpha claim. Ideal removal of every B error gives 2728281/137520225 =
0.0198391255; actual score remains 0.0220410125. Full-frame restores share bbox
and changed count823 but **not** error (11380 vs49411). Initial synthetic exclusion
test was ineffective; repaired actual-helper controls and isolated bypasses are
independently discriminated. Independent image checks and 24/24 CTest pass.
Three-dispatch localization cap reached including repair; cause parked in Phase E.

**A7c (READ negative, [check](../scratch/phase-a7-pause-caption-native.md)):** explicit
fourth-round native exception did not locate the caption renderer. `STR_PAUSED`
is stored at0x4C3C18, with a pointer at0x4C35E4 **offset0**, not+0xC as the
candidate pin prose claimed. This is a key-like identifier, not the display
resource. Known pause/resume stores re-read; one event/state function inspected.
Exact-address xref misses do not prove no indexed consumer. No draw/layout/alpha
rule or composition test accepted. Independent raw pins/bundle authentication
and24/24 CTest pass; re-park after count4. Future bridge must begin at the in-level
draw path, not another pause-event-handler or key-only search.

**A7d (partial READ, [check](../scratch/phase-a7-draw-loop-caption-bridge.md)):**
frame0x441845 passes H=receiver+0x2018 to0x44CD06, which can call0x44D0ED
and base-list draw0x44E150 -> per-element0x414920. One +0x48 flag arm passes
argument432; final screen placement and Paused identity are unproved. Gate
0x4E33C8/nonzero and H+0x50/positive are not connected to pause state1.
The missing edge is **who fills H and selects its text**, not whether a generic
text-to-list/list-to-submit path exists. Timer H+0x224 and index0x44/0x45 records
H+0x758/+0x77C are separate. Do not infer no indexed consumer from absent exact
xrefs or invent BSS table contents. Independent16 pin windows/25 targets/eight
full assembly ranges, authenticated39-record bundle and24/24 CTest pass. No
implementation or gate improvement; re-park after dispatch5 with exact producer
resume condition and no automatic sixth round.

**A7e (READ negative, [check](../scratch/phase-a7-base-list-producer.md)):**
builder call `0x45D27B` does not fill H. `0x45CFF9` entry `ecx` is `[ebp-0x94]`,
and that slot is also the `this` of frame `0x441845`, but the builder's stack
object is `G+0x25C68` with `this=[G+0x5570]` and lookup index `0x54` via
`0x486847`. `0x25C68` is `0x23C50` away from `0x2018`. The call sits on one
case-0 branch that stores `[G+0x572C]=1`; switch case 1 at `0x45D459` does not
call `0x4404FA`. The same arm later calls placement helper `0x44DF40` and
`0x44E010` on `G+0x25C68`, not draw `0x44E150` on H. Index `0x54` text and any
copy from `G+0x25C68` into H are unread. Independent list-immediate mutation
`0x25C68` to `0x2018` fails and restores; 24/24 CTest. Gate unchanged. Do not
repeat `0x45D27B` as the H producer.

**A7f (READ negative, [check](../scratch/phase-a7-caller-dest-offset.md)):**
17 direct `E8` calls to `0x4404FA`, and none has immediate `0x2018` in the
160 bytes before the call. `0x44CD06`'s only direct caller is the frame's
`add ecx, 0x2018`, and its builder calls pass that `this` plus `0x758`,
`0x77C`, or `0x224`. `0x44D348` / `0x44D389` are in `0x44D316`, whose only
direct caller does `add ecx, 0x2018` from `[ebp-0x10]` and then builds at
`this+0x758` / `this+0x77C`. `[ebp-0x10]` is not shown to be the frame
receiver. Control `0x45D25D` remains `add edx, 0x25C68`. A copy from
`G+0x25C68` into H was not searched. Gate unchanged.

**A7g (READ negative, [check](../scratch/phase-a7-25c68-to-h.md)):** ten
`.text` encodings of immediate `0x25C68`, each an `add` then a call or
push. Six are inside `0x45CFF9` and use `[ebp-0x94]`, so they are
`G+0x25C68`. `0x45E33C` calls `0x44E150` on that object. The other four use
`[ebp-0x14]` or `[ebp-0x40]` in other frames; `0x4AD43E` calls `0x449130`,
not the `0x415140` used at `0x45BE4E`. No hit stores the pointer at
`+0x2018`. Independent immediate mutation `0x25C68` to `0x2018` fails and
restores; 24/24 CTest. Gate unchanged.

**A7h (READ, [check](../scratch/phase-a7-pause-list-draw.md)):** the
G+`0x25C68` placement call pushes `[0x58CB44] - 2*metric` first
(`[ebp+0xC]` → `[this+0x18]`) and the `sar` difference second
(`[ebp+8]` → `[this+0x14]`). Neither immediate is `432` or `320`.
`0x44E010` gets dwords `0x3F613404` and `0xEA4A8C15`. `0x44E150` at
`0x45E342` has no stack arguments and runs only when `[G+0x572C] == 1`.
No constant in that trio lies in y `432..447`. The same function stores
`160`, `480`, `180`, and `260` and passes them to `0x47AD8C` on
G+`0x26000` before another `0x44E150`. Gate unchanged.

**A7i (READ, [check](../scratch/phase-a7-rect-26000.md)):** `0x47AD8C`
walks from double `160` toward `480` (`fsub` at `0x47ADAA`). The other
pair, `180` and `260`, is passed through and does not meet y `432..447`
when left unchanged. `0x45E157` can instead add and subtract
`round(760 * ([G+0x25FF8] - 0.5)^2)` (`0x4B1810` is `760.0`) before the
same push, which can move the upper edge through the band. That double
is unread. Gate unchanged.

**A7j (READ, [check](../scratch/phase-a7-25ff8-stores.md)):** three `fstp`
to `+0x25FF8`. `0x45CB39` writes `3.5`. `0x45CBB2` subtracts `2.0` when
`[obj+0x1CB7C] == 0`. `0x45CEB3` subtracts `[obj+0x1C968]`. `0x45CFF9`
calls those functions at `0x45D89D` and `0x45DA48` with `[ebp-0x94]`
before the scale read. A field of `3.5` leaves the rectangle setup
because it is above `1.5`. A field of `1.5` would scale the top edge to
`1020`. The value at the read is still open. Gate unchanged.

**A7k (READ, [check](../scratch/phase-a7-1c968-source.md)):** the value
subtracted at `0x45DA48` is `(double)(int32)[G+0x25C64] / 1000.0`.
`0x4B1758` is `1000.0`, and `edx` at the copy is `[ebp-0x94]`. With the
integer at `0` and `V_init` at `1.5`, the scaled top is `1020`. The
integer is not a constant on this path. Gate unchanged.

**A7l (READ, [check](../scratch/phase-a7-25c64-writers.md)):** two stores
of `[G+0x25C64]`. `0x45D09D` writes the return of `0x4657D1([G+0x557C])`
minus `[G+0x25C60]`. `0x4657D1` calls `[0x4B0220]` and subtracts
`[ecx+8]`. `0x45D96B` stores `100` (`0.1 * 1000.0`) when the dword is
already above `100` or `[G+0x1C9B3]` is nonzero. The pause-frame value
is not a file constant. Gate unchanged.

**A7m (MEASURED negative, [check](../scratch/phase-a7-view-masks.md)):** four
launches, one saved pair. `initial` / `mouse` / `right` share baseline
SHA-256 `b80f4a06442bc14df243490d5eb34083ced83157d9f1c1f42d595b590e2b79f2`.
Fixed ROI versus `original-0.png` is `6117452/45441000 = 0.1346240619704672`.
Production versus that original remains `708159/45441000 = 0.015584142`.
Paused camera doubles `+0x50` and `+0x60` moved from about `-266.944` /
`266.944` to `-338.445` / `338.445`. The fresh wider row
`7444647/84031425 = 0.08859360649899725` is that render on the drifted frame.
Durable caption-excluded score remains `2624993/135798210`. Gate unchanged.
A later pair has to pass the fixed ROI against its own retained original
before the route prologues are patched.

**A7n (MEASURED negative, [check](../scratch/phase-a7-input-view-masks.md)):**
one start, pid 23460. All eight baselines fail ROI `[100,80,280,410)`
against their retained originals. Left is `6476525/45441000 =
0.14252602275478093`; right is `7028355/45441000 = 0.15466990163068595`.
Those two are the prescribed key steps. `canonical/baseline.png` matches
`banner-check-after.png`. `elev_down` and `elev0_neg` are one PNG. The logged
zoom i32 is `control+0x0C`; `0x45CC61` is `mov ecx, [eax+0x10]`. No `C3`
prologue, no wider-mask score, camera-block bytes not saved. Gate unchanged.
Do not repeat this input list.

## A7 checkpoint: amended scene-completeness gate passes

A7n paused too early (about 1 s after entry): the opening camera move had not
settled, so its `canonical` baseline sat a quarter turn from the retained
original. **A7p ([check](../scratch/phase-a7-unpause-view-masks.md))** fixed this
by walking 14 s unpaused before pausing: the settled camera hash is
`7e6e1a8a77c29f860f82dd8cde3f60c9f89ecf3f1352d3c2a6801cc9867d0fff`, and eight of
nine `MM_CLIMB_002` views then pass the wider mask (caption panel removed).
`zoom1` alone failed (`5473596/228864285`) until the user also excluded the ten
skill-panel buttons (x `[18,90)`, object `0x4C9208` stride `0x1B0`, y bands
`[12,44)`…`[372,404)`): with that exclusion all nine views pass
(`0.011169`–`0.019314`).

**A7q ([check](../scratch/phase-a7-second-level-mask.md))** generalized the
no-camera-input case to a second level, `MM_DIG_000`, reusing A7p's capture
path unedited: `2606399/132499530 = 0.019671005625453916`, passing the same
amended mask. This needed reading two camera-block fields the renderer's own
input contract uses but `render_camera_skin.py` only approximates with a fixed
fixture constant: `aspect` at offset 288 and `angle_turn_parameter` at offset
280 of the 360-byte block at `0x4CBFC8` (confirmed against
`tools/harness/fixtures/camera_a1/reference-state.json`). The live row count at
`0x004C6E28` read 96 on both `MM_CLIMB_002` and `MM_DIG_000` — a two-level
observation, not an established level-independence rule.

**The amended A7 gate — nine `MM_CLIMB_002` views plus one second level, with
the pause caption panel `[277,422,363,454)` and the ten skill-panel boxes
excluded, each ≤0.02 — now passes in full**, on the facilitator's own
independent re-score of A7p and A7q. A7 is accepted in `PLAN.md`. Left open,
explicitly out of this gate's scope: camera movement (zoom/rotation/elevation)
on any level but `MM_CLIMB_002`, and whether `world_rows` is level-independent
in general.


<!-- Source lines 1661-1703 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->

**C3ah (mocked controller, [check](../scratch/phase-c3ah-controller-lifecycle.md)): test the actual launcher, not an abstract substitute.** Initial six-test io skeleton rejected; copied actual main/import/receipt branches pass the repaired8-test suite and three separate facilitator fixtures. A termination/wait exception could leave success true; scratch copy now fails closed and the isolated predicate removal is discriminated. Correct helper exports source-defined RECORD_SIZE0x118. The mock-only copy still needs real unique-path/helper bindings and a final deployed-script drycheck; original AD recipe remains rejected, no live approval/start or native observation.

**C3aj (deployment contract, [check](../scratch/phase-c3aj-deployed-controller.md)): the exact new controller bindings are dry-checked, not launched.** Owned corrected helper/unique OUT-ledger-approval paths replace old defaults; size rejects before Popen, only current-owned backups restore after process closure. Actual-main10tests/four independent fixtures and a single guard mutation pass;39/39CTest. No marker/start or actual entry/cleanup observed. Original AD remains rejected; a separate live brief owns the authorized start.

**C3af (conditional billboard chain, [check](../scratch/phase-c3af-billboard-chain.md)): production now carries ancestors and the shared camera/S/P chain,17draws once.** Initial68draw/test rewrite and cancelling focal guess rejected. Independent17/340 C3o fields match raw-reference builder inputs; S-only source mutation fails/restores,39/39CTest. Fixed full frame0.0846922 still fails; C1 gates pass. Binary32 context arithmetic is a reference diagnostic, not byte-proved runtime precision. S identity lowers pixel error, so no native pixel protection/selection-by-score claim. Predicate/dispatch/clip/packing/state remain open.

**C3ak (packing component, [check](../scratch/phase-c3ak-vertex-pack.md)): `packSceneVertex` reproduces explicit final known fields and preserves caller specular.** Independent2077retained+2077synthetic byte cases pass; isolated RHW numerator fails/restores;40/40CTest. No normalization of raw1, no per-byte mask/clamp; signed32 input/error domain is ours. Candidate clipping count is not native. BackdropScene call-site adoption remains a separate task; no packing-caused pixel score accepted yet.

**C3ai (predicate context, [check](../scratch/phase-c3ai-predicate-context.md)): `collectEyes` now includes the actual camera/ancestor/S chain.** All465 per-source C3p decisions match306pass/159skip. Identity context changes136decisions (not465), returns265skips and worsens pixel error5065521; restored40/40CTest. Fresh full frame0.0631375060 still fails, C1 gates pass. Reference-input source integration only; actual admission/cache/dispatch/clip/state remain conditional/open.

**C3al (caret alpha, [check](../scratch/phase-c3al-caret-alpha.md)): retained billboards use conditional normal-constructor flags8 alpha policy, not actor>239.** Own17/17name join and768RGBA cases verify disabled-test/texture-alpha semantics; source-cutoff mutant fails/restores,41/41CTest. Full frame worsens to0.0712348771, not pixel acceptance; C1 gates pass. No effective-device/cache claim or score selection. Source-contract lead: check normalized billboard Z against the shared buffer's declared physical-view units before fitting rendering state/format.

**C3 final one-shot (host boundary, [check](../scratch/phase-c3-pause-one-shot.md)): a fully mocked controller still needs the real host's value representation.** AJ's ten mocks all passed, but actual Task Scheduler StateEnum serialized as numeric4 for Running; the string-only preflight rejected before Popen. Own actual-main numeric4/stringRunning fixtures discriminate that single difference, and separate real PowerShell enumeration confirms numeric JSON versus explicit string. Zero stock starts/attaches/writes; cleanup exact. Do not retry under the exhausted16-attempt run or mistake this for missing method permission/native crash. Normalize/test the actual serialization bridge offline before later execution; pause values and effective format remain unknown.

## Rendering frame map checkpoint (C3at/C3au, offline)

[Frame map](../RENDER_FRAME_MAP.md): under C3u's explicit mode/enable gates the
native conditional spine is primary-slot4 → tube → secondary-slot8. Current
main/BackdropScene emits its whole secondary stream before TubeScene: a known
integration-order mismatch, not an attributed pixel cause. Actual repair4 gate
values/admission/effective state remain unknown; raw diffuse adoption is not full
f32 pack adoption. Initial map omissions were repaired and independently checked
([C3at](../scratch/phase-c3at-frame-map.md)). The chosen pre-wrapper comparison
boundary is conditional, not observed runtime compatibility
([C3au](../scratch/phase-c3au-comparison-boundary.md)). No gate movement or live start.

**C3av ([check](../scratch/phase-c3av-thin-comparison.md)): synthetic recording and
comparison mechanics pass, not native capture feasibility.** Per-draw request/
resource checkpoints are not lifecycle/effective-state history. Independent
checks caught identical incomplete logs passing and a known attribute hidden
by an unknown-only exclusion; both repaired. Nine valid field changes differ,
nine incomplete/out-of-scope controls reject/indeterminate, isolated-X bypass
fails/restores;12/12Python and44/44CTest. C3/C1 gates unchanged,0starts. Native
identity/ABI/attribution/history/coverage/non-interference remain unproved; use
the [separate proposal](../RENDER_CAPTURE_PROPOSAL.md), not an automatic live follow-on.

**C3bc (offline verifier initial/repair1, [check](../scratch/phase-c3bc-verifier-safety.md)): transaction batching is not a demonstrated safe concurrent hook install.** Pinned Windows Gum source disables writable remap, reports RWX support and takes a direct page-update branch without suspending other target threads; the later non-RWX fallback's suspension does not cover it. A target-specific first-redirect safety guarantee remains missing, not an observed torn write/crash. Full callback source does emit FXSAVE/FXRSTOR; do not infer an FPU defect from the isolated pushax comment. Actual postinstall receipts are deferred by the approved protocol, not a prerequisite to an offline parser. Source/layout binding remains conditional. Two attempts spent, no accepted verifier/integration, refusal unchanged; 44/44 CTest, 0 starts, retained C3/C1 gates unchanged.

**C3bd (stage ordering initial/repair1, [check](../scratch/phase-c3bd-stage-order.md)): production now places supported primary before the tube and mixed secondary after it.** Roots59644/91782 reuse C3u's conditional holder association; Entrance/Exit remain explicitly legacy-unattributed, not primary by exclusion. Independent raw-tree partition923/82/16/156mesh records and17secondary billboards matches actual-stage candidate counters; counters are not visible/native draw counts. Own production-order-only mutation fails permanent and separate callback oracles, restores exactly;44/44CTest. Fresh repair4/all10C1 images equal retained baselines: C3 .0635481771 FAIL; C1 .0160864/.0161212 PASS. Thus this is a discriminated SOURCE scheduling correction with no pixel protection/improvement, not native frame acceptance. General admission/model-holder timing/clip/packing/effective state remain open; observer refusal unchanged;0starts.

**C3be (offline stage sensitivity initial/repair1, [check](../scratch/phase-c3be-residual-attribution.md)): largest removal penalty is not the dominant existing error.** Tube suppression worsens rest error most (+28392771), but primary suppression changes pixels carrying10364721/12497635 (82.933459%) of baseline rest error, versus tube1852377 (14.821820%). All removals worsen full-frame error. These are non-additive counterfactual final-RGB supports, not accepted raster coverage, native ownership or a correction. Own mask/arithmetic checks, isolated primary-source guard/revert, exact source restore,44/44CTest and11fresh identical images pass; C3FAIL/C1PASS unchanged,0starts. Per-primary-source accepted-write/final-influence join and matched native admission/state remain unmeasured; stop at2attempts, no automatic Tube split, implementation or observer reopening.


