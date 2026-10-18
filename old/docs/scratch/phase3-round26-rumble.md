# Phase 3 round 26 — Rumble

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`.

Harness: `tools/harness/measure_rumble.py` (new). Re-run:

```
python tools/harness/measure_rumble.py --audit
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

**Result: no Rumble world gadget.** Cue `Rumble` is real SFX (not
Balloon/ForceF-shaped). The only absolute load of `DAT_004E32F0` is
inside object step `FUN_00423749` (vtable `0x004B0F00` +0x14): on a
deferred state commit when previous committed state `+0xa4` was
nonzero, channel `FUN_00406a00` → SFX `FUN_0044535b`. The cue does not
attach a lemming state and does not touch the actor list. No
`rumble.a3d`. `SOUNDS\RUMBLE.WAV` exists (played via the cue).

The play site's object is a zone / lemming-count state machine (helper
`FUN_00423858` walks list `0x004C88D0`) that settles pose double
`+0x90` to 0.0 / 1.0 and solid byte `+0x58` — door/gate-shaped —
but that object is **not** a cue-named Rumble gadget. Rumble is only
its transition sound. Not camera shake, not bomber aftermath, not UI.

---

## PREDICTIONS (written before the static audit)

### P1 — slot matches brief

`push 0x4c0cfc` at `0x0045E690`; store to `+0x1c8e8` (global
`0x004C6A08 + 0x1c8e8 = DAT_004E32F0`).

Falsified if the store after the Rumble push writes a different
displacement.

### P2 — sole play load at `0x00423823`

Abs ref at `0x00423823` is the only `.text` imm32 hit to
`DAT_004E32F0`. It sits inside `FUN_00423749`.

Falsified if additional absolute refs exist, or if the load is not a
play path into `FUN_0044535b`.

### P3 — brief's extra `+0x1c8e8` sites are call false positives

Imm32 search also hits `0x46dfc3` / `0x489453` because `0x1c8e8`
little-endian begins `e8 c8 01 00`, which matches `call rel32` with
`rel=0x1c8`. Real displacement sites are only register / preload /
unload (`0x45e6a4` / `0x45ead3` / `0x45f122`).

Falsified if those sites are `mov` with `[reg+0x1c8e8]`.

### P4 — no rumble.a3d / no exclusive Rumble gadget

No `rumble*.a3d` in `lemmings.box` / `patch.box` / COMPILED `.LVL`
payloads. Actor list `0x004CBB18` has no imm in `FUN_00423749`. Cue
does not mutate a lemming.

Falsified if an a3d placement or exclusive cell-bit / lemming-attach
path is Rumble-named.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| cue `Rumble` | `0x004C0CFC` (`MineMetal\0\0\0Rumble\0\0AntiGrav…`); sole string imm `push` at `0x0045E690` (imm32 at `0x0045E691`) |
| register | `FUN_0045e51f`: `FUN_0041d7d0(..., s_Rumble)` → `*(+0x1c8e8)` store disp `0x0045E6A4` (`89 82 e8 c8 01 00`) |
| cue object base | `0x004C6A08`; Rumble global `DAT_004E32F0` |
| preload / unload | disp `0x0045EAD3` (`8b 91 …`) / `0x0045F122` (`8b 88 …`) |
| false `+0x1c8e8` | `0x0046DFC3` / `0x00489453`: `e8 c8 01 00 00` = `call +0x1c8` |
| abs play load | **only** `0x00423823` `mov eax,[DAT_004E32F0]` (imm at `0x00423824`) |
| containing step | `FUN_00423749` (body `0x00423749`..`0x00423857`); vtable `0x004B0F00` +0x14 |
| vtable install | ctor `FUN_00423480` `mov [reg],0x4b0f00` at `0x00423505`; copy `FUN_00423583` / `FUN_00423631` |
| play arm | after timer `+0xf0 <= 0` and `+0xa0 != +0xa4`: settle via `FUN_00426fa8` (`+0x90=0.0` when `+0xa0==1`) or `FUN_00426fca` (`+0x90=1.0`); **if `+0xa4 != 0`** then `FUN_00406a00` → `FUN_0044535b(Rumble)`; then `+0xa4 = +0xa0` |
| count helper | `FUN_00423858` walks `0x004C88D0`; may set desired `+0xa0=2` when threshold met |
| actor list | **no** `0x004CBB18` imm inside `FUN_00423749` |
| camera / bomb | play arm is SFX only; no bomber / Ohno / Explode path |

P1–P4 hold.

---

## Offline audit

`--audit` on stock exe + `lemmings.box` / `patch.box`:

| check | result |
| --- | --- |
| `rumble*.a3d` in boxes / COMPILED `.LVL` | **none** |
| `SOUNDS\RUMBLE.WAV` | present (played via cue) |
| exclusive Rumble cell bit / a3d placement | N/A — no Rumble gadget object |

---

## Live series

Not required for the Rumble-cue question. Sole play path and
register/preload/unload inventory are static. No process launched;
no live writes.

---

## Settled

**What to a lemming:** the cue itself is **SFX only**. There is **no**
Rumble world gadget / no lemming attach. A separate object
(`0x004B0F00`) may open/close (pose / `+0x58`) using lemming counts
from list `0x004C88D0`; that object's full mechanic is out of scope
for this cue round — Rumble is only played on its nonzero→new state
commit.

**When played:** object step `FUN_00423749` when deferred state timer
`+0xf0` elapses, desired `+0xa0` differs from committed `+0xa4`, and
previous committed state was nonzero.

Sim: no Rumble gadget (`kRumbleGadgetInRetail = false` in `World.h`).
Harness: `measure_rumble.py --audit`.
Test: `testRumbleCueNoGadget` (mutate-alone: flip the flag to `true`).

Process: none. Live writes: none.
