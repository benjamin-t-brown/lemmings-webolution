# Phase A6d13: master render phase writer READ; walk budget pointer open

Brief `build/tmp/a6d13-render-clock-dt-brief.md`. Packet
`build/tmp/a6d13-render-clock-dt/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `ec736f33-0da3-4e41-9901-7243e89c794e`. Offline. No `ActorAnim` hook.

## Result

- **READ:** `0x45CC3D` `fstp [ecx+0x26028]` stores
  `([timer+0x40] + 128.0) / 256.0`. Timer load is `0x42DCD0`
  (`fld [ecx+0x40]`) after `ecx = [game+0x55a8]`. Constants
  `0x4B1350` = 128.0, `0x4B07A8` = 256.0.
- **READ:** `0x41BDD4` still loops on `fld [ [ebp+0xC] ]` at `0x41BE00`.
  `.rdata 0x4B0D2C` is `d4 bd 41 00` (`0x41BDD4`).
- **Unsettled:** `0x42F0DC` `call [edx+4]` (when `[lemming+0x38c]==0`)
  has **no** stack pushes. The `.text` store that fills the budget
  `double*` at `[ebp+0xC]` is not cited. No production `ActorAnim` hook.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x45CC20`–`0x45CC3D`, `0x42DCDA`, `0x41BDDD`–`0x41BE02`,
  `0x42F088`–`0x42F0DC`, `0x4B1350`, `0x4B07A8`, `0x4B0D2C`.
- **Narrowed:** `(timer+128)/256` is a wrap phase, not elapsed seconds.
  Do not treat `[game+0x26028]` as the walk remaining-budget double.
- **Narrowed:** the `[+0x38c]!=0` arm calls `0x43C9D2` with four pushes;
  the zero arm is the bare `call [vtable+4]`.
- No production change. No game. No CTest.

## Next

Round 2: name the store that supplies `[ebp+0xC]` on the `0x42F0DC` path,
or show that slot `+4` is not `0x41BDD4` there. Evidence: this note,
[A6b](phase-a6-actor-anim-clock.md).
