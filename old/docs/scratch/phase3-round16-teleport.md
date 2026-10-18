# Phase 3 round 16 — teleport

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Harness: `tools/harness/measure_teleport.py` (new). Re-run:

```
python tools/harness/measure_teleport.py --audit
# restart the process, then
python tools/harness/measure_teleport.py --enter MM_TELE_003 --dump-objects --watch 40 --hz 80
```

---

## PREDICTIONS (written before live runs)

### P1 — asset census

`teleport.a3d` and `teleport_fx.a3d` each appear in the same **18** retail
levels; `teleport_fx` fails the world-pose tube check (identity / FX only).
No cell bit is exclusive to those 18 levels (unlike anti-grav `0x8000`).

Falsified by any mismatch in `--audit`.

### P2 — behaviour on MM_TELE_003

A walker that overlaps a `teleport.a3d` pad relocates to near the paired
pad's column / world Y. Detection is object AABB (`0x00430BA1`) or an
object-step walk of a dedicated list — not a cell bit alone.

Falsified by: 40 s watch with hatch traffic and **zero** actor-list jumps
with `|dX|>2.5` or `|dY|>5`.

### P3 — which object owns behaviour

Among live object-shaped vtables on MM_TELE_003, the teleport pads are the
ones whose poses match the four `teleport.a3d` translations (cols ~15, 38,
155, 177). `teleport_fx` is visual only if no such object matches FX.

---

## Offline audit

`--audit` on `lemmings.box` + `patch.box` (102 COMPILED levels):

| check | result |
| --- | --- |
| `teleport.a3d` levels | **18** |
| `teleport_fx.a3d` levels | **18**, same set |
| bits exclusive to all tele levels | **[]** (empty) |
| MM_TELE_003 poses | 4× `teleport.a3d`; fx poses fail tube (FX-only) |

MM_TELE_003 file poses (atan2(−tx,tz), world Y-up):

| # | ty | col≈ |
| --- | --- | --- |
| T0 | 30.000 | 155.012 |
| T1 | 64.000 | 177.000 |
| T2 | 38.000 | 37.993 |
| T3 | 61.000 | 15.000 |

18 levels: `H06_MT_1` `H08_MT_1` `H10_MT_1` `M12_MT_1` `M16_MG_1`
`M17_MT_1` `M18_MT_1` `M19_MT_1` `M21_KF_1` `MM_TELE_003` `SHOW_KF3`
`SHOW_MT3` `VH08_MT` `VH10_MT` `VH12_KF1` `VH13_MT` `VH18_JC` `VH21_JC`.

P1 holds. Cell-bit-alone detection is **falsified** (clean negative).

---

## Live series (MM_TELE_003)

Entered via `measure_float_splat.enter` / goto_level record swap.
`VERIFY loaded_level='Compiled\\MM_TELE_003.lvl'`.
`slot [0][0] restored: True` immediately after enter (no grid paint this round).

### Live pad objects (vtable `0x004B1250`, heap scan)

| ptr | pose (X,Y,Z) | AABB X | AABB Y | +0x98 id | +0x260 partner |
| --- | --- | --- | --- | --- | --- |
| `0x279eb80` | (79, 61, 0.5) | [73..85] | [61..69] | 4 | → (241,64) |
| `0x27947d0` | (241, 64, 0.5) | [235..247] | [64..72] | 6 | → (79,61) |
| `0x278fac0` | (219, 30, 0.5) | [213..225] | [30..38] | 5 | → (102,38) |
| `0x2799b20` | (102, 38, 0.5) | [96..108] | [38..46] | 3 | → (219,30) |

AABB shape: **col ±6 × [ty .. ty+8]** closed (12×8 world units). Matches
file cols ~15/38/155/177 once live X is used (cylinder unroll).

### Actor-list jumps (`0x004CBB18`, 45 s @ 80 Hz)

Many jumps; sample:

- `(79.0,63) → (241.0,64)` face −1→−1, state tele-snap→tele-out cycle
- `(219.0,32) → (102.0,38)` face −1→−1

Facing preserved on every observed jump. Path is walk → type `0x28`
(vtable `0x004B0C1C` tele-out) → type `0x2c` (`0x004B0CE8` tele-snap) →
XY snap to partner. P2 and P3 hold; `teleport_fx.a3d` has no matching
behaviour object.

---

## Static reads

| item | address / evidence |
| --- | --- |
| Pad object vtable install | `0x00434474` `mov [ecx], 0x4b1250`; clears `+0x260` at `0x0043447d` |
| Vtable `0x004B1250` slots | dtor `0x4358c0`, step family includes `0x434aaf` / `0x434b0f` |
| Partner resolve | stream id at object `+0x98`; linker `0x00444BA1` writes partner into `+0x260` |
| Overlap sites used by tele path | `0x00435467`, `0x004354CB` call general AABB `0x00430BA1` |
| Attach type `0x28` | `0x0041A310` (countdown push **4** into state+0x18 at `0x0041A317`) |
| Attach type `0x2c` | `0x0041B590` |
| Relocate | `0x0043503C`: X ≈ partner[+0x1b0] (+ facing×6×t term); Y = partner[+0x1b8] + **2.0** (literal at `0x004B1238`) |
| No `"teleport"` ASCII in exe image | search 0 hits (brief lead confirmed) |

AABB call-site inventory (19 sites of `0x00430BA1`) — known users:

- anti-grav `0x0041C1E3` (round 9)
- exit among the set (round 11)
- teleport `0x00435467` / `0x004354CB` (this round)
- remainder still unread for other gadgets

---

## Sim mapping

| original | webolution |
| --- | --- |
| object AABB 12×8 | `Teleport` / `teleportTriggerFromPose` |
| type 0x28 then 0x2c delay | `State::Teleporting` + `teleportDelayTicks=4` |
| relocate to partner | `destX`/`destY` = partner feet cell |
| Y+2 snap then settle | integer sim snaps to partner world Y (not +2) — measured brief in-air |
| pair via +0x98 / +0x260 | stand-in: file order `i ↔ i+n/2` (even count); id word not yet in .LVL parse |
| `teleport_fx.a3d` | skipped by `findLevelObjects` (identity pose) |

Mutate-alone: `#if 0` the `teleportAt()` arm in `World::step` →
`testTeleportPad` FAIL `walker relocated to partner (200, 40) within 20 ticks`
(290 checks, 1 failure). Restored + touch → suite 16/16.

---

## Process / restores

- Live writes: level-table slot `[0][0]` only (goto_level); restored `True`
  before watch. No collision-grid paint.
- Process `52496` (`Lemmings Revolution.exe`) killed after measurement.
  No Lemmings process at closeout.

---

## Verdict

A teleport pad is an **object-only** gadget (`teleport.a3d`, vtable
`0x004B1250`). Overlap via `0x00430BA1` attaches tele-out then tele-snap;
the object step relocates the lemming to the partner pad's pose (facing
preserved). `teleport_fx.a3d` is visual only. No exclusive cell bit.
)
