# Phase A6d18: float top at `0x47AB30`; no integer first row (READ)

Brief `build/tmp/a6d18-billboard-top-row-brief.md`. Packet
`build/tmp/a6d18-billboard-top-row/`. Cursor `round-agent` on
`composer-2.5-fast`, dispatch `da799898-e120-48e7-8fb3-c8f4c37ce8ff`.
Offline. No `ActorSprite` change.

## Result

- **READ:** `0x47AB27` `fld [ebp+0x10]`, `0x47AB2A` `fsub [ebp-0x10]`,
  `0x47AB30` `fstp [this+0x160]`. `[ebp+0x10]` is already
  `originY − unknown1` (`0x47A7BB` `fild [rec+0x34]` / `fsubr`).
  `[ebp-0x10]` is `fild [rec+0x3C]` at `0x47A7C7` (frame height).
  Copied as a qword to `[this+0x28]` at `0x47AB3C`.
- **READ:** `0x47AAD6` is the unmirrored **right** edge
  (`fild` doubled width, `fadd [ebp+8]`, `fstp [this+0x158]`), not top.
- **READ:** vertex pack `0x46D4F9`–`0x46D516` is `fld qword` / `fstp dword`.
  No `fistp` in `[0x47A75D, 0x47AB96)` or `[0x46D3C1, 0x46D620)`.
  The `DF` byte at `0x46D5AC` is `call 0x46DE90`, not an integer store.
- **Match:** our `ceil(p.top)` in `drawActorSprite` is the A4v cover
  fit, not an exe instruction. Do not add `originY − 1`.

Prediction that a native `fistp`/`floor` of top exists in this chain is
**falsified**.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x47A7BB`–`0x47A7CA`, `0x47AB27`–`0x47AB4B`, `0x47AAD1`–`0x47AADF`,
  `0x46D4F9`–`0x46D528`, `0x46D5AC`.
- Independent opcode scan: `DB` hits in `0x47A75D` are `fild`, not `fistp`.
- `ActorSprite.cpp` / `.h` / `test_actorsprite.cpp` have no diff from this
  round. No game.

## Next

Measure `floor(top)` vs `ceil(top)` as the first row only, with float
corners unchanged. Do not ship it from the read. Evidence: this note,
[A6d17](phase-a6-actor2-bbox-pixels.md),
[A4v](phase-a4-actor-support-residual.md).
