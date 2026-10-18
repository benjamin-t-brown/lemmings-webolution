# Phase 3 round 31 — cell-flag writers (`0x100` / `0x400`)

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`.

Harness: `tools/harness/measure_cell_writers.py` (new). Re-run:

```
python tools/harness/measure_cell_writers.py --audit
python tools/harness/read_steppers.py --at 0x00427554 --len 0xC0
python tools/harness/read_steppers.py --at 0x004270DC --len 0x100
python tools/harness/read_steppers.py --at 0x00423749 --len 0x120
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.
Propose §1.6 bullet in the agent report; do not edit the ledger here.

---

## SUMMARY

| Thing | Status | Value |
| --- | --- | --- |
| **`0x100` writer** | **MEASURED authored-only** | no runtime or/and-imm / stamp-fed setter in `.text` |
| **`0x100` reader** | **unchanged (round 15)** | `0x00415552` → may set `lem[+0x5f]=1`; not death |
| **`0x400` writer** | **MEASURED door/gate** | vtable `0x004B0F00` paints/erases `| 0x403` / `& 0xfbfc` |
| **`0x400` reader** | **MEASURED mesh scan** | `0x004615E9` via `0x00462264`; stops/continues on bit |
| **Round 15 story** | **unchanged** | acid/drown still `0x40`; `0x100` still flag5f only |
| **sim** | no change | door/gate already deferred (round 26); no missing lemming mechanic |

---

## PREDICTIONS (written before the audit)

### P1 — `0x100` has no runtime cell writer

No `or reg, 0x100`, no and-clear of bit 8 near `call 0x40de60`, no
`push 0x100` feeding `CELL_STAMP` `0x004311D0`. The three authored grids
are the whole retail source.

**Holds.** Audit: 0 or-imm, 0 clear-near-probe, 0 stamp-fed pushes;
levels `H08_MT_1`, `H22_MT_1`, `VH06_MT` only.

### P2 — `0x400` is the door/gate solid stamp (vtable `0x004B0F00`)

Round 26 already named the Rumble host door/gate-shaped. The `or 0x403`
paint belongs to that object's anim settle / init / world pass, not to
bricks (round 6).

**Holds.** Paint callers `0x004271D5` / `0x00427509` / `0x0043B1F8`;
erase siblings; object step `0x00423749` → anim `0x004270DC`; vt
`0x004B0F00` +0x14 / +0x1c.

### P3 — mesh scan is the `0x400` reader; not a lemming hazard

`0x00461704` / `0x00461843` inside `0x004615E9` are mesh-rebuild
geometry, reached from world pass after paint via `0x00462264`.

**Holds.** Wrap call at `0x0046226E`.

### P4 — Round 15 reader story does not change

Settling writers does not reopen drown / acid / zone splat.

**Holds.** Reader bytes at `0x00415552` unchanged; no correction forced.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| `0x100` reader | `0x00415552` `81 e2 00 01 00 00` |
| no `or edx, 0x100` | `.text` pattern `81 ca 00 01 00 00` → 0 hits |
| no `or eax, 0x100` | `0d 00 01 00 00` → 0 hits |
| paint `0x403` | `0x004275C1` `81 ca 03 04 00 00` |
| erase `0x403` | `0x00427641` `81 e2 fc fb 00 00` |
| paint FN / erase FN | `0x00427554` / `0x004275D4` (rect from obj +0x40/+0x44/+0x4c/+0x48) |
| anim settle | `FUN_004270dc`: +0x98→+0x90; paint then `+0x58=0`; erase then `+0x58=1` |
| object step | `FUN_00423749` (vt `0x004B0F00` +0x14) calls anim at `0x0042375D` |
| object init | `FUN_0042421d` (vt +0x1c) → `FUN_0042727c` paint/erase from `+0x58` |
| world pass | `FUN_0043b150` lists `0x004C8968` / `0x004C8988`; `+0x58` → paint or erase |
| mesh wrap | `FUN_00462264` → `FUN_004615e9` at `0x0046226E` |
| scan tests | `0x00461704` `and eax, 0x400`; `0x00461843` `and eax, 0x401` |
| vtable install | `0x00423505` / `0x004235E1` / `0x00423652` `mov …, 0x4b0f00` |
| level load | `FUN_00448af0` from `FUN_004432c1` into screen `+0x1f60` |

---

## Offline audit

```
levels 102
any cell & 0x100: 3 -> H08_MT_1 H22_MT_1 VH06_MT
any cell & 0x400: 52
exact word 0x403: 14827 cells across 51 levels
```

Authored `0x403` is consistent with doors that load closed (init paints
from `+0xa0` / `+0x58`) and with offline level data that already stamped
the same steel-solid shape. Runtime still toggles via paint/erase.

---

## Live series

Not required for the writer question. Paint/erase and caller chain are
static; round 15 already measured live paint of `0x100` → flag5f. No
process launched; no live writes.

---

## Verdict

**`0x100` is level-authored only.** No runtime cell writer was found.
The round-15 reader story stands: ground cell with `0x100` (and `0x40`
clear) may set `lem[+0x5f]=1`; it is not acid death and not drown.

**`0x400` is the door/gate solid-stamp marker.** Object vtable
`0x004B0F00` paints `| 0x403` (solid \| indestructible \| `0x400`) over
its cell rectangle when the pose settles closed, and clears with
`& 0xfbfc` when it opens. The mesh rebuild scan uses the bit to bound
painted runs. Same object plays Rumble on state commit (round 26). Not
a lemming attach / death bit.

**Does not change Round 15 / CollisionGrid reader story for `0x100`.**
Names `kCellFlag0x400` as door/gate stamp; keeps `kCellFlag0x100`
placeholder (flag5f side-effect, authored).

Sim: no new mechanic required this round — door/gate world object was
already deferred from the Rumble cue round; `0x100` needs no death path.

---

## Files

- `tools/harness/measure_cell_writers.py` (new)
- `src/assets/CollisionGrid.h` (comments)
- `docs/scratch/phase3-round31-cell-writers.md` (this file)
