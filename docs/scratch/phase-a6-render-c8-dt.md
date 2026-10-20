# Phase A6d15: `[lemming+0x78+0xC8]` is not the A6b clock (READ)

Brief `build/tmp/a6d15-render-c8-dt-brief.md`. Packet
`build/tmp/a6d15-render-c8-dt/`. Cursor `round-agent` on
`composer-2.5-fast`, dispatch `f61297ac-f1bb-4ac8-9933-13dae2c21137`.
Offline. No `ActorAnim` hook.

## Result

- **READ:** `0x43CC68` (and `0x43C9D2`) calls `0x40EAE0` with
  dest=`this+0xB8`, vec=`this+0x90`, xform=`0x4C6B88`, then loads
  `[this+0xC8]`. Compare to `0.0` (`0x4B0338`); skip if below.
  `1.0 / [this+0xC8]` (`0x4B02D0`) scales `[this+0xB8]` and
  `[this+0xC0]` into the `0x47A75D` UV operands. `0.0003`
  (`0x4B1498`) is subtracted only for the `0x43CDA0` argument, not
  stored back.
- **READ:** `0x40EB50` writes `dest[0]`, `dest[8]`, `dest[0x10]` as a
  3×3 × vec3. `0x40EAE0` then adds `[xform+0x48]`. Stores into
  `[this+0xC8]` are `0x40EBD5` and `0x40EB3C` (`fstp [edx+0x10]` with
  `edx = this+0xB8`).
- **READ:** A6b elapsed seconds still go
  `vtable+0x2C` → `0x43CF30` → `0x468833`. `0x43CE00` does not write
  `+0xC8`. `0x401B80` on `this+0x90` is a no-op stub.
- **Falsified:** per-frame remaining-budget / total-clip-duration
  readings of `+0xC8`. The field is the post-transform third component
  of the `+0x90` triple (homogeneous / UV divisor), not the SPR clock.

Worker headline “no `.text` store” is **narrowed**: there is no
`fstp qword [reg+disp 0xC8]`, but the dest+`0x10` path writes the slot
every `0x43CC68` call. `0x4C6B88` sits in `.data` past rawsize
(`rawsize 0x7000`); do not cite file bytes there.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x43CC68`–`0x43CD4E`, `0x40EAE0`–`0x40EB41`,
  `0x40EB50`–`0x40EBD9`, `0x40EBE0`, `0x40EBF0`, `0x43CE00`,
  `0x43CF30`, `0x43C9D2`–`0x43CA28`, `0x4B0338` = 0.0,
  `0x4B1498` = 0.0003, `0x4B02D0` = 1.0.
- Independent `.text` scan: 23 disp-`0xC8` hits; the `0x43CCxx` /
  `0x43CAxx` ones are `fld`/`fdiv` only. Dword `mov [reg+0xC8]` sites
  exist (`0x460A2D`, `0x460E1D`, `0x461365`); `0x461365` stores a
  pointer on another object, not this double.
- No production change. No game. No CTest.

## Next

Park render-clock dt after three rounds. Keep the integer-ms stand-in.
Incoming seconds to `0x42E2BF` stay unread; do not spend a fourth
round on that family unless a temporal change is shown to move the A6
gate (A6d4 already swept 0–11). Evidence: this note,
[A6d13](phase-a6-render-clock-dt.md),
[A6d14](phase-a6-walk-budget-ptr.md),
[A6b](phase-a6-actor-anim-clock.md).
