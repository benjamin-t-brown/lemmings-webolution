# Actor rendering findings

> Dated historical research excerpts, preserved in their original wording and order within each source range.
> Earlier conclusions may be superseded by later entries; this is not current project status or
> authorization. See [STATUS](../STATUS.md) for current scope and [LESSONS](../LESSONS.md)
> for essential startup guidance. Some excerpts refer to findings that appeared earlier in the
> former combined file; use their linked evidence and the source-range comments below.

<!-- Source lines 331-753 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## A4f checkpoint: actor billboard geometry exact; pixels fail on vertex tint

The [A4f check](../scratch/phase-a4-actor-billboard.md) adds `src/app/ActorSprite`. For the
two retained mirrored fixtures, destination corners match native within one binary32
ULP. Rule from `0x47A75D`: Y minus `unknown1` at full scale before the branch; mirror
byte `ctrl+0x1B4` selects the branch; the mirrored branch rescales `unknown1`, the
unmirrored one never does (an asymmetry to preserve). Scale delta is the global
`[0x58D53C] − [0x58CB38]`. `ctrl+0x1B0` reorders UV arguments and is not modelled.
The unmirrored branch is READ only.

Pixels are **not** accepted: candidate MAE is worse than an absent-actor render
(five 0.00797 vs 0.00619; eight 0.00934 vs 0.00685). `0x440A10` initialises vertex
RGB to 255.0, but the live actor holds 152.0 in all four records; the writer is
unfound. A comparison-only 152/255 scale beats absent-actor on both fixtures, a U flip
does not. Do not hard-code 152. 20/20 tests; nine tube views unchanged.

