# What we already know

Hard reset of 2026-10-18. Everything retired lives under `old/` — `old/PLAN.md`,
`old/docs/` (the previous plan, four handoffs, 42 round notes), `old/src/gfx` and
`old/src/shell`. Nothing was deleted. This file is the carry-forward: the facts worth
knowing before you touch anything, each one with the address or the script behind it.

**Read this before the plan.** It exists so a new agent does not re-derive, re-measure, or
re-litigate what cost previous sessions weeks.

## The one rule

The stock 640x480 `Lemmings Revolution.exe` is the authority — not classic Lemmings, not
plausibility, not a decompiler's prose. A claim needs **an address and the bytes read
there**, or a re-runnable script. A decompiler line is not a measurement.

SHA-256 of the authority: `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, 32-bit PE.

## Camera checkpoint: A1 complete for the tested scope

The accepted pose/lens operand reads and their exact addresses are in
[the A1 facilitator check](scratch/phase-a1-camera.md). Its standalone fixture has
independent original-image mutation/restore proofs; it is not a finished renderer.
That first checkpoint retained a failed left view. The later
[true live-arena replay](scratch/phase-a1-live-arena.md) cleared all four retained
views, and the [zoom/elevation check](scratch/phase-a1-zoom-elevation.md) completed
A1 for the tested MM_CLIMB_002 camera branches. Do not revive the old failure as
the current verdict, or expand the bounded passes into full-scene acceptance.

The scale helper executes **FPTAN**, not sine (`0x0047B095` calls `0x0049D8D4`,
`D9 F2` at `0x0049D8F2`). Projection divides by transformed depth and reverses screen y
(`0x0046DDF0` for the tube). `0x0047D0C0` is the sprite/billboard path, as corrected
in the [outer-route check](scratch/phase-a1-outer-route.md). The captured center was `(319,239)` at stock resolution,
from BSS globals `0x0058D294` / `0x0058D298`; each component has a separate pixel mutation.
The conventional center assumption and earlier sine interpretation were explicitly
corrected. Exact original floating-point intermediate precision is still unestablished.

Control action names do not imply unconditional rotation. The captured default path
moved the virtual cursor without a grab; a grab over empty scenery changed the meaning of
the same inputs. Window activation and selection matter. Preserve the failed view and the
scope limits in the linked note before proposing the next round.

The [active cylinder draw follow-up](scratch/phase-a1-cylinder-draw.md) corroborates a
bounded frame-time inner-bank submission at the retained left camera and matches its
source vertex pool to the stored level. It does not fix the outer-only fixture's failed
pixels by itself; the later live-arena note carries the passing result. Use the
foreground-acquisition helper from a hidden worker; it masks
activation input from the game and verifies actual focus. Do not require a manual click
for every probe or use the older input-queue attachment helper.

The [cell-fixture replay](scratch/phase-a1-cell-oracle.md) retains the subsequent
failed geometry candidate, isolated mutation survivors and explicit corrections.
Its source/runtime mapping is READ evidence, with unprotected selection mutations.
The subsequent [emission check](scratch/phase-a1-cell-emission.md) established a
selected quad's ordered coordinates and UVs; the
[projection/cache replay](scratch/phase-a1-projection-cache.md) supplied a bounded
source derivation. The [depth check](scratch/phase-a1-depth-clipping.md) found no
numeric executable near/far pair; the wrapper's internal Z bits remain unreadable.
The [control mapping](scratch/phase-a1-control-mapping.md) and final zoom check
supersede the earlier open-camera handoff. The gameplay writer of `control+0x10`
is still unlocated (its behavior is measured). The entry point remains A4.

## Tube checkpoint: A2 fixed-ROI checks complete; wider diagnostics still fail

The [A2 facilitator check](scratch/phase-a2-tube.md) and
`tools/harness/verify_tube_a2.py` retain the independent byte reads and replays.
Texture addressing flips vertical bands: `g//4 + ((bands-1-b)//2)*8`, then
`+24` for INN. The old unflipped brief agreed only on the middle bands: it
disagrees on 128 of 192 captured cells. The store is `0x460A2D 89b0c8000000`.

The `0x43A4EC` walker draws INN before the three OUT routes; the OUT sides use
different mesh banks, and the center uses signed-short filters. Those choices
are READ evidence where isolated mutations survive the pixel gate. All nine
retained views and the exposed inner-wall region pass, but the broader changed-
pixel diagnostic **fails at 0.022030839 against 0.02**. No full-frame or all-level
fidelity is accepted. Retain that failure for A4 integration.

The upper gold band survives reversible suppression of the entire tube walker
(`0x43A4EC`, original `55`, temporary `C3`, restored `55`); it is a separate scene
contribution in that frame, with exact resource still unknown. The scenery,
objects, actors and UI inventory is in the A2 note. Code/camera bytes, save and
wrapper were restored; full screenshot restoration is not exact because the
hatch/count area animates. The A3 checkpoint below supersedes this handoff; **next is A4**.

## Sprite checkpoint: A3 selection verified; production integration remains A4

The [A3 facilitator check](scratch/phase-a3-sprite-view.md) corrects a settled-doc
error: `.SPR` outer entries are **temporal**, inner entries are **directional**.
Across all six lemming files, walk/fall have five directional images per temporal
entry, block/cartwheel eight, shrug one. Walk's 12 and fall's 7 were LO temporal
counts, not views. Native `0x468833` advances controller `+0x1CC` using action
rate `[action+0x10]` (`0x46887F 8b5010`, `0x4688E6 898acc010000`).
Controller `+0x1D8` is a chooser rotation input, not that SPR rate; older harness
labels must not override the consumer's bytes. Existing simulation series stand.

Chooser `0x46840B` uses the normalized actor-world-position minus camera
translation and the stored orientation basis, rotated at phase+1/16 and then
another 3/16 turns. PI is `3.1415927410125732`; actual FSIN/FCOS opcodes are
`0x49CBA2 d9fe` / `0x49CC52 d9ff`. Scores get one +/-2 correction outside
[-1,1]. Five-entry selection rewrites mirror; eight-entry and <=1 paths preserve
it. The <=1 path forces index zero. Do not substitute uniform angular bins or
flatten the action→temporal-array dereference. Exact bytes and the actor producer
are in the linked note and `tools/harness/verify_spriteview_a3.py`.

The independent checks pass 608 native instruction cases and 42/42 live actor
predictions, including a controlled eight-entry graphics action and two camera
sector crossings. Shared-stock-raster replay has zero candidate/restored crop
error for both five/eight fixtures; isolated index/mirror mutations alter pixels.
**This is selection acceptance, not replacement raster fidelity.** The worker's
stricter index-zero sensitivity >0.02 gate still fails and is retained. One-entry
and other fallback counts are READ/test coverage; inverted poses and all-action
pixel fidelity are not accepted. All 15 CTest tests pass. A4 must integrate the
helper with correct temporal/directional axes and retain A2's broader failures.

## A4 checkpoint: a runnable headless renderer, but not yet the original's tube

The [A4a check](scratch/phase-a4-renderer-core.md) added the first production
`webolution` target (SDL2 linked, headless `--shot` only; no window/controls yet).
It loads a retail level and draws the tube from stored data at the verified A1
camera. **17/17 tests pass.** All nine retained A2 views still fail the unchanged
`0.02` gate; best is `initial` at `0.037812306`.

The stored `CylinderMesh::surface` pool is **byte-identical** to the live source
bank (897 vertices, SHA-256
`4e3a52f579c663fc0e97d217d8ea5d780c246f9c6bb33c112534ac6b20c3b630`), and the
constant wall comes from `CylinderMesh::vertices`. The runtime **per-cell mesh is
not** the stored `PolyList 09` records: 8/450 unique index sets match exactly
(10/450 with rotations/reversals), 0/450 by full index+UV. The earlier worker
count "16/450" was corrected. A file-side presence rule does hold: 156/156
runtime outer-wall cells are solid in the stored collision grid, 0 present&empty,
9 absent&solid. Production code must not use the live arena as an input.

The missing piece is the producer of the per-cell quad arrays at `0x4ECB00`
(indices, UVs, colour, mesh0/mesh4 selection) from stored data. Find that before
adding controls; a smooth tube is not the original. The A2 wider/cavity/rim
failures and the A3 sensitivity failure remain retained.

## A4b checkpoint: the cell mesh is generated from the collision grid

The [A4b check](scratch/phase-a4-cell-mesh-source.md) located the producer chain.
The per-cell quads at `0x4ECB10` are **built at level load**, not read from stored
`PolyList 09` or stored per-cell topology. The chain is pinned by bytes:
`0x43A41A -> 0x461344 -> 0x463070(0x4ECB10)` resets the mesh pool; the bucket
sweep `0x43A431..0x43A44E` calls `0x4608BB`; `0x460A41 -> 0x462264 ->
0x4615E9` builds the face list; `0x46227E -> 0x463410` assembles mesh0/mesh4;
`0x4622ED` loads the `0x4ECB10` pool and `0x464F9C` bumps it by
`count*0x80 + 0x10`. The collision accessor `0x40DE60` indexes
`world+0x41C[(x&0xff) + y*0x100]`, so the topology input is the stored 256x96
collision grid. The stored 897-vertex pool is byte-identical to the live bank;
that it is “the baked output of the same generator” is an **inference**, not a
measured byte fact.

**The exact scan is still open:** `FUN_004615E9` (2100 bytes) and the part table
`FUN_00461E1D` are not decoded, so no production rule was implemented and the
A4a `0/9` gate is unchanged. 17/17 tests still pass. Next: decode those two
functions and validate an offline generator against the retained arena before
touching production.

## A4c checkpoint: the cell-mesh rule reproduces the live arena

The [A4c check](scratch/phase-a4-cell-mesh-rule.md) decoded the rule and reproduced
the retained arena exactly from stored data: **389/389 mesh0** and **387/387 mesh4**
quads by indices, UVs, colour and order. `FUN_004615E9` is the **gap pass**, not a
face emitter; `FUN_00461E1D` builds the per-column part lists; `FUN_00463410`
assembles mesh0 (rectangle order, right gaps, top gaps) and mesh4 (re-sorted
rectangles, left gaps, top gaps). The key correction is that `LevelIndex::grid0`
is the **`(x, y, layer) -> pool index` table** at `world+0x42C`, key
`layer + (x + y*0x101)*2` (`0x43C93A`, `0x43C948`); 897 of 49,858 slots are
populated on `MM_CLIMB_002`.

`src/assets/CellMesh` implements the rule and `test_cellmesh` is file-derived;
18/18 tests pass. The one-term mutation `g.end = y + 1` fails the key-support
invariant (389/387 counts alone do not catch it) and restores byte-identically.
The nine retained views still **0/9** (best `initial` `0.037620585`, slightly
better than wall `0.037812306`); A2 cavity/rim and A3 sensitivity remain failed.
The missing piece is now the **draw/texture binding**: `--geom cellmesh` adds both
banks to the constant wall and does not apply A2's mesh0/mesh4 route, INN/OUT
bank choice or center filters. The door path is implemented but unexercised;
only `MM_CLIMB_002` is validated.

## A4d checkpoint: the original tube route passes the nine-view gate

The [A4d check](scratch/phase-a4-cellmesh-route.md) routes the A4c cell mesh through
the `0x43A4EC` walker's real passes: INN (`c..b`, `entry+8`, material `+24`), OUT
right (`d-1..mid+1`, mesh4 reversed), OUT left (`a..mid-1`, mesh0 reversed), OUT
centre (`mid`, mesh0 forward `coord < centre*8` then mesh4 forward `coord >=`).
The constant lattice wall is **replaced**, not kept: keeping it is 0/9. The INN
inline quad is the block's layer-0 rectangle gated by `entry+0xF8` (98/98 arena
match) and the child coordinate is the solid rectangle's `x0` (776/776). The `HI`
cylinder-tile variant is required: plain is 0/9, `HI` is 9/9. The loader's own
`HI` selection convention is inferred, not byte-read.

All nine retained views pass the fixed gate (`initial` 0.015584142, worst `left`
0.015679210); the production frame matches the A2 passing fixture at 2.6e-5
full-frame. 18/18 tests pass; `verify_cellmesh_route.py` re-reads 24 instructions
and 5 stored doubles. Swapping mesh0/mesh4 or disabling INN drops to 6/9; plain
tiles/keep-wall drop to 0/9; inverting the centre filters is **not discriminated**
by this ROI. The A2 wider `0.022041013`, cavity, upper rim and A3 sensitivity
failures remain. `MM_CLIMB_002` only; the door and `entry+0xB8` paths are
unexercised. The A4e checkpoint below supersedes the window/control handoff.

## A4e checkpoint: window and camera input integrated; scene and actors remain open

The [A4e check](scratch/phase-a4-window-controls.md) adds the 640x480 SDL window,
fixed 144 Hz camera updates and one-shot input queue. Normal launch uses `route`;
`--shot` remains headless. Independent verification passes 19/19 tests, 54
retained-stock input comparisons, native presentation/SPACE/close smoke, and
all nine A4d scores and PNG hashes unchanged. Fourteen isolated source regressions
are discriminated after correcting an initially ineffective zoom-exit test.
The old default wall fails the fixed pixel gate at `0.031833014`; restored route
passes at `0.015584142`.

