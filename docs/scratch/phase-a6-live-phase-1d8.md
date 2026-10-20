# Phase A6d5: live `+0x1D8` is 0.0 — pixel-fit −0.125 is not native

Brief `build/tmp/a6d5-live-phase-1d8-brief.md`. Packet
`build/tmp/a6d5-live-phase-1d8/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `d61a8177-d48e-4d02-b2ce-00e4abf55156`. One memory-only start (pid 62968).
No `dgVoodoo.conf` write.

## Result

- **READ:** three walkers (`vtable 0x4B10E8`) at the 10 s `MM_CLIMB_002` pause
  hold **`0000000000000000` → 0.0** at controller `actor+0x128+0x1D8`.
  None is −0.125.
- **MEASURED:** production `composeActorPhase` default is **0.0**. Default
  webolution shot hashes `c4e6cf47…` (A6d3 temporal-0 inject). Actor 2 MAE
  **0.11246451442529874** (worse than tube-only **0.10089786756453423**).
- **Parked:** offline −0.125 still lowers actor 2 MAE to **0.08861160625866508**.
  That is a pixel fit, not this field.

JSON key `position_plus8_xyz` in `live-report.json` is **+0x108** (model), not
`+0x08`. The `+0x1D8` addresses and zero bytes do not depend on that label.

## Facilitator check

2026-10-20, Cursor facilitator.

- Address arithmetic: `actor+0x128` and `+0x1D8` match the three reported
  controller / field addresses. 8 zero bytes decode as 0.0.
- Isolated mutation `composeActorPhase = -0.125` hashes `e3ad3755…` (A6d4
  default shot). Restored 0.0.
- `test_actorcompose` + `test_spriteview` pass. 23/23 CTest after restore.
- No game; no `LemWeb-*`. Install hashes in the packet equal before/after
  (exe `32675bdb…`, `dgVoodoo.conf` `9a34b1b3…`).

## Parked

Native view index / strip at this pause versus `selectSpriteView` at phase 0.
The −0.125 fit flips a view; live `+0x1D8` does not. Resume with `+0x1B8` /
`+0x1B4` (and cheap `+0x1C8`/`+0x1CC`) on the same protocol. Evidence: this note.