**A4g narrowing** ([check](../scratch/phase-a4-actor-tint-writer.md)): the 152 records sit
on the controller `actor+0x128`, not the draw object `actor+0x78`. `0x406260` is the
uniform-RGB setter. Its one computed caller, `0x404434`, is the type-`0x23` **fluid**
sinking fade `(Y − [lvl+0x50]) / [lvl+0x98]`, clamped to 1, ×255, so it does not apply to
walk/block. `0x40E9EC` gives prototype controllers `16·i` (never 152). A4h reads `0x4682a6`/`0x455ba0`
as copying `actor+0x128` **into** eight prototypes (`rep movsd`, `0x1B0` bytes, `i < 8`),
so the actor is the source, never the target. `0x454AE2` is a second copy-out that is
tinted by a constant-255 global. A4i/A4j's GDB hardware watches gave no writer, and A4j
caught one missing a polled 255 → 0 change. A4i's "not rewritten" is therefore
**retracted to inconclusive**. A4k (Frida, measured): four walker controllers built by the
spawn chain (`0x4409E0` from `0x468118`, 255.0) later hold 152.0, so the 152 is written
after construction. Its writer is still unnamed (A4k's script bugs are recorded in the note).
A4l puts the flip within 1–16 ms of construction, and it is not via `0x406260` or `0x455ba0`.
In A4m, walker controllers come through the lemming-constructor stack (`0x43CE39` ←
`0x42DECC` ← `0x425C35` ← `0x442D44` ← `0x45DCD9`). The 17 level-load constructions
(`0x47B35B` …) are a separate stack, so a backtrace taken from the first spawn is not a
walker's. A4n (measured bracket, three walkers): the lemming constructor `0x42DE51` returns
with 255. The flip comes before the spawner `0x425A95` reaches `0x425CE1`, which leaves the
virtual call `[vtable+0x14]` at `0x425CCC`: the new lemming's first update, read as
`0x42E2BF` via vtable `0x4B10E8`. **A4o (measured + read): the writer is `0x403820`**, which
copies one RGB-double triple into all four vertex records (`+0x178 → +0x110 → +0xA8 →
+0x40`). It goes 255 → 152/152/152 on the first update. The source is
`0x403F40(0x4C6A10, ftol(actor+0x08), ftol(actor+0x10))`:
`palette[0x4C6A10+0x678 + 0x18·map[(b<<8)/2 + (a&0xFF)/2]]`, with the map at
`[0x4C6A10+0x670]`. **The lemming tint is a position-indexed colour map**, not a
per-action constant. **A4p (measured):** the map is the stored `u32 12288, u8[12288]` block
after the collision grid (`MM_CLIMB_002.LVL` offset 338319). A 6,144-byte palette of 256
× 3 f64 follows with no prefix, at 350607. Both are byte-identical live. `MM_CLIMB_002`
uses entries 0–12, a grey ramp starting 152, 147, 140 …. In 20/20 samples the lookup
(`idx = y·128 + (x & 0xFF)/2`, x = `actor+0x08`, y = `actor+0x10`) equals the vertex RGB,
but every sample was on byte 0, so a colour change between cells is not yet observed.
**A4q:** `src/assets/LightMap` implements it, and the A4f pixel gate passes (0.00323 /
0.00362, both below absent-actor). Every palette double in all 102 levels is a whole
number in 0–255.

**A4r (READ, [check](../scratch/phase-a4-actor-render-state.md)):** material updater
`0x47261F` writes stage-0 `MODULATE(TEXTURE, DIFFUSE)` RGB on all three paths that
write material states. Pointer and flags caches can skip those writes. Alpha is
texture-only or texture-times-diffuse depending on ordered flag branches; bit 8
and bit 0x10 return before the low-bit tests. Blend routine `0x414D80` separately
selects disabled, ONE/ONE, SRCALPHA/ONE, or SRCALPHA/INVSRCALPHA. Device3 API names
are a strong layout/call-shape inference; the actor's flags and selector remain
unresolved. This supports A4q's RGB rule without accepting actor-specific alpha,
blend state, effective device state, or the sprite residual. Independent checker
`tools/harness/verify_actor_render_state_a4r.py` pins operands, branch exits and SDK
definitions. No production behavior changed; 20/20 tests pass.

**A4s (retained observation + READ, [check](../scratch/phase-a4-actor-material-binding.md)):**
the two A4f controllers at `actor+0x128` are the submit objects consumed by
`0x414920`. Both recorded first dwords are selector **0**; their selected
direction records supply resource handles 119/120 (session-local). Selector zero
encodes blend-disable on a cache miss, not proof of effective device state.
Resolved material `+0x24` flags remain unrecorded in the checked packets, so the
actor's material/alpha branch remains unresolved. BSS placement does **not** prove
the flags cannot be derived statically. No production change or new pixel claim;
independent fixture/byte checks and 20/20 CTest pass.

**A4t (MEASURED + READ, [check](../scratch/phase-a4-actor-material-flags.md)):** live on
`MM_CLIMB_002`, the material descriptor `[[handle·0x88 + [0x58CB84] + 0x80]] + 0x24`
(manager object `0x58CB48`, pool at `+0x30`) holds flags **`0x2`** for both lemming
sheets read, `$LemSheetHi15` and `$LemSheetHi12`. Five walkers and a controlled
eight fixture all hit those two descriptors. In A4r's order, `0x2` selects
**row 3**: alpha `SELECTARG1(TEXTURE)`, alpha test GREATER against
`[0x4C24E0]` = `0xA0`. Selector is 0, which encodes blend-disable. This is the
encoded path on a flags-cache miss, not effective device state. Other
actions/levels and the raster residual are not covered. A worker's "session 0
blocks scheduled tasks" diagnosis was wrong: the session-1 task worked, and the
defect was dereferencing the embedded `actor+0x128`.

**A4u (code + offline MEASURED, [check](../scratch/phase-a4-actor-alpha-test.md)):**
`drawActorSprite` now applies row 3. A texel is drawn only when its alpha is greater than
`0xA0`. A drawn texel is written opaque, with no blend. The reference load is reread at
`0x472896`, and the state pushes `0xF`/`0x18`/`0x19` at `0x472880..0x4728C2`. The test is
**pixel-invisible** on both A4f fixtures: their source frames are strictly 0/255 alpha,
and the gate MAE and composed hashes equal A4q's. Only the synthetic unit test
discriminates `>` from `>=`. The five fixture's frame is on `$LEMSHEETHI13`, whose flags
A4t did not measure (A4t read Hi15 and Hi12), so row 3 there is assumed.

**A4v (code + offline MEASURED, [check](../scratch/phase-a4-actor-support-residual.md)):**
the ~0.08 support residual was a **half-pixel convention**, not filtering. Native matches
an integer pixel centre: cover `left <= x < right`, UV `(x − left)/(right − left)`, nearest
texel. Footprint now equals native on both fixtures; support MAE 0.0819 → 0.00143 and
0.0778 → 0.00152. It is a two-fixture fit, not a byte. `0x470F29` writes alpha reference
`0xEF` then `0xA0` only on its LINEAR filter branch, and A4t read `0xA0` live, so the
filter in force at actor draw is **untraced** and contradicts the nearest-texel fit. The
gate passes under each reverted part; only the unit test protects them, and the first V
test could not see V.

**A4w (MEASURED live, [check](../scratch/phase-a4-actor-hi13-flags.md)):** `$LemSheetHi13`
descriptor `+0x24` = **`0x2`** on `MM_CLIMB_002`, read through a bound actor with no
measurement writes (level entry uses the restored `enter_fast.py` swap). Every `$LemSheetHi0..23` entry in the 139-entry pool reads `0x2`, so row 3
holds for all lemming sheets on this level (encoded flags; device state still unread).

**A4x (READ + MEASURED live, [check](../scratch/phase-a4-actor-filter-state.md)):** the
actor submit `0x47A75D` brackets its draw with `0x470F29(0)` before (`0x47A776`, when
`[0x58D53C]` == 0, which it read live) and `0x470F29(1)` after (`0x47AB8B`). So actors
draw under **POINT** filtering with alpha reference **`0xEF`**. The `0xA0` A4t and A4w read
is the between-draw value (live: `0xEF` in 0.43% of 1 kHz samples). `dgVoodoo.conf`
filtering is `appdriven`. **Lesson: a global read between draws is not the state at
draw**; look for a bracket around the consumer before trusting a sampled render global.
A4u's `> 0xA0` should be `> 0xEF` (pixel-invisible on both fixtures).

**A4y (code + offline MEASURED, [check](../scratch/phase-a4-actor-alpha-reference.md)):**
the actor alpha reference is now **`0xEF`**, with strict `>`. Destination-pixel tests
reject `0xA1` and `0xEF`, accept `0xF0`, and fail under either the old `0xA0` constant
or an isolated `>=` mutation. The facilitator's separate exhaustive raster checker
passes all 256 alpha values; both A4f gate results and image hashes equal A4v's
exactly (binary-alpha fixtures cannot see the correction). Fresh 20/20 CTest pass;
no new live measurement, effective-state claim or app actor integration. A4x's
scope limits and A2/A3's broader failures stand. Next queued action: A5m map.

**A5m (repository map, [check](../scratch/phase-a5m-actor-integration-map.md)):** the
app still draws the tube only; helpers exist but no World-to-actor draw chain.
Do not pass terrain x/y straight as 3D world coordinates or treat `Lemming::frame`
as a universal temporal index: the drowning path adds milliseconds, unlike the
general frame increment. World-pose conversion, orientation basis/phase/prior
mirror, animation progression and shared tube depth remain adapter prerequisites.
No new original-game measurement or implementation landed. Next queued: A4z,
whose environment-specific brief needs adapting before a local Windows dispatch.

**A4z (live capture + offline replay, [check](../scratch/phase-a4-actor-third-fixture.md)):**
the new half-scale sample passes (support MAE 0.001380834), but full-scale zoom1
fails even after matched-camera replay: 27 extra candidate pixels and support
MAE 0.0284529 > 0.0151558. This does not isolate the pixel-centre convention as
the cause. **A4x's POINT/EF claim is zero-zoom scoped:** `0x47A766 833d3cd5580000`
and `0x47A76D 750c` skip that setter at nonzero zoom. Inherited zoom1
filter/alpha state remains open; do not fix it by fitting pixels. Own recompute,
byte pins, replay and 20/20 CTest pass; source/install unchanged and process
stopped. enter_fast's raw level-slot restoration bytes were not retained, so
the live ledger is not fully byte-audited. Next: zoom1 filter/alpha provenance.

**A4aa (READ, [check](../scratch/phase-a4-zoom1-filter-provenance.md)):** no unique
pre-zoom1-actor assignment was established within the bounded static trace.
The explicit frame/tube bodies have no immediate 0x470F29 call, but unresolved
subdraws and unexamined descendants are not certified clean. The setter chooses
local 1/EF or conditional 2/A0 **before** requesting the two filter states; it
does not necessarily send POINT then LINEAR. Global ALPHAREF source and cached
device ALPHAREF are different evidence. Own 38 byte pins, independent A4r checker
and 20/20 CTest pass. Nearest-prologue heuristic graphs/literal-address scans do
not prove transitive or computed-writer absence. Next: actor-correlated zoom1
filter-request observation; no production raster fix or effective-state claim.

**A4ab (failed live experiment, [check](../scratch/phase-a4-zoom1-filter-trace-failed.md)):**
the five-hook run crashed in D3DImm, with zero bound draws/request tuples retained.
No hook-specific cause was isolated. HOOK_INSTALL records were emitted before
attach; they are announcements, not success proof. Buffered unpersisted callbacks
mean an empty trace is not proof callbacks never ran. Own mock reproduction finds
Number.add in the actor binder; that is a dormant harness defect, not a native
crash diagnosis. Mid-call esp[0] is the device argument, not a return address.
This run's raw slot ledger and install hashes match; live hook-code restoration
readback was not achieved. Own cleanup, hashes and 20/20 CTest pass; no filter or
pixel acceptance. Next: offline binder repair before any new stock attempt.

**A4ac (offline harness regression, [check](../scratch/phase-a4-actor-binder-offline.md)):**
`hooks_a4ac_candidate.js` repairs four dormant binder bugs while keeping
`hooks_a4ab.js` byte-identical: (1) converts scalar `arrAddr` with `ptr(arrAddr)`
and guards slot reads via `u32(arr.add(i*4))`; (2) returns `{ actors, match }` on
match instead of returning `match` directly (which lacked `.match` and caused
`DRAW` `if (!snap.match)` to misclassify matches as `BIND_MISS`); (3) uses
`ptr(...).add(0x128).toString()` for `selection.ctrl` instead of `hexN(ptr(...))`
(which coerced a pointer object to `'0x0'`); and (4) binds only on `actor+0x128`,
rejecting the unmeasured `action_obj+0xb0` alias. Four isolated single-line
mutations fail the `node:vm` suite and restore byte-exact; 20/20 CTest pass.

**A4ad (offline harness regression + READ, [check](../scratch/phase-a4-hook-stack-announce-offline.md)):**
`hooks_a4ad_candidate.js` (`1445dad5...`) repairs the two non-binder trace-script
defects from A4ab while keeping `hooks_a4ab.js` and `hooks_a4ac_candidate.js`
byte-identical: (1) `attachWithAnnounce` calls `Interceptor.attach` before
emitting `HOOK_INSTALL` and emits `HOOK_ATTACH_FAIL` on throw; (2) mid-call
`0x470F86` (`ff90a0000000`) and `0x470FA3` (`ff91a0000000`) record `retaddr: null`,
`stack_convention: 'pre_call_stdcall_4'`, `device` (`[esp+0]`), `stage` (`[esp+4]`),
`state` (`[esp+8]`) and `value` (`[esp+12]`) while preserving function-entry
`retaddr` at `0x470F29` (`55 8bec`). Full 134-byte PE read of `0x470F29..0x470FAE`
(including `0x470F8F 51 push ecx`) and four isolated single-feature mutations
under `node:vm` pass; 20/20 CTest pass. Next: static provenance of `0x437BA3`
(`0x441879`) and the indirect tube subdraws (`0x43A9BB`, `0x43AA47`).

**A4ae (READ, [check](../scratch/phase-a4-pre-actor-subcalls-static.md)):**
`0x437BA3` (`0x441879`, `ecx = 0x4CBB48`) forwards `ecx+0x10` to `0x47496D`, which
pushes/pops matrix stack `0x58D5A0` (`0x45FDB0`/`0x43BBE0`), swaps `[0x58CE24]`,
and dispatches `0x4749A7: call [edx+4]`; its depth-4 `E8` tree has zero direct
`0x470F29` calls and zero `0x4C24E0` references. Auditing all 5 `.text` call
sites of `0x43B517` (`0x42A241`, `0x42E153`, `0x42FCD1`, `0x43212A`, `0x43231D`)
corrects A4x/A4aa's draw-site assumption: **lemmings (`vtable 0x4B10E8`,
`+0x10 = 0x42EEF2`) are pushed into `0x4C6A10+0x1E88` (`0x4C8898`) and drawn
inside `0x43A4EC` at `0x43A9BB`/`0x43AA47`**, whereas `0x4C6A08+0x1F70` (`0x44190D`)
holds `0x2A8`-byte `0x4B1020` objects (`+0x10 = 0x42C5C9`). Thus the downward
`0x4C8898` walk inside `0x43A4EC` dispatches `0x42C5C9` (`0x4B1020`), `0x42EEF2`
(`0x4B10E8`), and `0x4323EB` (`0x4B0E00`) before `0x44190D` runs.
Narrowed worker `find_function_prologue` heuristic (`0x42A0EB`/`0x42FBDB` false
mid-function `55 8b ec` matches and `+0x340` subobject `0x415FA0`). Next: static
audit of `0x42C5C9`, `0x4323EB`, and `0x42EEF2` (`A4af`).