READ corrections: zoom elevation/nudge setup is entry-only (`0x45306A 837a1401`,
`0x453077 83781000`). The edge ratio has a strict `<0.1` deadzone
(`0x453167 dc1d40104b00`, `0x453172 740e`), and its double 30 gain is at
`0x4B1748 0000000000003e40` (`0x4531C6 dc0d48174b00`), not the earlier predictor
comment's `0x4C03D8`. The linked note retains exact flip-delta, yaw-wrap and
cursor-clamp reads. No stock process was started and no new live measurement
was claimed. Fixed 144 Hz approximates the sampled ~143.9 Hz original; accumulation
is capped at 0.25 s. Main has no object-attachment/picking or pause/menu integration.
Next proposal: one actor billboard fixture using accepted A3 selection, then
simulation integration and omitted scene. A2/A3 broader failures remain retained.

## A4f checkpoint: actor billboard geometry exact; pixels fail on vertex tint

The [A4f check](scratch/phase-a4-actor-billboard.md) adds `src/app/ActorSprite`. For the
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

**A4g narrowing** ([check](scratch/phase-a4-actor-tint-writer.md)): the 152 records sit
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

**A4r (READ, [check](scratch/phase-a4-actor-render-state.md)):** material updater
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

**A4s (retained observation + READ, [check](scratch/phase-a4-actor-material-binding.md)):**
the two A4f controllers at `actor+0x128` are the submit objects consumed by
`0x414920`. Both recorded first dwords are selector **0**; their selected
direction records supply resource handles 119/120 (session-local). Selector zero
encodes blend-disable on a cache miss, not proof of effective device state.
Resolved material `+0x24` flags remain unrecorded in the checked packets, so the
actor's material/alpha branch remains unresolved. BSS placement does **not** prove
the flags cannot be derived statically. No production change or new pixel claim;
independent fixture/byte checks and 20/20 CTest pass.

**A4t (MEASURED + READ, [check](scratch/phase-a4-actor-material-flags.md)):** live on
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

**A4u (code + offline MEASURED, [check](scratch/phase-a4-actor-alpha-test.md)):**
`drawActorSprite` now applies row 3. A texel is drawn only when its alpha is greater than
`0xA0`. A drawn texel is written opaque, with no blend. The reference load is reread at
`0x472896`, and the state pushes `0xF`/`0x18`/`0x19` at `0x472880..0x4728C2`. The test is
**pixel-invisible** on both A4f fixtures: their source frames are strictly 0/255 alpha,
and the gate MAE and composed hashes equal A4q's. Only the synthetic unit test
discriminates `>` from `>=`. The five fixture's frame is on `$LEMSHEETHI13`, whose flags
A4t did not measure (A4t read Hi15 and Hi12), so row 3 there is assumed.

**A4v (code + offline MEASURED, [check](scratch/phase-a4-actor-support-residual.md)):**
the ~0.08 support residual was a **half-pixel convention**, not filtering. Native matches
an integer pixel centre: cover `left <= x < right`, UV `(x − left)/(right − left)`, nearest
texel. Footprint now equals native on both fixtures; support MAE 0.0819 → 0.00143 and
0.0778 → 0.00152. It is a two-fixture fit, not a byte. `0x470F29` writes alpha reference
`0xEF` then `0xA0` only on its LINEAR filter branch, and A4t read `0xA0` live, so the
filter in force at actor draw is **untraced** and contradicts the nearest-texel fit. The
gate passes under each reverted part; only the unit test protects them, and the first V
test could not see V.

**A4w (MEASURED live, [check](scratch/phase-a4-actor-hi13-flags.md)):** `$LemSheetHi13`
descriptor `+0x24` = **`0x2`** on `MM_CLIMB_002`, read through a bound actor with no
measurement writes (level entry uses the restored `enter_fast.py` swap). Every `$LemSheetHi0..23` entry in the 139-entry pool reads `0x2`, so row 3
holds for all lemming sheets on this level (encoded flags; device state still unread).

**A4x (READ + MEASURED live, [check](scratch/phase-a4-actor-filter-state.md)):** the
actor submit `0x47A75D` brackets its draw with `0x470F29(0)` before (`0x47A776`, when
`[0x58D53C]` == 0, which it read live) and `0x470F29(1)` after (`0x47AB8B`). So actors
draw under **POINT** filtering with alpha reference **`0xEF`**. The `0xA0` A4t and A4w read
is the between-draw value (live: `0xEF` in 0.43% of 1 kHz samples). `dgVoodoo.conf`
filtering is `appdriven`. **Lesson: a global read between draws is not the state at
draw**; look for a bracket around the consumer before trusting a sampled render global.
A4u's `> 0xA0` should be `> 0xEF` (pixel-invisible on both fixtures).

**A4y (code + offline MEASURED, [check](scratch/phase-a4-actor-alpha-reference.md)):**
the actor alpha reference is now **`0xEF`**, with strict `>`. Destination-pixel tests
reject `0xA1` and `0xEF`, accept `0xF0`, and fail under either the old `0xA0` constant
or an isolated `>=` mutation. The facilitator's separate exhaustive raster checker
passes all 256 alpha values; both A4f gate results and image hashes equal A4v's
exactly (binary-alpha fixtures cannot see the correction). Fresh 20/20 CTest pass;
no new live measurement, effective-state claim or app actor integration. A4x's
scope limits and A2/A3's broader failures stand. Next queued action: A5m map.

**A5m (repository map, [check](scratch/phase-a5m-actor-integration-map.md)):** the
app still draws the tube only; helpers exist but no World-to-actor draw chain.
Do not pass terrain x/y straight as 3D world coordinates or treat `Lemming::frame`
as a universal temporal index: the drowning path adds milliseconds, unlike the
general frame increment. World-pose conversion, orientation basis/phase/prior
mirror, animation progression and shared tube depth remain adapter prerequisites.
No new original-game measurement or implementation landed. Next queued: A4z,
whose environment-specific brief needs adapting before a local Windows dispatch.

**A4z (live capture + offline replay, [check](scratch/phase-a4-actor-third-fixture.md)):**
the new half-scale sample passes (support MAE 0.001380834), but full-scale zoom1
fails even after matched-camera replay: 27 extra candidate pixels and support
MAE 0.0284529 > 0.0151558. This does not isolate the pixel-centre convention as
the cause. **A4x's POINT/EF claim is zero-zoom scoped:** `0x47A766 833d3cd5580000`
and `0x47A76D 750c` skip that setter at nonzero zoom. Inherited zoom1
filter/alpha state remains open; do not fix it by fitting pixels. Own recompute,
byte pins, replay and 20/20 CTest pass; source/install unchanged and process
stopped. enter_fast's raw level-slot restoration bytes were not retained, so
the live ledger is not fully byte-audited. Next: zoom1 filter/alpha provenance.

**A4aa (READ, [check](scratch/phase-a4-zoom1-filter-provenance.md)):** no unique
pre-zoom1-actor assignment was established within the bounded static trace.
The explicit frame/tube bodies have no immediate 0x470F29 call, but unresolved
subdraws and unexamined descendants are not certified clean. The setter chooses
local 1/EF or conditional 2/A0 **before** requesting the two filter states; it
does not necessarily send POINT then LINEAR. Global ALPHAREF source and cached
device ALPHAREF are different evidence. Own 38 byte pins, independent A4r checker
and 20/20 CTest pass. Nearest-prologue heuristic graphs/literal-address scans do
not prove transitive or computed-writer absence. Next: actor-correlated zoom1
filter-request observation; no production raster fix or effective-state claim.

**A4ab (failed live experiment, [check](scratch/phase-a4-zoom1-filter-trace-failed.md)):**
the five-hook run crashed in D3DImm, with zero bound draws/request tuples retained.
No hook-specific cause was isolated. HOOK_INSTALL records were emitted before
attach; they are announcements, not success proof. Buffered unpersisted callbacks
mean an empty trace is not proof callbacks never ran. Own mock reproduction finds
Number.add in the actor binder; that is a dormant harness defect, not a native
crash diagnosis. Mid-call esp[0] is the device argument, not a return address.
This run's raw slot ledger and install hashes match; live hook-code restoration
readback was not achieved. Own cleanup, hashes and 20/20 CTest pass; no filter or
pixel acceptance. Next: offline binder repair before any new stock attempt.

**A4ac (offline harness regression, [check](scratch/phase-a4-actor-binder-offline.md)):**
`hooks_a4ac_candidate.js` repairs four dormant binder bugs while keeping
`hooks_a4ab.js` byte-identical: (1) converts scalar `arrAddr` with `ptr(arrAddr)`
and guards slot reads via `u32(arr.add(i*4))`; (2) returns `{ actors, match }` on
match instead of returning `match` directly (which lacked `.match` and caused
`DRAW` `if (!snap.match)` to misclassify matches as `BIND_MISS`); (3) uses
`ptr(...).add(0x128).toString()` for `selection.ctrl` instead of `hexN(ptr(...))`
(which coerced a pointer object to `'0x0'`); and (4) binds only on `actor+0x128`,
rejecting the unmeasured `action_obj+0xb0` alias. Four isolated single-line
mutations fail the `node:vm` suite and restore byte-exact; 20/20 CTest pass.

**A4ad (offline harness regression + READ, [check](scratch/phase-a4-hook-stack-announce-offline.md)):**
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

**A4ae (READ, [check](scratch/phase-a4-pre-actor-subcalls-static.md)):**
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

**A4af (READ, [check](scratch/phase-a4-tube-child-renderers-filter-static.md)):**
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

**A6a (READ + MEASURED, [check](scratch/phase-a6-actor-pose.md)):** model
`+0x108` is `0x43D56C` bilinear interpolation of the cell mesh at actor `+0x08`.
X is bucketed by 8 and wrapped `& 0x1F`, Y is bucketed by 16, and Z blends the
inner/outer shells. Missing `grid0` keys are filled from the cylinder template by
`0x43A31D` -> `0x43D2B8`. The basis is the normalized difference after adding
`facing * 0.1` to X (`0x42EF38`). Three live walkers on `MM_CLIMB_002` match the
position within 1e-4. The basis misses the 1e-6 falsifier (max 2.962e-5; native
values are float32) and is parked. **Do not invent missing outer vertices**:
the first packet's radial scaling fit to 3.7e-7 without an address. Sim -> `+0x08`
(sub-cell X, Y flip, Z slab) is unsettled.

**A6b (READ + MEASURED, [check](scratch/phase-a6-actor-anim-clock.md)):** controller
`+0x1CC` advances in `0x468833` by `acc += elapsedSeconds * [strip+0x10]`, then
while `acc > 0.06` increments the index and **subtracts** 0.06 (remainder
carried). `0x468AB8` resets index and accumulator on strip bind. `+0x201` set
wraps to 0; clear holds at count-1. A 45-sample walker series is integer
milliseconds (6–9) per interval; a no-resync forward replay matches. **Do not
use `Lemming::frame` as the SPR temporal index.** Immediate `param_1` is
`min(remaining budget, time-to-next-cell / 5.0)` from `0x41BDD4`; the outer
budget writer is unsettled. The first packet's 0.2 s/tick prediction is
falsified.

**A6c (READ, [check](scratch/phase-a6-actor-tube-composition.md)):** actors in
`0x4C6A10+0x1E88` draw **after** INN and all three OUT passes, walking the
list **high index to 0**. `[elem+0x18] >= 1.0` uses `0x43A9BB`; `< 1.0` uses
`0x43AA47` (A4ae; the A6c packet reversed this). No executable
`SetRenderState` for ZENABLE/ZWRITE/ZFUNC/ZBIAS. Per-pixel actor Z versus
tube Z is unsettled.

**A6d (repo MEASURED, [check](scratch/phase-a6-injected-walkers.md)):** the
production *route* can overlay three injected A6a walkers after the tube
(pose → A3 view / walk temporal 0 → sprite). Empty list keeps the A4e
initial PNG hash.

**A6d2 (MEASURED negative, [check](scratch/phase-a6-app-bbox-gate.md)):**
`main` now injects the three A6a walkers on `MM_CLIMB_002` route. Against
A1 `original-0.png`, all three placement boxes get **worse** MAE (0.035→0.115,
0.013→0.096, 0.014→0.086). That original is the camera fixture, not the A6a
pause — do not treat this as a compose-path failure until the actor-list
state matches. Sim → `+0x08` remains unsettled.

**A6d3 (MEASURED negative, [check](scratch/phase-a6-matched-state-bbox.md)):**
one live pause (screenshot + `+0x08` together, A1 camera). Injecting that table
improves two placement boxes (0.113→0.079, 0.106→0.089) and **worsens** the
slope walker (0.101→0.112). Compose is walk temporal 0. Sim → `+0x08` remains
unsettled.

**A6d4 (MEASURED + pixel fit, [check](scratch/phase-a6-actor2-temporal-sweep.md)):**
walk temporal 0–11 does **not** fix actor 2 (best 6 → 0.10369 > 0.10090).
`composeActorPhase = -0.125` (PROVISIONAL, not a `+0x1D8` read) makes all three
boxes better than tube-only (0.113→0.076, 0.106→0.095, 0.101→0.089).

**A6d5 (READ, [check](scratch/phase-a6-live-phase-1d8.md)):** all three walkers
hold **0.0** at controller `+0x1D8` (`0000000000000000`). Production phase
default is 0. Actor 2 stays worse than tube-only (0.11246). The −0.125 pixel
fit is parked. Native view index at that pause is unread.

**A6d6 (READ negative, [check](scratch/phase-a6-live-view-index.md)):** native
`+0x1B8`/`+0x1B4` **match** phase-0 `selectSpriteView` on the inject poses
(slope walker view 2, mirror 1). Actor 2 bbox error is not a wrong view.
Native slope temporal is 4 (already swept; still worse than tube-only).
Actor-2 sprite (temporal / `+0x1D8` / view) is parked after three rounds.

