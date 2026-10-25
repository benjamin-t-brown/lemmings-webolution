# C3o — repaired type-2 geometry and UV pairing

2026-10-25. READ + DERIVED, offline only; no production or pixel acceptance.
Packet: `build/tmp/c3o-type2-draw/repair1-note.md`, numerical `draw-table.json`.
Original rejected packet is preserved byte-for-byte under `before-repair/`.

## Accepted rule and scope

The 17 retained zero-338 nodes name `^lolli*` / `^sheep*` textures. Each file
visibility byte is 1. With the supplied reference camera and conditional
640x480/origin-zero clip, five emit four vertices each; twelve are screen-empty.
These are not backface/near-plane rejects or observed GPU draws. Parent and
scene gates, runtime texture flags, API success and device state remain conditions.

At `0x47D0F2..D12E`, compose the node/parent/camera matrix with `0x58D498`.
Its translation is (x,y,z). From extents (a,b,h) at +320/+328/+330, the generated
points are `(x-a/2,y-h/2,z-a/2)`, `(x-b/2,y+h/2,z-b/2)`,
`(x+b/2,y+h/2,z-b/2)`, `(x+a/2,y-h/2,z-a/2)` (`0x47D152..D371`).
Transform these through `0x58D438` (`0x47D492..D4F3`) to P0..P3.
Generated point indices are **not** native screen-corner labels.

| Native slot/base | X | Y | Z |
| --- | --- | --- | --- |
| 0/+120 | cx+P0.x/P0.z | cy-P1.y/P1.z | P0.z |
| 1/+188 | cx+P1.x/P1.z | cy-P0.y/P0.z | P1.z |
| 2/+1F0 | cx+P2.x/P1.z | cy-P3.y/P0.z | P2.z |
| 3/+258 | cx+P3.x/P0.z | cy-P2.y/P1.z | P3.z |

Native X stores are `0x47D527/D5B3/D5D3/D544`; Y stores in slot order are
`0x47D5F3/D564/D581/D613`. Y destinations are +140/+1A8/+210/+278;
these crossed destinations caused rejection of the original evaluator and the
first facilitator checker. Both are historical, **not acceptance evidence**.
Z dword pairs at `0x47D669..D6C3` preserve each slot's own P.z.

UV slots remain `(f1,f4),(f1,f3),(f2,f3),(f2,f4)` from the four file doubles
(`0x47B57D..B5A9`, setter `0x440C50`). Adapter `0x414920` consumes slots
**0,3,2,1**. In this fixture all a=b, so the repair leaves rectangle bounds and
the 5/12 count unchanged while reversing the old UV/screen pairing on every row.
For the first node the clipped top V is f32 **0.10025838017463684**, not the
rejected **0.8997416496276855**. The source values, not image fitting, select it.

`0x46D460` packs binary32 screen X/Y/Z, RHW `0.25 / stored-f32-Z`, diffuse
and UV in 32-byte records; +14 specular is unwritten, **not zero**. SDK layout
maps primitive6/FVF1C4/device+70 to a TL triangle fan, conditional on the
documented interface route succeeding. This is not an effective-state claim.

## Facilitator check

`python -B build/tmp/c3o-facilitator-repair-check.py` uses separate raw LVL
reads and sequential transforms, not the worker evaluator to derive expectations.
It authenticates all **58** retained PE windows, constants, install files and SDK
header hashes, plus every repair store-audit instruction. Fresh disassembly is
in `build/tmp/c3o-facilitator-repair-check/`; native corner construction, reciprocal
spills, crossed stores, adapter offsets and UV setter were re-read.

All 17 slot tuples and 20 final known-field vertices match. Expectations use a
rectangle interpolation specific to these equal-width/constant-depth fixtures;
general polygon clipping and original FPU mode are **not independently certified**.
The independently derived packed stream rejects every one of the five old
streams. Calling the candidate with **only own_y=True** also fails on all five,
without editing source or install files. An unequal-depth point set separately
protects the shared reciprocals and all four Y-source choices.

Worker's `slot_store_regression.py` and Decimal `independent_check.py` rerun pass;
they supplement, not replace, the facilitator's check. Full build/CTest **34/34**.
No Lemmings process, zero starts/attaches; install hashes match the packet.
No game writes or production edits occurred. Original rejected evidence and its
retractions remain preserved. Constructor defaults/resource names are retained
conditional source facts; uploaded texel words, runtime flags, buffer-history
specular, wrapper state and animated nonzero-338 branch are not accepted here.

## Timing / next

Original dispatch 13:37:26; repair dispatch 13:56:13; repair complete 14:03:55;
accepted 14:10:27 MDT. Original-to-acceptance **33m01s**; repair **14m14s**.
The C3 full-frame baseline stays **0.2050523**, not <=0.02. Next verify C3n,
then derive scene device/material state and implement/render against repair4.