**A4af (READ, [check](../scratch/phase-a4-tube-child-renderers-filter-static.md)):**
`0x42EEF2` (`Lemming::Render`) dispatches `lemming+0x78` (`vtable 0x4B14A8+4 =
0x43CC68`) at `0x42F0DC`, which adds `0xB0` (`ecx = lemming+0x128`) and calls
`0x47A75D` at `0x43CD4E` (one of only 2 `.text` callers of `0x47A75D`, the other
being HUD loop `0x45B784`); `0x42C5C9` tail-calls `0x4323EB` (`0x42C709`), and
neither calls `0x470F29`. Across the entire `.text` section, `0x470F29` is the
**only** function that requests stage-0 `MAGFILTER`/`MINFILTER` (`0x10`/`0x11`,
out of 24 `+0xA0` sites) or writes `[0x4C24E0]` (out of 3 `0x4C24E0` sites), has
0 literal pointer occurrences in the PE, and has 10 `E8` call sites: 2 init/option
sites (`0x47022B`, `0x470F1E`) and 4 steady-state `(entry, exit)` brackets
(`0x44E165..0x44E1A5`, `0x454C89..0x454CAF`, `0x45B6D9..0x45B706`,
`0x47A776..0x47AB8B`, all `ecx = 0x58CD80`), every one of which restores
`0x470F29(1)` on exit. Backward CFG reachability on `[0x47A75D, 0x47AB96)`
(narrowing worker `fall_through()` on `call`) proves 0 instructions before
`0x47AB84` can reach `0x47AB93` (`ret 0x10`) without executing `0x47AB84..0x47AB8B`
(`0x470F29(1)`). Thus when `[0x58D53C] != 0` (zoom1), `0x47A75D` skips
`0x47A776: call 0x470f29(0)` and runs under the preceding `0x470F29(1)` state
(`MAGFILTER=2, MINFILTER=2`, `[0x4C24E0]=0xA0` when `[0x58CD80+0x3A1] != 0`).
Next: offline replay of retained A4z zoom1 fixture under `{POINT, LINEAR} x
{0xEF, 0xA0}` (`A4ag`).