**A6d7 (MEASURED, [check](scratch/phase-a6-live-pose-pair.md)):**
`ActorPose(+0x08)` matches live `+0x108` within 1e-4 on three walkers at a
10 s pause (worst 2.188e-6; slope Y 8.551e-7). Slope `+0x08` Y is **36.560**
at that pause, not the A6d3 inject **34.480** — different pauses. The A6d3
bbox miss is not a pose-adapter failure.

**A6d8 (MEASURED negative, [check](scratch/phase-a6-actor2-z-order.md)):**
drawing walkers before the tube makes all three A6d3 boxes **equal** tube-only
(fully occluded). After-tube still fails actor 2 (0.101→0.112). Production
stays after-tube.

**A6d9 (READ + MEASURED negative, [check](scratch/phase-a6-overlay-alpha-ef.md)):**
overlay composite now uses A4y `a > 0xEF`. On the A6d3 walk path the inject
PNG is unchanged (`c4e6cf47…`) because `drawActorSprite` already writes
overlay alpha as 0 or 255. Actor 2 stays 0.101→0.112. Do not blend or
revive −0.125 to chase that box.

**A6d10 (READ, [check](scratch/phase-a6-sim-plus8-writer.md)):**
`+0x08`/`+0x10`/`+0x18` stores are `0x42EEC9` / `0x42EE9A` /
`0x42EDF9`+`0x42EE47`. Walk X also inlines `0x40DEC0` at `0x41BE95`.
`0x40DEC0` wraps X into `[0,256)`; `0x40DF50` clamps Y to
`[0, height-1]`; Z helper `0x42EDCD` pulls toward 0.5 by `±arg*0.1`.
`0x43D56C` still reads `+8` directly (`0x42EEF2` copy). `0x42F22E`
adds `[+0x360]`/`[+0x368]` for overlap only; `+0x360` is the Phase 3
facing bias (0 / −1 at `0x42F66B`), not a cell origin. No sim mapper.

**A6d11 (READ, [check](scratch/phase-a6-plus8-spawn-init.md)):**
the first hatch-spawn sample is `rep movsd` at `0x4230F9`:
`+0x08 = (double)` hatch int16 `+0x9A`, `+0x10 = (double)` hatch
`+0x9C`, `+0x18 = 0.5` (`push 0x3FE00000` at `0x42DE6F`).
`terrainSampleFromHatchSpawn` holds that triple. Post-spawn Y/X
and live Z 0.75 are not this ctor.

**A6d12 (READ, [check](scratch/phase-a6-plus8-first-y.md)):**
the first lasting post-spawn `+0x10` store on the attached fall
object is `0x41627F`:
`0x40DF50([+0x10] − (double)[+0x44] × step)` with `[+0x44]=1` at
spawn. `0x416219` feeds `+0x18` via `0x43DA38`, not `+0x10`.
`0x42EE9A` is a later/alternate arm. First-tick order unsettled.
No Y mapper.

**A6d13 (READ partial, [check](scratch/phase-a6-render-clock-dt.md)):**
`0x45CC3D` writes `[game+0x26028] = ([timer+0x40]+128)/256`. That is a
wrap phase, not the walk remaining-budget double. `0x41BDD4` still
reads `[[ebp+0xC]]` at `0x41BE00`. `0x42F0DC` `call [vtable+4]` has
no visible budget pointer. No `ActorAnim` hook.

**A6d14 (READ, [check](scratch/phase-a6-walk-budget-ptr.md)):**
`0x42F0DC` slot +4 is **`0x43CC68`** (vtable `0x4B14A8` on
`lemming+0x78`), which uses **`[this+0xC8] − 0.0003`**. `0x41BDD4`
is only slot +4 on **`0x4B0D28`** at **`lemming+0x340`**, with the
budget pointer `LEA [ebp-0x1C]` after `0x42E308`. Incoming seconds
and the `[+0xC8]` writer are unread. No `ActorAnim` hook.

**A6d15 (READ, [check](scratch/phase-a6-render-c8-dt.md)):**
`0x40EAE0`/`0x40EB50` write `[this+0xC8]` as
`fstp [dest+0x10]` (`0x40EBD5`, `0x40EB3C`) after a 3×3 ×
`this+0x90` plus `[0x4C6B88+0x48]`. `0x43CC68` then does
`1.0 / [this+0xC8]` for `0x47A75D` UV, not the A6b clock.
No `fstp [reg+disp 0xC8]`. `0x4C6B88` is past `.data` rawsize.
No `ActorAnim` hook.

**A6d16 (READ negative, [check](scratch/phase-a6-actor-tube-pixel-z.md)):**
`0x47A75D` → `0x414920` has no per-pixel actor-vs-tube Z compare.
`[this+0xC8]` vs `0.0` is a whole-draw skip. Fragment reject is
`0x472896` `ALPHAREF` (`0xEF`). `0x4149FD` calls `0x46D3C1` to pack
vertices, not to load tube software depth. Overlay stays
replace-if-`a>0xEF`.

**A6d17 (MEASURED, [check](scratch/phase-a6-actor2-bbox-pixels.md)):**
actor-2 box 315–326 × 283–301 has **106** painted pixels,
**extra_coverage 0**, MAE 0.101→0.112 unchanged. Worker
`integer_shift=94` is a `+6` neighbor test (exact neighbor match
is 7). Isolated `originY − 1.0` beats tube at **0.09760** and
improves actors 0 and 1; restored; do not ship. 61/61 compose,
24/24 CTest.

**A6d18 (READ, [check](scratch/phase-a6-billboard-top-row.md)):**
`0x47AB27`–`0x47AB30` stores float top `[this+0x160]` as
`(originY − unknown1) − height`. `0x47AAD6` is the right edge.
No `fistp` on the submit chain. `ceil(p.top)` stays the A4v cover
fit. No `ActorSprite` hook.

**A6d19 (MEASURED negative, [check](scratch/phase-a6-first-row-floor.md)):**
`y0 = floor(top)` leaves the A6d3 matched frame identical to `ceil`
(SHA `15016c20…`) and actor-2 MAE **0.11246**. Initial-view boxes
move and still fail. A4v `y=2` fixture fails under floor. `ceil`
restored (`977bda40…`).

**A6d20 (READ, [check](scratch/phase-a6-screen-y-bias.md)):**
`[ebp+0x10]` on entry to `0x47A75D` is
`[0x58D298] − (1/[this+0xC8])·[this+0xC0]`, stored at `0x43CD32`
(`fsubp`, not `fsubrp`). `0x47A7BB` subtracts `unknown1` only.
No ±1. The divisor is raw `+0xC8`. Do not ship `originY − 1`.

**A6d21 (READ, [check](scratch/phase-a6-plus90-vector.md)):**
`0x4B14A8` slot +8 is `0x4658D6`, which copies 24 bytes of the
`0x43D56C` result to `this+0x90` at `0x46591A`. `0x42D3E9` writes
the lemming root `+0x90`, a different object. `0x40EAE0` still
multiplies the render triple by `0x4C6B88`. No `ActorSprite` hook.

**A6d22 (READ, [check](scratch/phase-a6-xform-4c6b88.md)):**
`0x46BE99` copies 96 bytes from the `0x58D5A0` stack top into
`0x4C6B88` (`0x4C6A08+0x180`) on the `0x43A4EC` path. That is the
matrix `0x43CC71` passes to `0x40EAE0`. Not file-static identity.
Not shown to equal `worldToLensedView`. No `ActorSprite` hook.
Screen-Y bias parked after this third round.

**A6d23 (MEASURED + READ, [check](scratch/phase-a6-native-camera-pair.md)):**
at a fresh canonical-camera pause, compiled production screen Y agrees with
three actor-cache triples within **8.963683444562776e-05 px** (predeclared
1e-4 target). No screenshot; not the A6d3 pause, no A6 pixel pass. Affine
depth is about 0.0003 above cache: `0x43CCB5` subtracts `0x4B1498`, then
`0x43CDAD` stores through `(render+0xB0)+0x18`, aliasing **render+0xC8**.
The existing production bias is consistent; do not fit another offset.
Between-draw matrix provenance remains unproven. Screen-Y lead re-parked;
24/24 CTest. Entry-slot restoration readback matches 280 bytes, but the
audit intercepts restoration only, not the initial imported writer.

**A6d24 (offline MEASURED, [check](scratch/phase-a6-full-frame-gate.md)):**
the unscored full-frame A6 clause now **fails**: unchanged actors-on is
`49,881,782 / 235,008,000 = 0.21225567640250545`, not <=0.02. Tube-only is
`49,888,588 / 235,008,000`. Fresh PNGs byte-match A6d3; this is not a new
regression. Fixed boxes still improve for actors 0/1 and worsen for actor 2
(0.10089786756453423 -> 0.11246451442529874). Independent integer/histogram
recompute, single-channel scorer check and 24/24 CTest pass. No named scene
cause measured; full controller/temporal pairing incomplete. Localize dominant
global error before more actor pixel-fit work; do not shrink the full-frame gate
to A4d's passing ROI.

**A7a (offline MEASURED, [check](scratch/phase-a7-wider-mask-localize.md)):** current
tube-only production wider-mask score reproduces A4d: **3031085/137520225 =
0.022041012512886742**, still >0.02. Mask and error reference come from the
durable A2 baseline/suppression pair, not A1 `original-0.png`. Predeclared 16×16
top five cells carry only **25.93%** of positive cell excess; four are in the
lower y=432 band. This falsifies five-cell concentration, not a connected-region
or causal claim. No missing-model attribution or production change. Only initial
has an identified matching wider mask in the checked retained evidence; do not
reuse it for other cameras. Independent full/cell histogram checks, fresh PNG
identity and 24/24 CTest pass. A7 remains open; no live allowance renewed.

**A7b (offline MEASURED, [check](scratch/phase-a7-lower-band-characterize.md)):**
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

**A7c (READ negative, [check](scratch/phase-a7-pause-caption-native.md)):** explicit
fourth-round native exception did not locate the caption renderer. `STR_PAUSED`
is stored at0x4C3C18, with a pointer at0x4C35E4 **offset0**, not+0xC as the
candidate pin prose claimed. This is a key-like identifier, not the display
resource. Known pause/resume stores re-read; one event/state function inspected.
Exact-address xref misses do not prove no indexed consumer. No draw/layout/alpha
rule or composition test accepted. Independent raw pins/bundle authentication
and24/24 CTest pass; re-park after count4. Future bridge must begin at the in-level
draw path, not another pause-event-handler or key-only search.

**A7d (partial READ, [check](scratch/phase-a7-draw-loop-caption-bridge.md)):**
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

**A7e (READ negative, [check](scratch/phase-a7-base-list-producer.md)):**
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

**A7f (READ negative, [check](scratch/phase-a7-caller-dest-offset.md)):**
17 direct `E8` calls to `0x4404FA`, and none has immediate `0x2018` in the
160 bytes before the call. `0x44CD06`'s only direct caller is the frame's
`add ecx, 0x2018`, and its builder calls pass that `this` plus `0x758`,
`0x77C`, or `0x224`. `0x44D348` / `0x44D389` are in `0x44D316`, whose only
direct caller does `add ecx, 0x2018` from `[ebp-0x10]` and then builds at
`this+0x758` / `this+0x77C`. `[ebp-0x10]` is not shown to be the frame
receiver. Control `0x45D25D` remains `add edx, 0x25C68`. A copy from
`G+0x25C68` into H was not searched. Gate unchanged.

**A7g (READ negative, [check](scratch/phase-a7-25c68-to-h.md)):** ten
`.text` encodings of immediate `0x25C68`, each an `add` then a call or
push. Six are inside `0x45CFF9` and use `[ebp-0x94]`, so they are
`G+0x25C68`. `0x45E33C` calls `0x44E150` on that object. The other four use
`[ebp-0x14]` or `[ebp-0x40]` in other frames; `0x4AD43E` calls `0x449130`,
not the `0x415140` used at `0x45BE4E`. No hit stores the pointer at
`+0x2018`. Independent immediate mutation `0x25C68` to `0x2018` fails and
restores; 24/24 CTest. Gate unchanged.

**A7h (READ, [check](scratch/phase-a7-pause-list-draw.md)):** the
G+`0x25C68` placement call pushes `[0x58CB44] - 2*metric` first
(`[ebp+0xC]` → `[this+0x18]`) and the `sar` difference second
(`[ebp+8]` → `[this+0x14]`). Neither immediate is `432` or `320`.
`0x44E010` gets dwords `0x3F613404` and `0xEA4A8C15`. `0x44E150` at
`0x45E342` has no stack arguments and runs only when `[G+0x572C] == 1`.
No constant in that trio lies in y `432..447`. The same function stores
`160`, `480`, `180`, and `260` and passes them to `0x47AD8C` on
G+`0x26000` before another `0x44E150`. Gate unchanged.

**A7i (READ, [check](scratch/phase-a7-rect-26000.md)):** `0x47AD8C`
walks from double `160` toward `480` (`fsub` at `0x47ADAA`). The other
pair, `180` and `260`, is passed through and does not meet y `432..447`
when left unchanged. `0x45E157` can instead add and subtract
`round(760 * ([G+0x25FF8] - 0.5)^2)` (`0x4B1810` is `760.0`) before the
same push, which can move the upper edge through the band. That double
is unread. Gate unchanged.

**A7j (READ, [check](scratch/phase-a7-25ff8-stores.md)):** three `fstp`
to `+0x25FF8`. `0x45CB39` writes `3.5`. `0x45CBB2` subtracts `2.0` when
`[obj+0x1CB7C] == 0`. `0x45CEB3` subtracts `[obj+0x1C968]`. `0x45CFF9`
calls those functions at `0x45D89D` and `0x45DA48` with `[ebp-0x94]`
before the scale read. A field of `3.5` leaves the rectangle setup
because it is above `1.5`. A field of `1.5` would scale the top edge to
`1020`. The value at the read is still open. Gate unchanged.

