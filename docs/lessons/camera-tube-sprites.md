# Camera, tube, and sprite findings

> Dated historical research excerpts, preserved in their original wording and order within each source range.
> Earlier conclusions may be superseded by later entries; this is not current project status or
> authorization. See [STATUS](../STATUS.md) for current scope and [LESSONS](../LESSONS.md)
> for essential startup guidance. Some excerpts refer to findings that appeared earlier in the
> former combined file; use their linked evidence and the source-range comments below.

<!-- Source lines 126-330 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## Camera checkpoint: A1 complete for the tested scope

The accepted pose/lens operand reads and their exact addresses are in
[the A1 facilitator check](../scratch/phase-a1-camera.md). Its standalone fixture has
independent original-image mutation/restore proofs; it is not a finished renderer.
That first checkpoint retained a failed left view. The later
[true live-arena replay](../scratch/phase-a1-live-arena.md) cleared all four retained
views, and the [zoom/elevation check](../scratch/phase-a1-zoom-elevation.md) completed
A1 for the tested MM_CLIMB_002 camera branches. Do not revive the old failure as
the current verdict, or expand the bounded passes into full-scene acceptance.

The scale helper executes **FPTAN**, not sine (`0x0047B095` calls `0x0049D8D4`,
`D9 F2` at `0x0049D8F2`). Projection divides by transformed depth and reverses screen y
(`0x0046DDF0` for the tube). `0x0047D0C0` is the sprite/billboard path, as corrected
in the [outer-route check](../scratch/phase-a1-outer-route.md). The captured center was `(319,239)` at stock resolution,
from BSS globals `0x0058D294` / `0x0058D298`; each component has a separate pixel mutation.
The conventional center assumption and earlier sine interpretation were explicitly
corrected. Exact original floating-point intermediate precision is still unestablished.

Control action names do not imply unconditional rotation. The captured default path
moved the virtual cursor without a grab; a grab over empty scenery changed the meaning of
the same inputs. Window activation and selection matter. Preserve the failed view and the
scope limits in the linked note before proposing the next round.

The [active cylinder draw follow-up](../scratch/phase-a1-cylinder-draw.md) corroborates a
bounded frame-time inner-bank submission at the retained left camera and matches its
source vertex pool to the stored level. It does not fix the outer-only fixture's failed
pixels by itself; the later live-arena note carries the passing result. Use the
foreground-acquisition helper from a hidden worker; it masks
activation input from the game and verifies actual focus. Do not require a manual click
for every probe or use the older input-queue attachment helper.

The [cell-fixture replay](../scratch/phase-a1-cell-oracle.md) retains the subsequent
failed geometry candidate, isolated mutation survivors and explicit corrections.
Its source/runtime mapping is READ evidence, with unprotected selection mutations.
The subsequent [emission check](../scratch/phase-a1-cell-emission.md) established a
selected quad's ordered coordinates and UVs; the
[projection/cache replay](../scratch/phase-a1-projection-cache.md) supplied a bounded
source derivation. The [depth check](../scratch/phase-a1-depth-clipping.md) found no
numeric executable near/far pair; the wrapper's internal Z bits remain unreadable.
The [control mapping](../scratch/phase-a1-control-mapping.md) and final zoom check
supersede the earlier open-camera handoff. The gameplay writer of `control+0x10`
is still unlocated (its behavior is measured). The entry point remains A4.

## Tube checkpoint: A2 fixed-ROI checks complete; wider diagnostics still fail

The [A2 facilitator check](../scratch/phase-a2-tube.md) and
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

The [A3 facilitator check](../scratch/phase-a3-sprite-view.md) corrects a settled-doc
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

The [A4a check](../scratch/phase-a4-renderer-core.md) added the first production
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

The [A4b check](../scratch/phase-a4-cell-mesh-source.md) located the producer chain.
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

The [A4c check](../scratch/phase-a4-cell-mesh-rule.md) decoded the rule and reproduced
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

The [A4d check](../scratch/phase-a4-cellmesh-route.md) routes the A4c cell mesh through
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

The [A4e check](../scratch/phase-a4-window-controls.md) adds the 640x480 SDL window,
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