**A6a (READ + MEASURED, [check](../scratch/phase-a6-actor-pose.md)):** model
`+0x108` is `0x43D56C` bilinear interpolation of the cell mesh at actor `+0x08`.
X is bucketed by 8 and wrapped `& 0x1F`, Y is bucketed by 16, and Z blends the
inner/outer shells. Missing `grid0` keys are filled from the cylinder template by
`0x43A31D` -> `0x43D2B8`. The basis is the normalized difference after adding
`facing * 0.1` to X (`0x42EF38`). Three live walkers on `MM_CLIMB_002` match the
position within 1e-4. The basis misses the 1e-6 falsifier (max 2.962e-5; native
values are float32) and is parked. **Do not invent missing outer vertices**:
the first packet's radial scaling fit to 3.7e-7 without an address. Sim -> `+0x08`
(sub-cell X, Y flip, Z slab) is unsettled.

**A6b (READ + MEASURED, [check](../scratch/phase-a6-actor-anim-clock.md)):** controller
`+0x1CC` advances in `0x468833` by `acc += elapsedSeconds * [strip+0x10]`, then
while `acc > 0.06` increments the index and **subtracts** 0.06 (remainder
carried). `0x468AB8` resets index and accumulator on strip bind. `+0x201` set
wraps to 0; clear holds at count-1. A 45-sample walker series is integer
milliseconds (6–9) per interval; a no-resync forward replay matches. **Do not
use `Lemming::frame` as the SPR temporal index.** Immediate `param_1` is
`min(remaining budget, time-to-next-cell / 5.0)` from `0x41BDD4`; the outer
budget writer is unsettled. The first packet's 0.2 s/tick prediction is
falsified.

