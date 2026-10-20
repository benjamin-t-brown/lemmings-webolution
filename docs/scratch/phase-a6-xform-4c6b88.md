# Phase A6d22: `0x4C6B88` is a per-draw stack-top copy (READ)

Brief `build/tmp/a6d22-xform-4c6b88-brief.md`. Packet
`build/tmp/a6d22-xform-4c6b88/`. Cursor `round-agent` on
`composer-2.5-fast`, dispatch `7f7dd3cf-8a2f-404a-a413-e13d29015287`.
Offline. No `ActorSprite` change.

## Result

- **READ:** `0x46BE80` calls `0x43BCD0` on `0x58D5A0`, then
  `0x46BE91` `mov ecx, 0x18` and `0x46BE99` `rep movsd` copies
  **96 bytes** from that pointer into `[ebp+8]`.
- **READ:** `0x43BE50` returns `index * 0x60 + [this+0xC]`.
  `0x43BCD0` asks for `count-1`, so the source is the stack top.
- **READ:** `0x441845`’s `this` is the object at `0x4C6A08`.
  `0x441881` calls `0x43A4EC` with `ecx = this+8` (`0x4C6A10`).
  `0x43A627` calls `0x46C440` with `ecx = that+0x30`.
  `0x46C490` passes `this+0x148`, which is **`0x4C6B88`**.
- **READ:** `0x43A65F` later pushes that same `+0x178` into
  `0x40EAE0`. The lemming path pushes the immediate `0x4C6B88`
  at `0x43CC71`.
- **Narrowed:** this is a runtime view-stack snapshot, not a
  file-static identity and not the `+0x90` triple. It is **not**
  shown to be already inside `worldToLensedView`. No ±1. No hook.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x46BE80`–`0x46BE9E`, `0x46C48A`–`0x46C496`,
  `0x43A615`–`0x43A686`, `0x43BCD0`–`0x43BCE6`, `0x43BE50`–`0x43BE68`,
  `0x44184B`–`0x441884`.
- `0x4C6A08+8+0x30+0x148 = 0x4C6B88`.
- `ActorSprite` untouched. No game.

## Parked

Screen-Y bias after A6d20–A6d22. Resume only with a live compare of
the `0x58D5A0` top (or `0x4C6B88` after this copy) to
`worldToLensedView` on the matched camera. Do not ship `originY − 1`.
Evidence: this note, [A6d20](phase-a6-screen-y-bias.md),
[A6d21](phase-a6-plus90-vector.md).
