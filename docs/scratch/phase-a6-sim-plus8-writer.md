# Phase A6d10: writers of actor `+0x08` / `+0x10` / `+0x18` (READ)

Brief `build/tmp/a6d10-sim-plus8-writer-brief.md`. Packet
`build/tmp/a6d10-sim-plus8-writer/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `f5cd2415-f029-4fe0-b1f8-0d17be5f65c1`. Offline. No mapper.

## Result

- **READ:** helper stores
  - X `+0x08`: `0x42EEC9` `dd 59 08` in `0x42EEA3` —
    `0x40DEC0(arg − [+0x360])`
  - Y `+0x10`: `0x42EE9A` `dd 59 10` in `0x42EE74` —
    `0x40DF50(0x4C6A10, arg − [+0x368])`
  - Z `+0x18`: `0x42EDF9` / `0x42EE47` in `0x42EDCD` — pull toward **0.5**
    (`0x4B03A8`) by `±arg * 0.1` (`0x4B10C0`); snap to 0.5
- **READ:** walk inlines the X wrap without calling `0x42EEA3`:
  `0x41BE95` (`step * 5.0 * [+0x40] + [+8] → 0x40DEC0 → [+8]`),
  `0x41CDAE` (same with extra `* 2.0`). Extra inline `0x413A8F`:
  `0x40DEC0([+8] − [+0x40]) → [+8]`.
- **READ:** `0x40DEC0` wraps into `[0,256)` (`+256` if negative; subtract-256
  loop while `>= 256`). `0x40DF50` clamps Y to `[0, height-1]`.
- **READ:** `0x42F22E` builds the overlap sample
  `([+0x360]+[+8], [+0x368]+[+10], [+18])` for `0x430BA1`.
- **No mapper.** `sim::Lemming` → native sample is still missing spawn
  init, walk remainder, probed Y, and Z 0.75 vs 0.5.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read every listed store and both helpers. Callers via REA `xrefs`
  match: `0x42EEA3` ← `0x40F177` only; `0x42EE74` ← `0x40F187`,
  `0x41B8B4`, `0x41C6BA`, `0x41C833`, `0x42C810`, `0x42C854`; `0x42EDCD`
  ← `0x40E053`, `0x41BB0A` (`FUN_0041ba81`).
- `.rdata 0x4B03A8` = `00 00 00 00 00 00 e0 3f` (0.5);
  `0x4B10C0` = `9a 99 99 99 99 99 b9 3f` (0.1). Inside `.rdata` rawsize.
- **Narrowed:** `0x42EEF2` copies **24 bytes from actor+8** and calls
  `0x43D56C`. It does **not** add `+0x360`/`+0x368`. A6a still stands:
  pose uses `+0x08`/`+0x10`/`+0x18` directly. The worker’s “composed
  triple” wording applies to overlap (`0x42F22E` → `0x430BA1`), not to
  `+0x108`.
- **Narrowed:** `+0x360` is not a cell origin. `0x42F66B` writes **−1.0**
  facing left and **0.0** facing right (Phase 3 turn helper). Bounce
  `0x40F177` subtracts that bias inside `0x42EEA3`; the walk inline at
  `0x41BE95` does not.
- No production files. No game. No CTest (no code change).

## Parked / next

Sim → `+0x08` mapping. Resume with **spawn/init** of `+0x08`/`+0x10`/`+0x18`
(hatch int16 → first sample), not another `+0x360` pass. Evidence: this note.