**A6c (READ, [check](../scratch/phase-a6-actor-tube-composition.md)):** actors in
`0x4C6A10+0x1E88` draw **after** INN and all three OUT passes, walking the
list **high index to 0**. `[elem+0x18] >= 1.0` uses `0x43A9BB`; `< 1.0` uses
`0x43AA47` (A4ae; the A6c packet reversed this). No executable
`SetRenderState` for ZENABLE/ZWRITE/ZFUNC/ZBIAS. Per-pixel actor Z versus
tube Z is unsettled.

**A6d (repo MEASURED, [check](../scratch/phase-a6-injected-walkers.md)):** the
production *route* can overlay three injected A6a walkers after the tube
(pose → A3 view / walk temporal 0 → sprite). Empty list keeps the A4e
initial PNG hash.

**A6d2 (MEASURED negative, [check](../scratch/phase-a6-app-bbox-gate.md)):**
`main` now injects the three A6a walkers on `MM_CLIMB_002` route. Against
A1 `original-0.png`, all three placement boxes get **worse** MAE (0.035→0.115,
0.013→0.096, 0.014→0.086). That original is the camera fixture, not the A6a
pause — do not treat this as a compose-path failure until the actor-list
state matches. Sim → `+0x08` remains unsettled.

**A6d3 (MEASURED negative, [check](../scratch/phase-a6-matched-state-bbox.md)):**
one live pause (screenshot + `+0x08` together, A1 camera). Injecting that table
improves two placement boxes (0.113→0.079, 0.106→0.089) and **worsens** the
slope walker (0.101→0.112). Compose is walk temporal 0. Sim → `+0x08` remains
unsettled.

