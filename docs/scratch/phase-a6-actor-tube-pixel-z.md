# Phase A6d16: no per-pixel actor-vs-tube Z compare on submit (READ)

Brief `build/tmp/a6d16-actor-tube-pixel-z-brief.md`. Packet
`build/tmp/a6d16-actor-tube-pixel-z/`. Cursor `round-agent` on
`composer-2.5-fast`, dispatch `dce7b77f-c772-484e-9c70-24461a10ab4a`.
Offline. No composite hook.

## Result

- **READ (negative):** the actor submit chain
  `0x43CC68` → `0x47A75D` → `0x47AB96` → `0x414920` has **no**
  instruction that compares an actor fragment Z to a tube/scene depth
  and rejects the pixel.
- **READ:** `0x43CC94` `fcomp [this+0xC8], 0.0` (`0x4B0338`) is a
  whole-draw skip, not a per-pixel test. `0x43CDA0` only **stores**
  `[this+0xC8]−0.0003` into controller slots.
- **READ:** fragment reject on this path is D3DRS_ALPHAREF from
  `[0x4C24E0]` at `0x472896` (`push 0x18`), written `0xEF` at
  `0x470F39` when `0x470F29(0)` runs. That is A4y texel alpha, not
  tube Z.
- **READ:** `0x414920` does call `0x46D3C1` at `0x4149FD`
  (unconditional). `0x46D460` packs vertex Z and does
  `0.25 / packedZ` at `0x46D51F` (`0x4B19D8`). That is stream
  setup, not a load of the tube software depth buffer.

Prediction (no scene-depth compare on submit) is **confirmed**.
Replace-if-`a>0xEF` stays the overlay model.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x47A75D`–`0x47A7B0`, `0x47AB7F`/`0x47AB8B`, `0x47ABAF`,
  `0x414920`–`0x414A02`, `0x470F39`, `0x472880`–`0x4728AD`,
  `0x43CDA0`–`0x43CDE6`, `0x46D510`–`0x46D528`.
- Independent `.text` `E8` list on `[0x47A75D, 0x47AB96)` matches
  A4x/A4af: `0x470F29`, `0x468D00`, `0x468CE0`, `0x47ABC0`,
  `0x440C50`×4, `0x47AB96`. No lock-rect / tube-raster callee.
- Naive `fcomp` scan hits in `0x47A75D` are `fild`/`fadd` at
  `0x47AAD6`; the `0x46D521` hit is `fdivr 0.25`.
- Narrowed: worker “UV duration” wording for `[+0xC8]` is the A6d15
  homogeneous third component. Wrapper depth (A1) is still not an
  executable read.
- No production change. No game. No CTest.

## Next

Park per-pixel actor Z versus tube. Resume only if wrapper depth is
shown to move the A6 gate. Evidence: this note,
[A6c](phase-a6-actor-tube-composition.md),
[A1 depth](phase-a1-depth-clipping.md).
