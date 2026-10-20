# Phase A6d20: incoming `0x47A75D` Y is `cy − C0/C8` (READ)

Brief `build/tmp/a6d20-screen-y-bias-brief.md`. Packet
`build/tmp/a6d20-screen-y-bias/`. Cursor `round-agent` on
`composer-2.5-fast`, dispatch `114a797e-1289-4d5a-91bd-4b114227a636`.
Offline. No `ActorSprite` change.

## Result

- **READ:** `0x43CCF8` loads `1.0` (`0x4B02D0`) and `0x43CCFE` divides by
  **`[this+0xC8]`** with no `0.0003` subtract. `0x43CD07`–`0x43CD1B`
  stores screen X at `[ebp-8]`:
  `fild [0x58D294] + inv · [this+0xB8]` (`faddp` `DE C1`).
- **READ:** `0x43CD1E`–`0x43CD32` stores screen Y at `[ebp-0x18]`:
  `fild [0x58D298] − inv · [this+0xC0]`. The opcode at `0x43CD30` is
  **`fsubp` `DE E9`**, not `fsubrp`.
- **READ:** pushes at `0x43CD38`–`0x43CD44` pass that X qword then that
  Y qword. `0x47A75D` `ret 0x10`, so **`[ebp+0x10]` is the Y**.
  `0x47A7BB` then does `Y − (double)[rec+0x34]` (`fsubr`). That is
  `unknown1`, not a pixel bias.
- **Narrowed:** no ±1 on this operand. Do not ship `originY − 1`.
  The `0.0003` subtract stays on the earlier `0x43CDA0` argument
  (`0x43CCB5`), not on this divisor. A4f already discriminates that
  bias in placement; do not drop it to chase actor 2.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x43CCF8`–`0x43CD56` and `0x47A7BB`–`0x47A7C1`.
- `ActorSprite.cpp` / `.h` / `test_actorsprite.cpp` have no diff.
  No game.

## Next

The divided triple is whatever `0x40EAE0` wrote from **`[this+0x90]`**.
That writer is unread. Evidence: this note,
[A6d15](phase-a6-render-c8-dt.md),
[A4f](phase-a4-actor-billboard.md).