**A6d4 (MEASURED + pixel fit, [check](../scratch/phase-a6-actor2-temporal-sweep.md)):**
walk temporal 0–11 does **not** fix actor 2 (best 6 → 0.10369 > 0.10090).
`composeActorPhase = -0.125` (PROVISIONAL, not a `+0x1D8` read) makes all three
boxes better than tube-only (0.113→0.076, 0.106→0.095, 0.101→0.089).

**A6d5 (READ, [check](../scratch/phase-a6-live-phase-1d8.md)):** all three walkers
hold **0.0** at controller `+0x1D8` (`0000000000000000`). Production phase
default is 0. Actor 2 stays worse than tube-only (0.11246). The −0.125 pixel
fit is parked. Native view index at that pause is unread.

**A6d6 (READ negative, [check](../scratch/phase-a6-live-view-index.md)):** native
`+0x1B8`/`+0x1B4` **match** phase-0 `selectSpriteView` on the inject poses
(slope walker view 2, mirror 1). Actor 2 bbox error is not a wrong view.
Native slope temporal is 4 (already swept; still worse than tube-only).
Actor-2 sprite (temporal / `+0x1D8` / view) is parked after three rounds.

**A6d7 (MEASURED, [check](../scratch/phase-a6-live-pose-pair.md)):**
`ActorPose(+0x08)` matches live `+0x108` within 1e-4 on three walkers at a
10 s pause (worst 2.188e-6; slope Y 8.551e-7). Slope `+0x08` Y is **36.560**
at that pause, not the A6d3 inject **34.480** — different pauses. The A6d3
bbox miss is not a pose-adapter failure.

**A6d8 (MEASURED negative, [check](../scratch/phase-a6-actor2-z-order.md)):**
drawing walkers before the tube makes all three A6d3 boxes **equal** tube-only
(fully occluded). After-tube still fails actor 2 (0.101→0.112). Production
stays after-tube.

