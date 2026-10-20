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

## Hard-won rules of thumb

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
8. **Preserve quirks.** German menus, RETURN only when the menu highlight is exactly 0, a
   short ESC that fails to leave pause, the drawn crosshair disagreeing with the tested
   cursor. All real. Do not smooth them out.

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

The tick is **5 Hz (200 ms)**, measured three ways and not frame-locked. It had been guessed
at 20 Hz, which made everything timed in ticks run four times too fast.

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

**Known divergence, deliberately unfixed**: a left-facing turn costs our sim a tick and the
original nothing (`0x0041C67E`). Fixing it perturbs the blocker-turn expectation, so the two
must be taken together.

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
  **decrementing**, not a sprite change. Panel indices known: 0 climber, 1 parasol, 3
  blocker, 4 builder, 7 digger; 2, 5, 6, 8, 9 unidentified.
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
