# Phase A6d21: render `+0x90` is a 24-byte `0x43D56C` copy (READ)

Brief `build/tmp/a6d21-plus90-vector-brief.md`. Packet
`build/tmp/a6d21-plus90-vector/`. Cursor `round-agent` on
`composer-2.5-fast`, dispatch `effa628f-8d83-43a6-ad5c-b4a2811acdec`.
Offline. No `ActorSprite` change.

## Result

- **READ:** vtable `0x4B14A8` slot +8 is **`0x4658D6`**. That calls
  `0x43D56C` (`ecx = 0x4C6A10`) and then `0x465900`.
- **READ:** `0x46590F` sets `edi = this+0x90`. `0x465915` `mov ecx, 6`.
  `0x46591A` `rep movsd` copies **24 bytes** from the pointer returned
  by `0x43D56C`. `0x43DA2D` returns the caller’s output pointer after
  filling it (`0x43DA2B`).
- **READ:** `0x42D3E9` is `fstp qword [eax+0x90]` with `eax` the
  **lemming root** (`0x4B1078` slot +0x14 = `0x42D246`). That is
  `lemming+0x90`, not `lemming+0x78+0x90`.
- **Narrowed:** `0x40EAE0` still multiplies this triple by
  **`0x4C6B88`** before the A6d20 divide. Do not treat
  `worldToLensedView` as that second multiply. No ±1. No
  `ActorSprite` hook.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x4658D6`–`0x4658FD`, `0x465900`–`0x465921`,
  `0x43DA14`–`0x43DA35`, `0x42D3E3`–`0x42D3EF`.
- Unpacked `0x4B14B0` = `d6 58 46 00` and `0x4B108C` = `46 d2 42 00`.
- Direct `E8` callers of `0x4658D6`: `0x404BE0`, `0x451683`, `0x49448B`.
- `ActorSprite` untouched. No game.

## Next

Who writes the matrix at `0x4C6B88`, and is that multiply identity?
`0x4C6B88` is past `.data` rawsize. Evidence: this note,
[A6d20](phase-a6-screen-y-bias.md),
[A6a](phase-a6-actor-pose.md).