**A6d9 (READ + MEASURED negative, [check](../scratch/phase-a6-overlay-alpha-ef.md)):**
overlay composite now uses A4y `a > 0xEF`. On the A6d3 walk path the inject
PNG is unchanged (`c4e6cf47…`) because `drawActorSprite` already writes
overlay alpha as 0 or 255. Actor 2 stays 0.101→0.112. Do not blend or
revive −0.125 to chase that box.

**A6d10 (READ, [check](../scratch/phase-a6-sim-plus8-writer.md)):**
`+0x08`/`+0x10`/`+0x18` stores are `0x42EEC9` / `0x42EE9A` /
`0x42EDF9`+`0x42EE47`. Walk X also inlines `0x40DEC0` at `0x41BE95`.
`0x40DEC0` wraps X into `[0,256)`; `0x40DF50` clamps Y to
`[0, height-1]`; Z helper `0x42EDCD` pulls toward 0.5 by `±arg*0.1`.
`0x43D56C` still reads `+8` directly (`0x42EEF2` copy). `0x42F22E`
adds `[+0x360]`/`[+0x368]` for overlap only; `+0x360` is the Phase 3
facing bias (0 / −1 at `0x42F66B`), not a cell origin. No sim mapper.

**A6d11 (READ, [check](../scratch/phase-a6-plus8-spawn-init.md)):**
the first hatch-spawn sample is `rep movsd` at `0x4230F9`:
`+0x08 = (double)` hatch int16 `+0x9A`, `+0x10 = (double)` hatch
`+0x9C`, `+0x18 = 0.5` (`push 0x3FE00000` at `0x42DE6F`).
`terrainSampleFromHatchSpawn` holds that triple. Post-spawn Y/X
and live Z 0.75 are not this ctor.

**A6d12 (READ, [check](../scratch/phase-a6-plus8-first-y.md)):**
the first lasting post-spawn `+0x10` store on the attached fall
object is `0x41627F`:
`0x40DF50([+0x10] − (double)[+0x44] × step)` with `[+0x44]=1` at
spawn. `0x416219` feeds `+0x18` via `0x43DA38`, not `+0x10`.
`0x42EE9A` is a later/alternate arm. First-tick order unsettled.
No Y mapper.

**A6d13 (READ partial, [check](../scratch/phase-a6-render-clock-dt.md)):**
`0x45CC3D` writes `[game+0x26028] = ([timer+0x40]+128)/256`. That is a
wrap phase, not the walk remaining-budget double. `0x41BDD4` still
reads `[[ebp+0xC]]` at `0x41BE00`. `0x42F0DC` `call [vtable+4]` has
no visible budget pointer. No `ActorAnim` hook.

**A6d14 (READ, [check](../scratch/phase-a6-walk-budget-ptr.md)):**
`0x42F0DC` slot +4 is **`0x43CC68`** (vtable `0x4B14A8` on
`lemming+0x78`), which uses **`[this+0xC8] − 0.0003`**. `0x41BDD4`
is only slot +4 on **`0x4B0D28`** at **`lemming+0x340`**, with the
budget pointer `LEA [ebp-0x1C]` after `0x42E308`. Incoming seconds
and the `[+0xC8]` writer are unread. No `ActorAnim` hook.

**A6d15 (READ, [check](../scratch/phase-a6-render-c8-dt.md)):**
`0x40EAE0`/`0x40EB50` write `[this+0xC8]` as
`fstp [dest+0x10]` (`0x40EBD5`, `0x40EB3C`) after a 3×3 ×
`this+0x90` plus `[0x4C6B88+0x48]`. `0x43CC68` then does
`1.0 / [this+0xC8]` for `0x47A75D` UV, not the A6b clock.
No `fstp [reg+disp 0xC8]`. `0x4C6B88` is past `.data` rawsize.
No `ActorAnim` hook.

**A6d16 (READ negative, [check](../scratch/phase-a6-actor-tube-pixel-z.md)):**
`0x47A75D` → `0x414920` has no per-pixel actor-vs-tube Z compare.
`[this+0xC8]` vs `0.0` is a whole-draw skip. Fragment reject is
`0x472896` `ALPHAREF` (`0xEF`). `0x4149FD` calls `0x46D3C1` to pack
vertices, not to load tube software depth. Overlay stays
replace-if-`a>0xEF`.