**A7k (READ, [check](scratch/phase-a7-1c968-source.md)):** the value
subtracted at `0x45DA48` is `(double)(int32)[G+0x25C64] / 1000.0`.
`0x4B1758` is `1000.0`, and `edx` at the copy is `[ebp-0x94]`. With the
integer at `0` and `V_init` at `1.5`, the scaled top is `1020`. The
integer is not a constant on this path. Gate unchanged.

**A7l (READ, [check](scratch/phase-a7-25c64-writers.md)):** two stores
of `[G+0x25C64]`. `0x45D09D` writes the return of `0x4657D1([G+0x557C])`
minus `[G+0x25C60]`. `0x4657D1` calls `[0x4B0220]` and subtracts
`[ecx+8]`. `0x45D96B` stores `100` (`0.1 * 1000.0`) when the dword is
already above `100` or `[G+0x1C9B3]` is nonzero. The pause-frame value
is not a file constant. Gate unchanged.

**A7m (MEASURED negative, [check](scratch/phase-a7-view-masks.md)):** four
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

**A7n (MEASURED negative, [check](scratch/phase-a7-input-view-masks.md)):**
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
original. **A7p ([check](scratch/phase-a7-unpause-view-masks.md))** fixed this
by walking 14 s unpaused before pausing: the settled camera hash is
`7e6e1a8a77c29f860f82dd8cde3f60c9f89ecf3f1352d3c2a6801cc9867d0fff`, and eight of
nine `MM_CLIMB_002` views then pass the wider mask (caption panel removed).
`zoom1` alone failed (`5473596/228864285`) until the user also excluded the ten
skill-panel buttons (x `[18,90)`, object `0x4C9208` stride `0x1B0`, y bands
`[12,44)`…`[372,404)`): with that exclusion all nine views pass
(`0.011169`–`0.019314`).

**A7q ([check](scratch/phase-a7-second-level-mask.md))** generalized the
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

## Frontend checkpoint: B1 boot-menu vtables observed

[B1d](scratch/phase-b1-menu-vtable-live-read.md), one no-input stock boot, three identical
screen-counter-3 reads: the active global `[0x5D59EC]` points to an object with vtable
`0x4B1E88`, and the observed list head `[0x5D59E4]` has vtable `0x4B2528` (B1c ctor#15,
one of its seven backdrop-sharing candidates). Getter bytes at `0x487F93` / `0x487FB3`
and next-link offset +4 at `0x487BCA` were independently re-read. Do not repeat the
seven-candidate identity search. This is not a traced draw routine, semantic proof of
the list's role, or a B1 pixel-gate pass. The saved poll cannot distinguish an unreadable
terminal next pointer from null; one observed node does not prove list cardinality.

## Menu text is not a FUN_004404fa caller

[B1j](scratch/phase-b1j-menu-callers.md): 17 direct `E8` calls to `0x4404FA`,
in 12 procedures. None passes UK indices 125, 126, 120, 3, or 43. `push 126`
does not occur in `.text`. The only `push 120` (`0x487511`) calls allocator
`0x49C1A8`. The five labels' draw function is still unread. Do not repeat this
caller scan, and do not treat those calls as the main menu.

## Main-menu labels are the table at 0x4C4214

[B1k](scratch/phase-b1k-menu-label-draw.md): the file dwords are 125, 126,
120, 3, 43. `FUN_0048ecdf` pushes that table with count 5. `FUN_0048aa5e`
walks the strings through `0x4404CF` on the `"^Font"` object at `[0x4CBF78]`.
The first row is 320. Horizontal placement is a signed half of the word at
`[0x58CB40]`, and that word's value is not in the file. No BMP filename is
on this path. Do not repeat the 17-call scan.
[B1l](scratch/phase-b1l-menu-x-and-sheet.md): the address immediate occurs
63 times and none writes the word. `^FONT.BMP` is not in the exe.
`0x474519` looks `"^Font"` up on `0x58CB48`. Do not scan those 63 sites
again, and do not invent the horizontal pixel.

[B1m](scratch/phase-b1m-runtime-font-read.md), one stock start plus an offline
repair: saved word `80 02` at `0x58CB40` is 640 (X320), but the highlight memory
bracket failed: B1b's heap address is not portable to another process. Font+8
is a manager-local index (36 in this ended process), not an archive ordinal.
**0x58CB48 is the embedded manager, not a pointer slot.** Dereferencing it
captures the vtable's `.rdata`, not the manager pool. The first missing sample
is pool base `[0x58CB84]`; source sheet/dimensions remain unknown. The one-start
allowance is spent; no live retry or B1 render/score accepted.

**B1n ([check](scratch/phase-b1n-font-sheet-discriminator.md)):** the checked
`^Font` table prefix is consistent with five exported font sheets, so
bounds/ink/blank predicates do not identify the source. No renderer or B1
score. B1i's blanket256-record extent is unestablished: its772-byte window
overlaps `_FontSmall`'s known header by92bytes. The menu ASCII cells and
Quit replay stand. B1n's `_FontSmall` negative reads an unproved range and
is not a resource rejection; both-archive completeness was not proved.
Next is the directive's conditional offline live-acquisition preflight,
not a guessed font or another glyph fit.

**B1o ([check](scratch/phase-b1o-font-binding-preflight.md)):** the fallback
preflight failed at its eight-body cap, **before launch**. `0x403720` and
`0x406500` return their input record address, not a string pointer field.
The loader at `0x47218A` has conditional Lo/Hi, BMP existence/fallback and
ALF paths, with key descendants unexamined. No actual source or highlight
accessor is established. Partial decoder tests do not close either gap.
B1 is parked under the directive; no fifth round or automatic use of the
unused conditional start. Own byte/bundle/mock checks and30/30CTest pass.

**B1p ([check](scratch/phase-b1p-texture-loader.md)):** the generic loader is read,
the actual `^Font` source is not closed. `0x47244B` tests for `INN_`/`OUT_`; `^Font`
matches neither, so `[0x5CD5F2]==0` tries `^FontLo` and nonzero tries `^Font`
(`^FontHi` is unreachable). Neither archive has a `Lo`/`Hi` member, and `lemmings.box`
alone holds `TEXTURES\^FONT.BMP` / `.ALF` (ordinals 633 / 632), so both cases converge
there **if** only the two archives are registered and nothing loose overrides. That
premise is unproved: `0x47E40D`'s non-archive route, `[0x5D5604]`, `0x47E89B`, manager
`[0x5D5608]` order and `0x4805B8` are unread, and `0x47F759` takes the first
registered match. Source is a BMP and an ALF companion. Not a unique-source claim.

**B2a ([check](scratch/phase-b2a-sequence-preflight.md)):** preflight failed closed at
the 12-body cap, zero starts. No byte-proved highlight accessor, native key producer
or safe confirm/map/level/pause/back route; `0x004C416C` stays a construction counter.

**B1q ([check](scratch/phase-b1q-archive-open.md)):** `0x47E23B` registers `PATCH.BOX`,
then `LEMMINGS.BOX`, then `LEMMINGS.BOX` from registry `CDPath` (`0x4C2A54` / `0x4C2A60` /
`0x4C2A70`); `0x47DD54` registers nothing; `[0x5D5604]` has no exact writer. `patch.box`
has no `^Font` member and the `CDPath` copy of `lemmings.box` is byte-identical, so B1p's
convergence survives. Not shown: that registration ran before the font load at `0x45019E`
(callers `0x450771`, `0x45BEA7` unread). Still no unique-source claim.

**B2b ([check](scratch/phase-b2b-highlight-keypath.md)):** `0x483212` sets `[0x5D5970]`
to "any key newly pressed" (a flag, not a key identity); `0x48896A` events 2..5 end in the
focus store `[container+0x10]` at `0x48F80C`. Candidate highlight
`rows.index([[menu+0x10]+0x10])` is a shape, not an accessor: no file-backed chain to the
`0x4B2528` menu and no key-to-event edge. The worker exceeded its body cap (about 17 of 12).

## Texture loader order (B1r, 2026-10-23, READ only)

Startup `0x450696` calls the archive registrar `0x47E23B` at `0x450771` before `0x486F60(.,7)`
at `0x450A46`, whose chain pushes `^Font` at `0x4403B5`. The registrar body is skipped when
`[0x5D5610]==0`; the setter is unread, as are the loose-file helpers and the working directory.
`^Font` -> `lemmings.box` BMP+ALF stays conditional. Evidence: `docs/scratch/phase-b1r-load-order.md`.

## Live `^Font` read (B1t, 2026-10-23, OBSERVED, negative)

One stock start under `trace_font_source.py --launch` (`starti`, one hardware execute breakpoint at
`0x47E4AD`, 90 s): no hit, so the `^Font` source is **not observed**. This does not distinguish the loose route,
a load before the breakpoint was live, a debug register that missed the loading thread, or a condition that did
not match. `starti` itself works on the stock exe. Evidence: `docs/scratch/phase-b1t-font-live-read.md`.

## Both-arms preflight (B1u, 2026-10-23, OBSERVED on a throwaway, READ on the exe)

Hardware breakpoints set at the `starti` stop do not fire in the main thread (throwaway exe, GDB 17.2); only later
threads. B1t's single breakpoint was probably blind for that reason. Arming from a one-shot software stop at the entry
point `0x49D518` works; it is a debugger probe beyond the two hardware breakpoints, so it needs authorization. A third
route (`jne 0x47E598` when `[0x5D5604] != 0`) executes neither arm. Not run against the stock exe. Evidence:
`docs/scratch/phase-b1u-font-both-arms-preflight.md`.

## Pause exit keys (B2g, 2026-10-23, READ only)

In pause (`0x45D459`) a newly pressed P (action `0x13`) **or** ESC (action `0xE`) stores `ui_state` 0 at
`0x45D508`; the key poll `0x483212` sets a per-action flag only on a new press, so the code has no hold-length
test. This agrees with B2e's one hold-25 sample and does not support the preserved short-ESC quirk, but the quirk
is neither confirmed nor dropped until the live sweep. ESC in a level stores `ui_state` 2 and builds screen `0x1F`
(not the map). Evidence: `docs/scratch/phase-b2g-esc-sweep-preflight.md`.

## Live ESC sweep (B2h, 2026-10-23, OBSERVED)

ESC held 2, 5, 10, 25 and 40 frames in pause: `ui_state` 0 every time, counter 5, list empty. The short-ESC quirk
did not reproduce and is retracted. ESC in a level: `ui_state` 2, counter 5 to 7, one head node with vtable
`0x4B26B0`; O returns to pause. `+8` on a stacked node and level-to-map remain unobserved. Evidence:
`docs/scratch/phase-b2h-live-esc-sweep.md`.

## Level to map, handler to store (B2j, 2026-10-23, READ only)

Screen 0x20's handler `0x486510` (vtable `0x4B1CB0` slot 11) calls slot 3 `0x4886E4` with selector 0x23 when the
event id equals `[screen+0x48]` (`0x486599`); the factory builds ctor `0x49AA90`, vtable `0x4B3EB8`, which is the head
node B2h saw after the map-node click (so 0x23 = map is an inference). The 0x1F dialog builds 9 / 0x19, never 0x23;
`[level+0x55A2]` has no setter. The input that produces the event id and the trigger that builds screen 0x20 are
unread; no key or click path is named. Stacking candidate: `0x4886E4` skips the `0x4884F4` call on the old head, but
whether the old node survives is unread. Evidence: `docs/scratch/phase-b2j-level-to-map-preflight.md`.

## Level to map, input end (B2k, 2026-10-23, READ only)

The `[screen0x20+0x48]` value is a button object pointer stored by the builder `0x486335` (table `0x4C3414`) through
`0x488A92`, not a control id from the ctor. Producer: button vtable `0x4B1E30` slot 1 `0x48FCDD`, calling parent slot 11
with the widget, fed by the key-state table `0x483212`. Screen 0x20 is built when the main-loop mode word `[0x4C6A08]`
is 4 (`0x45DF5C` -> `0x45E086`); its writer is unread (no direct immediate store in `.text`). The old head node
survives `0x4886E4`. No key or click path is named; no start. Evidence: `docs/scratch/phase-b2k-level-to-map-input-end.md`.

## B2 menu object chain (B2a-B2c, 2026-10-23, READ only)

`[0x5D59E4]` is the screen-list head (`0x487FA0` writes it; `0x487CC0` stores `[obj+4]`). The
main-menu object (vtable `0x4B2528`) is built only for screen id 7 (`0x48744C`, `0x5C` bytes,
sole ctor caller `0x487469`) and is found by walking `+4` from the head. `[0x5D59E8]` has one
writer (`0x486ECE`); `[0x5D59EC]` is the chrome object, not the menu. Open: key -> event
producer for `0x48896A`, and the writer of `[menu+0x10]`. `0x487C60`/`0x487C80` store
`+0x24`/`+0x28`, not `+0x10`. B2 is parked (PLAN Phase E); no stock start used.
Evidence: `docs/scratch/phase-b2c-menu-chain.md`.

## Pause text at 0x44CD06

[C2a](scratch/phase-c2a-pause-timer.md): `0x44CD06` fills `H+0x758` with
string index `0x44` at (520, 15), `H+0x77C` with index `0x45` at (520, 45),
and `H+0x224` with `[0x4CBAAC] / 60` at (560, 430). The below-10 format is
`"%02d:0%d"`; otherwise `"%02d:%02d"`. The font pointer `[0x4CBF78]` is
registered through `0x4403A7` as `"^Font"` / table `0x4BF968`. The index
characters are a runtime lookup, not the file string `"Unknown String"`
except on the out-of-range path. The file language slot is 2 (`Uk`).
[C2b](scratch/phase-c2b-pause-index-text.md): both `LANGUAGE\UK.LTX` copies
store `Out` at index `0x44` and `Saved` at index `0x45`. These lists are the
counters, not the word `Paused`. The C2 gate was not attempted.