**A6d17 (MEASURED, [check](../scratch/phase-a6-actor2-bbox-pixels.md)):**
actor-2 box 315–326 × 283–301 has **106** painted pixels,
**extra_coverage 0**, MAE 0.101→0.112 unchanged. Worker
`integer_shift=94` is a `+6` neighbor test (exact neighbor match
is 7). Isolated `originY − 1.0` beats tube at **0.09760** and
improves actors 0 and 1; restored; do not ship. 61/61 compose,
24/24 CTest.

**A6d18 (READ, [check](../scratch/phase-a6-billboard-top-row.md)):**
`0x47AB27`–`0x47AB30` stores float top `[this+0x160]` as
`(originY − unknown1) − height`. `0x47AAD6` is the right edge.
No `fistp` on the submit chain. `ceil(p.top)` stays the A4v cover
fit. No `ActorSprite` hook.

**A6d19 (MEASURED negative, [check](../scratch/phase-a6-first-row-floor.md)):**
`y0 = floor(top)` leaves the A6d3 matched frame identical to `ceil`
(SHA `15016c20…`) and actor-2 MAE **0.11246**. Initial-view boxes
move and still fail. A4v `y=2` fixture fails under floor. `ceil`
restored (`977bda40…`).

**A6d20 (READ, [check](../scratch/phase-a6-screen-y-bias.md)):**
`[ebp+0x10]` on entry to `0x47A75D` is
`[0x58D298] − (1/[this+0xC8])·[this+0xC0]`, stored at `0x43CD32`
(`fsubp`, not `fsubrp`). `0x47A7BB` subtracts `unknown1` only.
No ±1. The divisor is raw `+0xC8`. Do not ship `originY − 1`.

**A6d21 (READ, [check](../scratch/phase-a6-plus90-vector.md)):**
`0x4B14A8` slot +8 is `0x4658D6`, which copies 24 bytes of the
`0x43D56C` result to `this+0x90` at `0x46591A`. `0x42D3E9` writes
the lemming root `+0x90`, a different object. `0x40EAE0` still
multiplies the render triple by `0x4C6B88`. No `ActorSprite` hook.

**A6d22 (READ, [check](../scratch/phase-a6-xform-4c6b88.md)):**
`0x46BE99` copies 96 bytes from the `0x58D5A0` stack top into
`0x4C6B88` (`0x4C6A08+0x180`) on the `0x43A4EC` path. That is the
matrix `0x43CC71` passes to `0x40EAE0`. Not file-static identity.
Not shown to equal `worldToLensedView`. No `ActorSprite` hook.
Screen-Y bias parked after this third round.

**A6d23 (MEASURED + READ, [check](../scratch/phase-a6-native-camera-pair.md)):**
at a fresh canonical-camera pause, compiled production screen Y agrees with
three actor-cache triples within **8.963683444562776e-05 px** (predeclared
1e-4 target). No screenshot; not the A6d3 pause, no A6 pixel pass. Affine
depth is about 0.0003 above cache: `0x43CCB5` subtracts `0x4B1498`, then
`0x43CDAD` stores through `(render+0xB0)+0x18`, aliasing **render+0xC8**.
The existing production bias is consistent; do not fit another offset.
Between-draw matrix provenance remains unproven. Screen-Y lead re-parked;
24/24 CTest. Entry-slot restoration readback matches 280 bytes, but the
audit intercepts restoration only, not the initial imported writer.

**A6d24 (offline MEASURED, [check](../scratch/phase-a6-full-frame-gate.md)):**
the unscored full-frame A6 clause now **fails**: unchanged actors-on is
`49,881,782 / 235,008,000 = 0.21225567640250545`, not <=0.02. Tube-only is
`49,888,588 / 235,008,000`. Fresh PNGs byte-match A6d3; this is not a new
regression. Fixed boxes still improve for actors 0/1 and worsen for actor 2
(0.10089786756453423 -> 0.11246451442529874). Independent integer/histogram
recompute, single-channel scorer check and 24/24 CTest pass. No named scene
cause measured; full controller/temporal pairing incomplete. Localize dominant
global error before more actor pixel-fit work; do not shrink the full-frame gate
to A4d's passing ROI.