## Hard-won rules of thumb

**A6f failed acquisition** ([check](scratch/phase-a6-fresh-pause-gate-failed.md)):
one start reached entry pause but failed before walking/PNG; no fresh A6 gate
measurement. Patching an importing module's `write_mem` does not intercept a
function's globals in its defining module: the only retained slot event was the
restore, not the initial `goto_level.select_level` write. Helper-only mocks missed
this integration boundary. Main CLI candidate removed; baseline restored, 24/24
CTest. Start allowance exhausted; no automatic live retry. A6e actor-2 failure stands.

**A6f repair3 failed acquisition** ([check](scratch/phase-a6-fresh-pause-gate-failed.md#repair3-renewed-attempt-also-failed-acquisition-before-game-creation)):
the renewed attempt failed before game creation: copying the capture script without
its `OUT/dgVoodoo.windowed.conf` dependency makes `read_bytes()` fail before the
empty-file fallback. Independent pure-read reproduction supports this diagnosis;
the PowerShell wrapper lost the full traceback. Finalizer's config rewrite is not
in `writes: []`; hash equality supports restored contents, not a complete write log.
No fresh gate score or production change; own build and 24/24 CTest pass. Stop-on-failure
forbids automatic retry despite no new stock process. The stricter old audit was waived.

**A6f repair4 (offline MEASURED, [check](scratch/phase-a6-fresh-pause-pixel-result.md)):**
one carried-over authorized start produced a fresh image/table: ROI **708159/45441000**
passes, and all three scored components improve. Own rerenders and isolated input-loader
bypass discriminate the optional `--walkers` path; exact source restored, 24/24 CTest.
**Not A6 acceptance:** `same_pause` is a literal with no second stability snapshot;
`actors[:3]` omits full-list count/identity, and box component rank is not actor-row
identity. The worker corrected those labels offline without changing any scores.
Full frame remains 0.2122073 (diagnostic). Old actor-2 failure stands; no slope sample
or cause established. Install hashes match, no game/tasks remain, start consumed.
Further live work needs explicit authorization and a full-list/stability brief;
do not revive the waived initial-write audit or call passing arithmetic full qualification.

1. **One cell is one world unit.** Never multiply a height or distance by
   `PIXELS_PER_UNIT`. The factor-of-2 lattice raster is retired, and doubling a world-unit
   delta is the single most repeated mistake in this project's history — it survived in the
   plan's own prose for weeks after being disproved.
2. **Expect the wrong *shape*, not the wrong number.** Every provisional value that turned
   out wrong was wrong structurally: three "intervals in ticks" were animation cycles; a
   blocker "list scan" was a terrain field; an "explosion radius" was a stored bitmap; a
   "wait of 2 ticks" was a 14-frame animation; an exit "rectangle" was an object AABB.
3. **Halving a classic-Lemmings value leaves a guess.** `maxStepUp` and `lemmingHeight` came
   out 2 and 3, not the halved 3 and 4. The blast clears 37 cells, not the 81 a halved
   radius-5 circle would.
4. **A plausible series can come from the wrong object.** A billboard-quad probe was fitted
   to "a lemming" and produced clean, plausible numbers for **nine rounds** before anyone
   noticed the quad was not a lemming. Two published measurements had to be retracted.
   Read lemmings from the **actor list at `0x004CBB18`**.
5. **Check `.data` rawsize before citing a byte.** `.data` has va `0x004BF000`, vsize
   `0x1185B8`, but **rawsize only `0x7000`** — file bytes exist only to `0x004C6000`. A
   round once asserted a gate byte of `0xFF` that the loader actually zero-fills.
6. **Mutate one thing alone and demand a behavioural failure.** A test whose expectation is
   derived from the field it tests passes at any value. Reverting a whole round at once
   proves only that *something* was tested.
7. **A clean negative is a result.** So is "this does not appear in any reachable level".
8. **Preserve quirks.** RETURN only when the menu highlight is exactly 0, a
   short ESC that fails to leave pause (retracted 2026-10-23, B2h), the drawn crosshair disagreeing with the tested
   cursor. All real. Do not smooth them out. Menu language is not a quirk: the German
   menus were an early harness side effect (user, 2026-10-22); the English install is fine.

## The data formats — solved

- **Container**: `lembox`, LRZ-compressed, base plus patch overlay, read from the user's
  install at runtime. `lrtool` lists, extracts and converts. All 10,376 textures decoded and
  visually verified; all 73 retail `.WAV` decode with a load-time resampler.
- **`.SPR` sprite sheets** carry temporal entries, each containing ordered directional
  images. The former “walk 12 / block 8 / fall 7 views” claim reversed the axes;
  see the A3 checkpoint above. Direction counts are five for walk/fall, eight for
  block/cartwheel, one for shrug across all six lemming resources. Frames reference
  sheets `$LemSheet2`..`$LemSheet10`. Sheet
  pixel size is recovered from the rectangle and its normalised form, and **only the
  Y-then-X reading gives a sheet's real size** — transposed axes were a real bug here.
  Only the six lemming sheets carry a palette, and all six share it.
- **`.LVL` chunk order**, anchored at the literal `"Cylinder"` chunk, lands on the collision
  grid in all 102 levels: `u32 8,"Cylinder"` / `u32[5]` zero / `u32 height` (96 everywhere) /
  `f64 0.36` / `u32 N,u32[N]` surface indices (49858) / `u32 R, R x 12 bytes` / **`u32 C,
  u32[C]` the collision grid, C == 256*height** / `u32 B, u8[B]` (12288).
- **The collision mask is stored, not computed.** 256 x 96 `u32` cells, length-prefixed,
  byte-identical to the grid the running engine collides against (verified: 98,304 bytes at
  offset 240,011 of `MM_CLIMB_002.LVL`, zero bytes differing).
- **The surface lattice, `PolyList` and `SurfaceMesh` are the *render* input, not
  collision.** `rasteriseSurface` must never be in a collision path. This one cost a whole
  phase: our rasteriser's sparse masks were never the terrain.
- **`RELEASERATE.PC` and `LEVELMAP.DAT` hold literal seconds.**

### Cell flags, every one with a reader instruction behind it

| bit | meaning |
| --- | --- |
| `0x0001` | solid and destructible (builder's `or 1` at `0x004101F8`) |
| `0x0002` | indestructible (tested first, never cleared) |
| `0x0004` / `0x0008` | no bash right / left |
| `0x0010` / `0x0020` | no dig upright / inverted |
| `0x0080` | not landable — the faller skips it (`0x0041A99A`) |
| `0x0040` | water / acid drown (rounds 14–15) — side-effect `0x2f` tests it at `0x00415386`, attaches drown `0x004B0570`, ~2.0 s. Acid is the **same** path, no separate state |
| `0x0100` | authored-only (round 31) — 3 levels, no runtime setter. Sets `lem[+0x5f]` |
| `0x0400` | door / gate stamp (round 31) — vtable `0x004B0F00`, `|0x403` / `&0xfbfc` |
| `0x0800` | blocker field — **runtime only**, 0 of 2,506,752 shipped cells |
| `0x1000` | trampoline bounce (round 13) — fall-land `0x00416560` attaches bounce `0x004B0810` *before* splat. **0 of 102** authored; painted at runtime by trampoline enter `0x0041B838` |
| `0x8000` | anti-grav pad — exactly the 18 levels embedding `anti_grav.a3d`, audited twice |
| `0x0200` / `0x2000` / `0x4000` | object zones — see below |

Zone bits `0x200` / `0x2000` / `0x4000` are **splat exemptions** (round 29): fluid `+0x128`
0/1/2 stamps them, the hatch sets `[+0x4c]` via `0x004BFFC8`, and land `FUN_00416433` skips
the splat for `0x200` with `[+0x4c]==1` and `0x4000` with `[+0x4c]==2`. Lava has no arm.

### Geometry and coordinates

- File grid is **Y-up**; `sim::Terrain` is Y-down. File row `r` maps to terrain row
  `height - 1 - r`.
- The engine turns a world Y into a file row by **truncation** — its float-to-int helper
  `0x0049C5AC` sets x87 rounding to chop — **not** `floor(worldH - y)`. The two differ by one
  cell for an integer Y, and that one cell has bitten twice.
- Actor `+0x08` is not a 3D position: the model position `+0x108` is the cell-mesh
  interpolation `0x43D56C` of it (8-unit columns `& 0x1F`, 16-unit rows, Z = shell blend).
  See [A6a](scratch/phase-a6-actor-pose.md). Writers: [A6d10](scratch/phase-a6-sim-plus8-writer.md).
  `+0x360` added by `0x42F22E` is a facing bias (0 / −1), not a cell origin.
- Hatch spawn is the two int16 at hatch `+0x9A` / `+0x9C` (plus count `+0x98`, type `+0xB0`),
  **not** the entrance model's translation: on all 144 retail hatch records the stored Y is
  the translation's world Y minus 2, and that difference is stored, not computed.
- The exit save test is the **general object-overlap AABB** at `0x00430BA1`, inclusive on
  every edge (mins `test ah,1`/`jne`; maxes `test ah,0x41`/`je`), against the lemming's raw
  `(X,Y,Z)` at `+8`, with Z a fixed `[0,1]` slab. The box is `[round(ang x width)±1] x
  [ty−9..ty−6]` — 3 columns by 4 rows, twelve cells — and round 12 dumped it on a level of
  each of the **four** retail exit models (`Exit.a3d` 84, `Exit_water.a3d` 14,
  `Exit_any.a3d` 9, `Exit_acid.a3d` 1): all the same box.
  **The column centre is `round`, not `floor`** — round 11 said floor, and 46 of 111
  placements disagree, including `MM_FLOAT`.
- `0x00430BA1` is the engine's general gadget-overlap predicate with **19 call sites**.
  Identified: anti-grav `0x0041C1E3`, the exit, teleport `0x00435467` / `0x004354CB`,
  SpeedPad via `FUN_00432742`, lever `0x00436D30`.

## The simulation — measured, and worth keeping

**The "5 Hz, measured three ways" claim below is withdrawn as of D1m (round 2026-10-21,
Phase D) — see [D1m](scratch/phase-d1-stepper-clock.md).** Two of the three old
measurements rest on the billboard-quad centroid method this project's own guardrails
already flag as unreliable for a closely related claim (the climbSpeed quad-lock
retraction, round 10); re-run against the actor list, the same level shows the *opposite*
pattern (position changes on 97% of consecutive 20 ms samples, not the ~10% a flat 200 ms
step would produce). The third measured the hatch's release gap, not the lemming stepper,
and D1d already found the hatch's own countdown is itself continuous. D1m's own bounded
disassembly read of the walk stepper (`0x0041BDD4`) found a mutable, re-checked,
byref time-budget loop — a shape with no reason to exist under a fixed step — and a
zero-fitted-parameter continuous replay collapsed the fall/landing-tick mismatch count by
76–100% against three live captures. **The conclusion is not yet "the tick rate is
wrong"** — `World::tick()` is still called once per 200 ms and nothing below has been
re-measured or re-implemented — but the premise that lemmings *step* discretely at 5 Hz,
rather than moving continuously and being *sampled* at 200 ms, no longer has supporting
evidence and has active evidence against it. D1n (queued) is the implementation/re-score
round; until it lands, read every `Rules` field below as "the per-tick arithmetic that
reproduces the original when driven once per 200 ms," not as a confirmed hardware step
rate.

The tick was **measured three ways** as 5 Hz (200 ms) and not frame-locked — see the
withdrawal above. It had been guessed at 20 Hz, which made everything timed in ticks run
four times too fast; that specific correction (20 Hz → 5 Hz bookkeeping) stands regardless
of D1m, since the alternative is now "continuous, sampled at 5 Hz," not "20 Hz."

**Durations are animation lengths, not tick counts**: `frames x 0.06 s` (the double at
`0x004B1940`), fired when the animation's integer percent reaches a stored frame. A single
"interval in ticks" cannot reproduce bash acting every 7 or 8 ticks averaging 7.2.

Every constant below is a read of the shipped bytes **and** a live series:

| | value | |
| --- | --- | --- |
| `walkSpeed` | 1 column / tick | |
| `climbSpeed` | 1 cell / tick | the 5.0 at `0x004B10B0` |
| `fallSpeed` | 4 | `20.0 * 0.2 s` |
| `floatSpeed` | 2 | **not a constant** — a division of `fallSpeed` by the 2.0 at `0x004B03F8` |
| `floatAfter` | 10 | umbrella latch |
| `floatOpenMs` | 480 | halving is delayed: `umbopen` is strip 12, 8 frames |
| `splatDistance` | 30 | 29 survives, 31 dies |
| `maxStepUp` | 2 | the probe is unrolled exactly twice |
| `lemmingHeight` | 3 | ceiling bias only |
| `climberTopOut` | 2 | `0x00413B22` subtracts `[+0x44]` once where the ceiling arm subtracts three |
| `hoistCycleMs` | 840 | strip 8 `flip`, 14 frames. At a 200 ms tick, 800–999 are indistinguishable |
| `hoistLift` | 2 | X stays on the wall column |
| `brickLength` | 2 | at percent 81; 12 bricks then a shrug |
| `blockerFieldHeight` | 4 | one column x four rows |
| `bomberFuseMs` | 5000 | then Ohno, then a further 1020 ms, then the crater |
| blast | **stored 7x7 stencil** at `0x004B1528`, 37 of 49 cells | |
| anti-grav | cell bit `0x8000` negates `[lemming+0x44]` via cartwheel `0x004B0CA8` type `0x2b` | |

**The walk probe snaps X to an exact integer before every test.** `0x0041BECE` truncates and
`0x0041BEDC` stores the integer back, then `0x0041BEE6` probes; `0x0041BF61` picks the next
boundary as `trunc(X)+1` facing right and `trunc(X)` facing left. Truncation gives a shared
edge to the right-hand cell, so a right-facing walker stops **on** the wall column and a
left-facing one at wall+1. This is why climbers sit at exactly X 232.000 / 128.00 / 64.000.

**Formerly a known divergence, fixed in Phase D (D1f, 2026-10-21)**: a walker that *was*
facing Left, turning to face Right at a wall, used to cost our sim a tick the original
never loses (`0x0041C67E`) — the original's turn arm returns 0 so its slice loop
(`0x0041BDD4`) keeps going in the same tick and the walker steps once more immediately.
`World::stepWalker` now does the same, gated on the facing the walker had *entering* the
call (not the facing a turn just produced), and only for that direction — a walker that
*was* facing Right already consumed the tick moving onto the wall column, so it must not
get the extra step. One `testBlockerTurnsWalkers` literal moved (131→132) as a direct,
justified consequence; the already-good "was Right, turns Left" case is unaffected and
still matches. See `docs/scratch/phase-d1-left-turn-fix.md`.

## The tooling

- **Static disassembly**: `python tools/harness/read_steppers.py --at 0x00430BA1 --len 0x90`
  (also `--states` for the 24 animation sites and 46 vtables, `--cell-flags`). Capstone.
- **REA** (Ghidra-backed decompilation) — see `docs/REA_SETUP.md`. This is the new primary
  tool for reading code we cannot measure live, which is most of rendering.
- **Live reads**: 40+ scripts under `tools/harness/`, all re-runnable, driven through
  `tools/dinput-shim`. `measure_steppers.py --compare walker|climber` is the per-tick oracle.
  `read_lvl_grid.py` and `read_grid.py` read the stored and live grids.
- **Screenshots**: `screengrab.py` captures via a **desktop BitBlt** — `PrintWindow` returns
  all black because dgVoodoo presents through a D3D11 swapchain. Needs the game foreground.
- **`compare_tick.py` is superseded** — it fits a billboard quad that is not a lemming. Do
  not quote its numbers.

### Environment traps, all learned the hard way

- Build through `.\scripts\Invoke-Ucrt64.ps1 "<cmd>"` (MSYS2 UCRT64). Test binaries need
  UCRT64 `bin` on `PATH` or they fail to load silently.
- **Do not put UCRT64 `bin` on `PATH` for `tools/harness/*.py`** — it shadows the default
  Python and `capstone` disappears.
- `tools/dinput-shim` is 32-bit mingw32, not UCRT64.
- `lrstate.attach` returns `(handle, pid)`. `read_lvl_grid.find_grid` returns
  `(offset, cells)`.
- **Skill assignment must go through the panel.** Proof is the stock count at `0x004C91D8`
  **decrementing**, not a sprite change. Physical indices ([C1a READ](scratch/phase-c1-panel-indices.md)):
  0 climber, 1 parasol, 2 bomber, 3 blocker, 4 builder, 5 basher, 6 miner, 7 digger;
  8 fast-forward and 9 Armageddon are commands, not ordinary per-lemming skills.
  `0x45A4DD` is eligibility; `0x45B570` is a separate factory with a latent special
  index8 path. Do not infer physical index8 behavior from its `questionmark` art key.
- **Cap live sampling.** An uncapped single-lemming run at ~300k `ReadProcessMemory`/s killed
  two game processes. `--hz 2000` is safe.
- `goto_level` **can silently fail** — it once reported entering one level while the loaded
  path stayed another, and the dump was of the wrong level. Read back the loaded path.
- `goto_level.focus()` has crashed dgVoodoo on this machine.
- **Correction to the retired `2 = pause` shortcut:** P gameplay pause is `ui_state 1`
  (store at `0x0045D226`), and P resumes `0` (store at `0x0045D508`). The clock was
  independently observed stopped/running/stopped; changing only the harness's pause
  classification to the old value failed, then its exact bytes were restored and the
  suite passed. Do not conflate gameplay pause with the separate menu state.
  Frontend screen id `0x004C416C` remains: 3 menu, 4 confirm, 5 map.
- If you write to the live collision grid (a legitimate, proven probe), **log every cell and
  restore all of them**.
- **Open lead, not a diagnosed cause (D2e, 2026-10-22):** two consecutive stock-exe crashes
  (`0xC0000005`, access violation) on back-to-back rounds, each earlier in the launch
  sequence than the last (main-menu foreground polling, then the boot-wait/first-`attach()`
  window). Neither round's own evidence names a faulting module, and a facilitator-only
  read-only Application-event-log/WER pass covering both crash windows found no matching
  `AppCrash_Lemmings*` report for either specific incident. Two things that check *did*
  find: this exe has a long pre-existing history of crashing on this machine in unrelated
  prior sessions (`WER\ReportArchive`), and the host was cycling `Microsoft-Windows-Winlogon`
  logon/logoff and `CodexSandboxService.OpenAI.Codex` restarts roughly every 40-70 seconds
  through both crash windows. The correlation is unproven, not a cause — reported so a
  future round reads the event log immediately after any crash rather than re-discovering
  this from scratch. See `docs/scratch/phase-d2-rescore-crash.md`.

## Useful strings the engine keeps

Cue and UI element names, packed null-terminated in `.data` from `0x004C0C7C` (inside
rawsize, so readable from the image):

`Letsgo`, `Yippee`, `Chink`, `Explode`, `Bubble`, `Scrape`, `Ohno`, `Changeop`, `Splat`,
`ForceF`, `Die`, `FallSplat`, `Lever`, `Mine`, `MineMetal`, `Rumble`, `AntiGrav`, `SpeedPad`,
`Action`, `Balloon`, `Fire`, `Witch`, `Bits`, `GreyIcons`, `BlackIcons`, `AdditiveIcons`,
`Axe`, `Smoke`, then cursors `_curNoLem`, `_curLem`, `_curNoLemLeft`, `_curLemLeft`,
`_curNoLemRight`, `_curLemRight`, `_curEnt`, `_curExit`, `^curGrab`, `_curMseDwn`,
`+shadow_new`, `^tab`, `^tab_selected`, `^out`, `^saved`, `^time`.

`^saved` / `^out` / `^time` are the HUD counters, and `_curExit` means the engine has its own
exit hover test — both matter for Phase C.

**Every cue in that list has now been chased down** (rounds 18–28), and most were clean
negatives. Do not re-investigate them; see the gadget table below.

**`GreyIcons`/`BlackIcons`/`AdditiveIcons` are chased (C1b, round 1): each sheet decodes to
exactly one sprite, not ten icons.** `SPRITES\GREYICONS.SPR`→`bomb` (18×25, one frame),
`SPRITES\BLACKICONS.SPR`→`questionmark` (16 near-duplicate frames, max 20×19),
`SPRITES\ADDITIVEICONS.SPR`→`armageddon` (13 frames, max 15×18) — independently re-decoded
twice, from the production reader and from a from-scratch parser. All three, plus `Bits`,
`Axe` and `Smoke`, are registered by the **same** function, `FUN_0045e51f`, through the
**same** generic sheet-loader `FUN_00446040` round 23 already named for `Axe`; that
function only loads handles, it does not draw. These three read as per-button **badges**
(questionmark→fast-forward's multiplier cycle, armageddon→the detonation animation,
bomb→plausibly a greyed/disabled overlay), not the ten base icons' strip — do not reuse
them as the panel's base art. `SPRITES\BITS.SPR` is the lemming's death-gib body parts, a
sibling clean negative. The ten base icons' actual sheet/draw call is still unlocated
([C1b](scratch/phase-c1-panel-icon-render.md)): the hover/selection chain
(`FUN_00454d6c`→`FUN_004532e2`→`0x45cff9`) contains no draw call; the real draw sits behind
an untraced per-skill vtable. Do not repeat the archive/PE-resource search — it was
exhaustive.

**C1c (round 2): the per-skill vtables don't draw either, and `LemmingsLO` is unconfirmable.**
All nine skill-constructor vtables' ~50 virtual-method bodies are simulation physics,
eligibility, cell-flag writers, or (several skills) a call into `FUN_0042fd25` that sets the
**lemming's own in-level action-pose animation name** — the input to the actor-rendering
pipeline (A4r/A4t), not the panel. `SPRITES\LEMMINGSLO.SPR`'s name string does not exist
anywhere in the exe's string table, so that lead cannot even be xref-searched. `panel_boxes`
(`0x4c9208`) has exactly 9 xrefs, all inside the hit-test function `0x454d6c`-`0x455656` — no
second, draw-side reader exists anywhere in the binary; do not re-check this. The per-level
main loop `FUN_0045cff9` (sole caller `0x450696`) calls `FUN_00414920` (confirmed real draw
primitive) via `FUN_00454a85`, but only for one resource-indexed overlay quad/text label per
frame, never a ten-slot array. One concrete unresolved lead remains for whoever resumes this:
`FUN_00454a85`'s closing indirect dispatch `(**(code**)(**(int**)(levelScreen+0x3c)+4))()` on
an unidentified object — REA's own "unresolved indirect call" limitation, not yet a target.

**C1d (round 3): the ten-widget array is found; full dispatch and the icon index are
still open.** `levelScreen`'s constructor (`0x452bc2`) builds exactly ten identical
sub-objects at stride `0x238` (`+0x70, +0x2a8, +0x4e0, +0x718, +0x950, +0xb88, +0xdc0,
+0xff8, +0x1230, +0x1468`), one shared constructor `FUN_004576b0` (used nowhere else),
one shared six-slot vtable `0x4b1774`. Slot+4 (`FUN_00457868`) is a genuine icon draw:
resource lookup by the widget's own `+8` index via `FUN_00414a10`, UV rect from the
widget's own `+0x21c..+0x228` divided by the resource's `+0x14/+0x18`, drawn via
`FUN_00414920`. `levelScreen+0x3c` is fixed to widget 0 at construction; the dispatch
`FUN_00454a85` reaches only that one widget per frame. No resolved caller reaches the
other nine, and no writer of any widget's `+8` was found. Do not re-derive any of this;
resume only on the two named open threads (a stride-`0x238` literal search for a possible
unresolved-indirect dispatch loop; a field-store search for `+8`).

**C1e (round 4, cap exception): both threads are clean negatives — parked.** An
exhaustive `search_bytes` scan for the `0x238` stride literal found exactly 7 matches in
all of `.text`, none a dispatch loop and none near `levelScreen`'s constructor. A second
exhaustive scan (all ten widgets' absolute `+8` offsets, plus a full re-read of the five
most-likely candidate functions) found no writer of any widget's icon index anywhere.
Both independently re-verified exactly. **Parked in PLAN.md Phase E** — closing this
needs a live memory read (not more static analysis); do not repeat either search.

## Gadgets — all settled, rounds 9 and 13–31

Implemented in `src/sim` and not to be re-investigated:

| gadget | verdict |
| --- | --- |
| **Anti-grav** | cell bit `0x8000` + an anti-grav object; negates `[lemming+0x44]` through cartwheel `0x004B0CA8` type `0x2b`. The bit alone does nothing |
| **Trampoline** | cell bit `0x1000`, painted at runtime by trampoline enter `0x0041B838`; bounce `0x004B0810` attaches on fall-land **before** the splat arm. `pressure_pad.a3d` is *not* the source |
| **Water / acid** | the same path — cell bit `0x40`, side-effect `0x2f`, drown `0x004B0570`, ~2.0 s. No separate acid state; type `0x2e` unused in retail. `Entrance_water` / `Entrance_acid` are sheets and zones, not the hazard |
| **Teleport** | object-only, no cell bit (18/18). `teleport.a3d` vtable `0x004B1250`, partner via `+0x260` from id `+0x98`, attach `0x28`→`0x2c` with a 4-tick countdown, relocate `0x0043503C` (Y+2.0). AABB 12x8, facing preserved |
| **SpeedPad** | object-only, no cell bit (9/9). `speedup_two_way.a3d` vtable `0x004B1178` → FASTER `0x004B0A38` for 5.0 s; `0x0042E2BF` doubles the main state while boosted. AABB 2x6 |
| **Lever** | object-only. `lever.a3d` vtable `0x004B1288`; overlap `0x00436D30` facing-gates against `+0xb8`; toggles and fans type 4/5 messages. **No lemming attach** |
| **Splat exemptions** | the zone bits, round 29 — see the cell-flag section |
| **Inverted steppers** | after an anti-grav flip, step-up / climb / dig match upright with gravity inverted; dig refuses on `0x20` when `[+0x44] == -1` (`0x00415A96`) |

Clean negatives — cues that are **not** gadgets, so do not go looking for one:
`Balloon` and `Fire` and `ForceF` have **zero** play references in retail; `Witch` is a pause
easter egg on `e10_kf_1` only; `Axe` is a HUD sprite sheet; `Bubble` is SFX on fluid type
`0x23`; `Scrape` is the dig/bash refusal SFX; `Rumble` is an object state-commit SFX;
`Changeop` is the skill-panel and options-slider SFX; `Die` is SFX on type-`0x23` enter.

## What the hard reset was for

**Thirty-one research rounds** measured the simulation correctly — physics, all nine skills,
every gadget — and left the *visible* game wrong: it booted into a flat 2D debug view, drew
every lemming from sprite view 0 regardless of viewing angle, and showed a placeholder menu
in our own font. The simulation was never the problem, and none of it is being thrown away.
The plan is now about what you can see — `PLAN.md`.

**Note on paths**: the 42 round notes moved to `old/docs/scratch/`. Comments in
`src/sim/World.h` and elsewhere still cite them as `docs/scratch/phase3-*.md`; read that as
`old/docs/scratch/`.

## Live `^Font` source (B1v, 2026-10-23, OBSERVED)

One stock start with both arms armed from a one-shot entry-point stop (the exe tolerated the int3): two hits on the
archive arm `0x47E4AD`, `Textures\^Font.bmp` (77 bytes at 44312738) and `.alf` (61440 at 44251298), gate `[0x5D5610]`=1,
zero loose `^Font`. Matches `lemmings.box` ordinals 633 / 632 read independently. Exactly one source up to the second hit;
a later third source, TLS callbacks and the archive-only route are not excluded. Evidence: `docs/scratch/phase-b1v-font-live-read.md`.

## Main-menu render (B1w, 2026-10-23, OBSERVED + READ)

Highlight-0 main menu from the observed `^Font` sheet and the read charmap scores 0.005244 full-frame against the B1b
reference (gate 0.02; own independent re-score). Labels are **centre-anchored**: `0x48A720` tests flag `0x40` at `0x48A747`
and offsets by half the width, so the brief's left-edge guess was wrong. Panels are measured from the reference, not read;
the backdrop is up to 7 levels lighter than the reference (cause unmeasured). Evidence: `docs/scratch/phase-b1w-menu-render.md`.


## HUD counters and timer (C2c, 2026-10-23, READ only)

Positions 520,15 / 520,45 / 560,430 are absolute pixels (`0x44DF40`). The count text (`%d` at `0x4C01F4` / `0x4C01F8`, Saved
from word `0x4CBAA4`) goes into the same lists as the labels (`0x44D316`, caller `0x444B98`); `0x44D22C` (caller `0x44318D`)
re-places them with right end near x 620, Out at y 15, Saved at y 50. Saved is white below word `0x4CBAB0` and (255,63,63) at
or above (R,G,B order unread). Open: replace versus append, the Out source, default glyph colour. Evidence:
`docs/scratch/phase-c2c-counters-timer-read.md`.

## Menu panel geometry (B1x, 2026-10-23, READ, partial)

`0x48B1C7` is a constructor tail, not the panel draw. The panel object (vtable `0x4B1E28`, ctor `0x489999`) holds a fill quad
(`obj+8`) and a border quad (`obj+0x1B0`); `0x489B39` insets the fill by 2 and the border by 1 from the rect `0x48A190` builds from
fields `+0x34..+0x48`. Those fields, the colour meaning of the ctor's 255.0 / 64.0 doubles and the highlight setter are unread, so
the renderer's panel values stay measured. Evidence: `docs/scratch/phase-b1x-panel-draw-read.md`.

## HUD counter digits replace the words (C2d, 2026-10-23, READ only)

`0x4404FA` zeroes the list count before it builds, so `0x44D316` leaves only digits in the Out and Saved lists. Out is the sum of
words at `this+0x5070 + idx*2` over the list at `this+0x1FB0` (`0x444ABC`); `0x4CBAB0` has no absolute-address writer. The Saved colour
object holds 255, 63, 63 (at or above target) or 255, 255, 255 in memory order; R,G,B naming and the default glyph colour are
unread. Evidence: `docs/scratch/phase-c2d-counters-open-reads.md`.

## Pause caption inputs (C2e, 2026-10-23, READ only; corrects G)

`G` in the A7 chain is the static `0x4C6A08`; the level screen `0x4CBFB0` is the pointer at `G+0x55A8`. The `0x47AD8C` rect is
gated by `[G+0x26024]`, which is cleared once `[G+0x25FF8]`, decremented only while `ui_state == 0`, drops below -1.0 (about 2.5 s),
so after a 14 s walk the rect is predicted off in the paused frame and A7k's 1020 was an intro value. The caption edge probably comes
from `G+0x25C68` (placement via `0x47ACF9` / `0x47AC50`, unread). Evidence: `docs/scratch/phase-c2e-caption-preflight.md`.

## Menu panel colours and X pad (B1y, 2026-10-23, READ, partial)

The panel fill colours are constants in the setter `0x489ADC`: (0xE0, 0x80, 0) when the flag byte `[+0x358]` is non-zero, (0xE0, 0x0A, 0)
otherwise, through `0x489A84` (0..255 channel values, no tint on the path); the border is 64.0 x3. The X pad is 0x14 minus the border
inset of 1 (derived). `panelTop` 1 and `panelHeight` 27 stay measured. Score unchanged at 0.005244. Evidence:
`docs/scratch/phase-b1y-panel-fields-colour.md`.

## HUD counters and timer drawn (C2g, 2026-10-23, render; reads partial)

Out, Saved and the timer are drawn from the C2c/C2d reads: digits replace the words, right end at 620 (`0x44D288`), Out y 15, Saved y 50, timer
`MM:SS` at 560,430. The amended A7 gate still passes on all ten views (numerator lower on three, equal on seven, since the HUD boxes hold no
gate-mask pixels there); the region residual drops in 27 of 27 cells. Right end versus centre, the Out definition, the Saved target word and the
default glyph colour stay UNREAD. Evidence: `docs/scratch/phase-c2g-counters-timer-render.md`.

## Pause caption panel formula (C2f, 2026-10-23, READ; corrects C2e)

The caption panel is not placed by `G+0x25C68`: `0x44E150` is a generic cell-list draw and `+0x14/+0x18` place the glyph text. The panel is
a quad at `G+0x25C90` written by `0x45D35A call 0x44DC50`: `[half(P40)-half(W)-8, P44-2H, half(P40)+half(W)+8, P44-H+3)` with `P40/P44 =
[0x58CB40/44]` and W/H the returns of `0x47ACF9` / `0x47AC50`. P40 = 640, P44 = 480, H = 29, W 70..71 reproduce the A7 target; those values
are measured-from-reference, not read. Evidence: `docs/scratch/phase-c2f-caption-placement-read.md`.

## Pause caption panel read live (C2h, 2026-10-23, MEASURED)

In the paused frame (`ui_state == 1`) the quad at `G+0x25C90` holds 277 / 422 / 363 / 454 as exact integers in all five snapshots, with
`[0x58CB40]` 640, `[0x58CB44]` 480 and `G+0x25C68+0x14/+0x18` 285 / 422 (so `half(W)` 35 and `H` 29, derived). The C2f formula reproduces the
A7 reference rect from read inputs. Whether the doubles are final pixels (no scale before raster) and the panel colour are unread. Evidence:
`docs/scratch/phase-c2h-caption-live-read.md`.

## Pause caption panel drawn, gate not met (C2i, 2026-10-23, render)

The panel (rectangle MEASURED, fill black and alpha 128/255 HYPOTHESIS from the setter immediates, blend ours) lowers the caption-region
residual on 10 of 10 A7 views, but with the caption exclusion removed the amended gate still fails on `initial`, `canonical` and `MM_DIG_000`
(initial 2879334 against 2742219). The "Paused" glyphs (key `STR_PAUSED`, consumer unread), the blend in `0x46D460` and the tube rows under the
panel are the remainder. Evidence: `docs/scratch/phase-c2i-caption-render.md`.

**C1g (Q1 round 1, offline, [note](scratch/phase-c1g-base-icon-draw-read.md)): the base icons are drawn by the skill-panel object `0x004C91D8`, not the `0x4b1774` widgets.** READ: frame arm `[G]==2` -> `0x45E0E6` -> `0x441845` -> `0x45B639`; each icon through `0x47A75D(icon, x, y)` at `0x45B784`; selected by animation name from `0x4C0694` (climb ... armageddon), frame `e8*nframes/ec` from `0x4C06E8`, not `widget_index+8`. Sheet: `0x4E3318/14/10` for armageddon/questionmark/bomb, else first non -1 of `0x4E335C[0..3]` (`sprintf("%s%s")` of `Lemmings`/`LemmingsWater`/`LemmingsAcid`/`EvilRat` + `Lo`/`Hi`). Counts: two digits, font `[0x4CBF7C]`, anchor (66, box top + 8). UNREAD: `0x47A75D` middle (x adjust, UV), loaded variant, count font file and colour. C1c's "`LemmingsLO` ruled out" is narrowed to unproven (the name is built by `sprintf`, so the missing literal proves nothing).

**C1h (Q1 round 2, offline, [note](scratch/phase-c1h-icon-placement-count-read.md)): the icon scale mode is READ, its inputs are not.** `0x47A75D` scales the frame offsets by `S = [0x58D53C] - [0x58CB38]` (halved at -1, native at 0, doubled at >= 1); `[0x58D53C] = [[0x4CBFB0]+0x10]` (`0x44186B`), `[0x58CB38]` is 0 for icons 2/8/9 and 1 otherwise. The sheet suffix is `Hi` unless a size-versus-budget loop (`0x443602..0x443733`) steps prefixes down to `Lo`. The count font object `[0x4CBF7C]` is not loaded by any `^Font` / `_FontSmall` call (those serve `[0x4CBF78]` and `[G+0x5574]`). UNREAD: `[[0x4CBFB0]+0x10]`, the sheet frame values, the budget, the count font file and colour. Q1 is at its cap of two rounds.

**C1i (Q2, the one stock start, [note](scratch/phase-c1i-icon-live-read.md)): entry failed again, nothing read, parked.** With the c3m retry and logging patch applied, `find_menu_index` still returned None (`no menu highlight`, 01:35:47, about 44 s after launch) and the game was foreground. The per-press diagnostics went to stdout and were lost, so the entry failure (c3j, c1i) is still not named. Install hashes restored, no game-memory write, process closed. Every C1 live item (`[levelScreen+0x10]`, icon fields, `0x4E335C..`, `[0x4CBF7C]`, hook) stays UNREAD; do not spend another start before the entry is understood offline.

**C1k (part 1b, offline, [note](scratch/phase-c1k-observable-entry.md)): the menu entry now records its own failure; the cause is still not named.** `find_menu_index` takes a `log` and writes one line per DOWN press (key polls consumed, foreground pid before and after, mod-5 and kept counts); `snapshot_idle` samples screen id, `ui_state`, key polls and foreground for 15 s with no input; the c3m retry and this change are landed in `tools/harness/goto_level.py`. The probe copy (`build/tmp/c1k-observable-entry/probe.py`, output `build/tmp/c1l-icon-live-read/`) reads exactly what the c1i probe read. The dry check is stubbed: it proves the lines reach `run.log`, not that DOWN reaches the game.

**C1j (part 1a, offline, [note](scratch/phase-c1j-named-writers-read.md)): the C1 live-read list is shorter, and C1h's count-font claim is corrected.** `[[0x4CBFB0]+0x10]` is a runtime mode field fed by messages and flag table `0x5D5850` (writers `0x4450EB`, `0x45D184`, `0x45700F`, `0x453369`), not a constant or a device query; its stock value is UNREAD. The frame record `F` is a `SpriteFrame` stream read by `0x485CE0`, so it is derivable from the `.SPR` once sheet and row are known. `[0x4CBF7C]` = `[G+0x5574]` and **is** loaded: `0x45D03E` -> `0x4403E5` pushes `_FontSmall` (C1h said no call loads it; withdrawn). `0x4E3310/14/18` = GreyIcons / BlackIcons / AdditiveIcons by registration stores `0x45E78D/A5/BD` (READ, no copy). Still live-only: the mode field, the per-icon `+0x1B8/1CC/1F8/1FC`, the `0x4E335C` handles, the font file and colour.

**C1l (part 2, the one stock start, [note](scratch/phase-c1l-icon-live-read.md)): the entry worked and the icon inputs were read; the hook did not run.** Idle snapshot: the menu was interactive from the first sample (screen 3, `ui_state` 3, key polls advancing, game foreground), the scan found its highlight after three presses, and the cause of the c3j / c1i failures is still not named. MEASURED at the paused start: `[levelScreen+0x10]` = 0 (so generic icons are halved, icons 2/8/9 native, by C1h's formula); `0x4E335C` = {6, -1, -1, -1}; `+0x1FC` is the registrar handle (6 for the seven generic icons, 1 / 2 / 3 for icons 2 / 8 / 9 = Grey / Black / Additive); all ten frame records F; `[0x4CBF7C]` and `[0x4CBF78]` are live font objects (`+8` = 37 and 36). UNREAD: the sheet behind handle 6, the font file and colour, the count text behind the objects' `+0xC` pointers, the hook's x / y. Install restored, one start used.

**C1m (directive 1a, offline, [note](scratch/phase-c1m-sheet-file-read.md)): handle 6 names `Lemmings` plus `Hi` or `Lo`, read from the loader; Hi versus Lo is a video-memory query.** `0x4432C1..0x4438B8` formats `%s%s` from the prefix table `0x4BFFC8` and suffix table `0x4BFFD8`, registers it through `0x446040` and stores the handle at `0x4E335C + 4*i`; the file is opened lazily by `0x484EB0` as `Sprites\%s.spr`, and `lemmings.box` holds `SPRITES\LEMMINGSHI.SPR` and `...LO.SPR`. Prefix presence is four flag bytes at `.LVL` offsets 36..39 (`MM_CLIMB_002` = 1,0,0,0). For that level `Hi` holds iff `[0x58CD80+0xCC]` (DirectDraw caps query, `0x1000`) is at least 2,678,724 bytes; the value is UNREAD, as is the registry slot-6 name. That handle 6 is the `0x443874` call is INFERRED from registration order. Facilitator re-derived the tables, header flags and threshold from the exe and level bytes; the step-down CFG and lazy-load chain were not re-derived.

**C1n (directive 1b, offline, [note](scratch/phase-c1n-count-font-read.md)): the count text is two digits built by `idiv 10`; the font is `_FontSmall` by name; its file and colour are only partly read.** `0x45A7D0..0x45A7FB` builds `count/10 + 0x30`, `count%10 + 0x30` (0 draws "00") and `0x4404FA` turns it into 0x1B0-byte glyph quads at `textobj+0xC`. Handle `+8` is the pool index in manager `0x58CB48`, keyed by the name `_FontSmall`, written through `0x440404` -> `0x474519`; it does not touch the sheet registrar. The only archive candidate is ordinal 656 `TEXTURES\_FONTSMALL.BMP` (181 bytes); glyphs '0'..'9' are 6 x 14 at x = 0, 6 ... 54 (`0x4BFC10`). UNREAD: that file's pixels, any tint, and icon 9's text pointer.

**C1o (directive part 2, tooling, [recipe](agents/MOUV_SSH.md)): `rea instrument-with-frida attach` works from an ssh worker against a session-1 process.** MEASURED on a throwaway `pythonw` target hooking `kernel32!Sleep`: hits reported, clean detach, target alive; the facilitator re-ran it. Not tried on the game, so no hook read of the game has been made.

**C1q (directive 1b round 2, offline, [note](scratch/phase-c1q-font-bitmap-read.md)): the count font bitmap is 64 x 16, 4 bpp, white digits on black.** The exe's decoder `0x467DE8`, run in an emulator on archive ordinal 656, yields a 630-byte BMP with palette index 1 = white and cells 6 wide at x = 0, 6 ... 54, matching table `0x4BFC10`. Vertex colour defaults to 255.0 (`0x440A10`), packed to `0xFFFFFFFF` by `0x46D460`, so the digits are white unless a later writer changes the quad colour: an inference, with those writers, the colour key (`0x47218A`) and the blend state UNREAD. The facilitator re-parsed the decoded BMP; the emulation of the decoder itself was not independently run.

**C1p (directive 1a round 2, offline, [note](scratch/phase-c1p-spr-sheet-read.md)): `LemmingsHi.spr` parses offline and equals c1l's live frame records for all seven generic icons.** `0x483EF0` reads 15 records (bash ... cartwheel) of sub-lists of 0x48-byte frames (source rect, four UV doubles, dx, dy, w, h, texture name) and a scale of 1.0 (Hi) or 0.5 (Lo). Indexed by c1l's `+0x1C8` / `+0x1CC` / `+0x1B8`, Hi matches dx, dy, w, h and all four UV doubles 7/7; Lo fails, so the c1l run used Hi (derived; the free-memory value stays unread). The facilitator's fresh parser reproduced 7/7 and 0/7. UNREAD: the sheet BMP encoding (the `$LEMSHEETHI*.BMP` members start `02 da 8d 60`, not `BM`), the archive lookup `0x47E40D`, the 384-byte tail's reader. The render still lacks the sheet pixels and the icon draw x, y.


**C1r (directive part 1, offline, [note](scratch/phase-c1r-sheet-pixels-decode.md)): the sheet members decode through the font's routine `0x467DE8`, and the seven generic icons' pixels are READ.** The member's leading `02` is a chunk count, not a method tag: per chunk u16 length, u16 output size, u8 type (3 raw, 4 = `0x467DE8`); caller `0x450270..0x450449`, call `0x4503FD`. All 115 `$LEMSHEET*` members decode to a BMP of the declared size (worker's unicorn sweep). The icon frame rectangle is x `[f8, fc)`, y `[f4, f0)`; its UV equals rect / sheet size, and the crops hold art (non-index-0 pixels 679/690 ... 689/690). Icons 2, 8, 9 use `GREYICONS` / `BLACKICONS` / `ADDITIVEICONS` (24, 24, 4 bpp), not the Hi sheets: partly read. The colour key (`0x47218A`), blend (`0x46D5F3`), tint writers and draw x, y stay UNREAD. The facilitator rebuilt the crop counts from the saved BMPs and c1l's live UV and re-read the call site; the decoder emulation was not independently run.

**C1s (directive part 2, the one stock start, [note](scratch/phase-c1s-frida-icon-hook.md)): the ten skill-panel icons are drawn at x = 39, 37, 38, 39, 40, 39, 41, 37, 38, 38 and y = 40, 92, 121, 158, 197, 238, 279, 317, 364, 410, and the count glyph quads are white.** MEASURED on the stock exe with one Frida hook on `0x47A75D` (`rea instrument-with-frida attach` from ssh onto the session-1 game: first attach to the stock game, clean detach, game alive, install hashes restored): 8,660 hits in 6 s, 4 complete frames of 10 icons, ECX = `0x4C91D8 + 0x1150 + i*0x208`, x and y equal C1g's READ arguments exactly. All 72 count-quad colour blocks read rgb 255.0 and alpha 255.0 (texture handle 37); the packed `0xFFFFFFFF` is derived, `[vertex+0x10]` was not read, and the blend state is UNREAD. `[0x58CE4C]` = 260,118,840 (above the Hi threshold); icon 9 has no text pointer. The facilitator re-derived every number from the saved raw output and checked the live side after the worker returned; the live reads themselves are single-start.

**C1t (directive part 3, offline, [note](scratch/phase-c1t-q3-panel-render.md)): the ten icons and the two-digit counts are drawn, and the C1 gate fails.** `--skill-panel` composites the C1r sheet pixels at the C1s positions through the C1h scale (`S = -1` for the seven generic icons, `0` for icons 2, 8, 9). The scorer's `skill_mae` is 0.10767973856209151 on `initial` (15157/140760) and 0.0554187566499468 on `MM_DIG_000` (3646/65790), identical to the pre-panel run. The scorer's mask inside the skill bands is only x 86..89 (184 and 86 pixels); the new pixels sit at x 26..76 and meet that mask in 0 pixels, so the masked error cannot move. A full-band compare of the `initial` render to `original-0.png` is 0.29204980255991286 (5147553/17625600), also above 0.02 (0.3146047226761075, 5545097/17625600, with the panel off). The oval button plates, the colour key and the device blend were not drawn. Not tuned.

**C1u (directive part 1, offline, [note](scratch/phase-c1u-footprint-mask.md)): the icon and glyph footprint is committed, and the saved C1t renders miss 0.02 on `initial`.** Ten icon rectangles use the C1s positions, the C1l frame sizes and the C1t edges (`dy` unscaled; scale 0.5 or 1). Eighteen glyph rectangles are the C1s quad-corner doubles, `[66, 72)` and `[72, 78)` by 14 rows; icon 9 has none. Plates are empty. The raster rule, fixed before the score, counts a pixel whose square intersects the rectangle: 5133 pixels. Against `original-0.png`, `render.png` is 0.22416530739836685 (880240/3926745) and `no-panel.png` is 0.3317052673397432. `MM_DIG_000` has no named stock frame. The facilitator recomputed the rectangles from the saved C1l and C1s files and re-read `0x47A7BB` and the `-1` divide arm; the scale-0 path to `0x47AB2A` was not re-walked.

**C1v (directive part 2 round 1, offline, [note](scratch/phase-c1v-plate-draw-read.md)): the ten button plates are quads drawn by `0x45B66A`, and a zero count skips the text submit.** `0x45B639` loops `i < 10`, takes `this+i*0x1B0+0x30` (`0x4C9208` when `this` is `0x4C91D8`) and calls `0x414920`. The box written at `0x45A795` is x `[18, 90]`, y `[12+40*i, 12+40*i+32]`. The handle is `[0x4E3348]`, stored from the name at `0x4C0E00`, whose bytes are `^tab`. The file and the pixels are unread, and nothing in these bytes says the texels are an oval. `0x45B6E4` compares `[this+i*4]` with 0 and skips `0x44E150`; that dword is the count `0x45A7D6` divides by 10. The plate draw is outside the branch. Colour key `0x47218A` and blend `0x46D5F3` stay unread. The facilitator re-read the loop, the box pushes and the five name bytes; the registration chain and `0x45FE10` were not re-derived.

**C1w (directive plate join, offline, [note](scratch/phase-c1w-plate-footprint.md)): the ten plate boxes are in the footprint, and the saved renders still miss 0.02.** The boxes are x `[18, 90]`, y `[12+40*i, 12+40*i+32]`. Icon and glyph edges are the C1u rectangles, unchanged. The union is 23166 pixels. Against `original-0.png`, `render.png` is 0.29181384257636983 (5171522/17721990) and `no-panel.png` is 0.315641979258537. `MM_DIG_000` stays reference-unnamed. The facilitator recomputed the union and both numerators, and a one-edge change of plate-0's right from 90 to 80 moved the render MAE to 0.29001710229161554 before restore. `^tab` pixels stay unread.

**C1x (directive part 2 round 2 of 2, offline, [note](scratch/phase-c1x-colour-blend-read.md)): `0x47218A` is the texture-loader entry, and `0x46D5F3` is an indirect call.** `0x47218A` is `push ebp`. The plate draw does not call it; `0x4744B1` does. `0x46D5F3` is `call dword ptr [ecx+0x70]`, `ecx = [[0x58CDB8]]`, after `push 0x1C4` and `push 6`. No blend mode is in that instruction. `0x4149FD` calls `0x46D3C1`, and `0x46D457` calls `0x46D460` only when four `test eax,eax` checks are nonzero. Those `eax` values, the method, and the `^tab` pixels are unread. The facilitator re-read those instructions. Part 2 stops here; the render was not started.

**C1z (directive part 1b, offline, [note](scratch/phase-c1z-plate-blend-hook.md)): the plate quads set blend in `0x414D80` and texture stage in `0x47261F`, and the hook is `0x41494F`.** `0x45B66A` calls `0x414920` with no branch before `0x41494F` (`call 0x414D80`). Slot `+0x58` there is blend enable `0x1B`, source `0x13` and destination `0x14`, skipped when the selector equals `[0x4C227C]`. `0x414944` calls `0x47261F`, which binds a texture through `+0x98` and writes stage-0 colour and alpha through `+0xA0`, plus alpha test through `+0x58`, unless the descriptor or the flags match their caches. Header names for those slots come from MinGW `IDirect3DDevice3`; no GUID was read. The live selector and flags are unread. The hook samples the flag cache after `0x47261F` returns, so that sample does not say whether the stage writes ran. The facilitator re-disassembled all four ranges and re-ran the dry check.

**C1y (directive part 1a, offline, [note](scratch/phase-c1y-tab-pixels-decode.md)): `^tab` decodes to a 72 by 32 BMP whose colour bytes are zero.** `0x45E8F7` pushes `0x4C0E00` (`^tab`) and `0x45E909` stores the slot. A leading `5E` sets flag 8 (`0x474471`), and `0x47236B` pushes `Textures\%s.alf`. `TEXTURES\^TAB.BMP` is 6966 decoded bytes, SHA-256 `d38db65196988c5b3d018a5a5f6386ba6333c5590e64ad4bfe67a55bac1403ed`, and all 6912 colour bytes are `0x00`. `TEXTURES\^TAB.ALF` is 2304 bytes with 2129 nonzero. The consumer `0x47E40D` and `^tab_selected` (`0x45E90F`) are unread. The facilitator re-ran the decode and re-read the registration, the flag-8 test, and the `.alf` format.

**C1ab (gate run round 1, offline, [note](scratch/phase-c1ab-prestage-hook.md)): the plate hook samples the descriptor and flag cache at `0x414944`, before `0x47261F` stores them.** `0x414944` is `call 0x47261F`. `onEnter` reads `[ebp-4]`, `[descriptor+0x24]`, `[0x5CD5F4]` and `[0x4C24E4]` on a plate hit. `0x41494F` still reads the blend selector before `call 0x414D80`. `0x47262E` skips the body when the descriptor equals the cache, and `0x472665` skips the flag store when the flags equal the cache, after the bind at `0x472653`. The live values are unread. The facilitator re-disassembled both ranges and re-ran the dry check.

**C1aa (gate run round 1, offline, [note](scratch/phase-c1aa-alf-alpha.md)): a `^tab` `.alf` byte is `sar` then `shl` into the 16-bit pixel.** `0x473897` is `sar eax, cl` with `cl = 8 - [ebp-8]`. `0x47389F` is `shl eax, cl` with `cl` the trailing-zero count of `[ebp-0x128]`. `0x473943` stores the word. `0x4736B6` skips the `sar` when that dword is 0. `0x473B1E` stores a popcount at `[context+0xc]`, and the upload passes `lea [ebp-0x14]` to `[vtable+0x20]`. `^tab_selected` is the same `0x45FE10` path; its colour bytes are nonzero and its `.alf` max is 248. The writer of `[ebp-0x128]` is unread. No constant shift is in these bytes. The facilitator re-disassembled the shifts and re-ran the rule command.

**C1ad (gate run, one stock start, [note](scratch/phase-c1ad-plate-blend-live.md)): at paused `MM_CLIMB_002` every logged plate uses blend selector 1.** MEASURED: 8650 plate hits, caller `0x45B66F`; the 40 stored records are four frames of plates 0–9 at `0x4C9208 + i*0x1B0`. Descriptor `0x2627270`, flags 8, blend cache `0x10620C1F`. The flag cache equals the flags on all 40. The descriptor cache differs only on plate 0. `[0x4C24E0]` is `0xA0` and was not shown to be stored, because the flag-cache compare skips those stage writes. Selector 1 is the landed nonzero arm (enable 1, source 5, destination 6) and the cache compare does not skip it. The facilitator re-parsed the hook log and re-hashed the install.
