# Gadget surface interpolation and actor contact

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The runner verifies the complete binary and every copied original routine.

| Routine | VA / file offset | Bytes | SHA-256 |
|---|---|---|---|
| Surface Y | `0x42C714` / `0x2C714` | 123 | `1d027537cca62d719729ce5be13513877fbd04f8d7c4a286a80530619867d7ca` |
| Actor contact | `0x42C78F` / `0x2C78F` | 292 | `acb54efd704224bb04bb9465531e2e1ee5bf17723cbfaa424a1e223426f13717` |
| Model point accessor | `0x412C90` / `0x12C90` | 27 | `d5a8fdd6b139351863017b8caf4146b1a9126fc5310d4d0a4c4a2a82e7dcf08a` |

REA surface evidence:
`ev_e22bb6f0aa180c31f9e6162f6064e8d30d74b0ece5e56d031b01efba414cedda`.
REA contact evidence:
`ev_7c3499e40996fadcd697ea267f68993e1d7219bb6beea12997d4f8ca78dcf1ea`.
Local disassembly confirms the accessor stride, data pointer and intermediate
binary64 store. The accessor is inlined through a typed span in recovered code.

## Recovered behavior

The receiver's model pointer at `+158` supplies a point array at model `+118`.
Its data pointer is at array `+0C`, with 24-byte XYZ points. Surface Y reads point
4 and point 5, stores their Y difference to binary64, reads point 5 again, then
returns this interpolation in x87 extended precision:

`((x - minimum_x) / (maximum_x - minimum_x)) * stored_rise + point5_y`.

No wrapping or clamping is performed by the interpolation helper. A zero extent
retains normal floating-point division behavior. The public view requires at
least six points and valid bounds, matching the original unchecked accesses.

Contact samples actor position plus `pair_360`, subtracts direction `+44` from
sample Y, and checks the bounds of actor gadget `+54`. If it misses, direction
is added back to the stored double sample and containment is tested again.
Two misses return false without changing the actor. A hit uses the *method
receiver's* surface interpolation, then moves actor sample Y through recovered
`42EE74`. The receiver and actor's contact gadget are separate inputs; they can
refer to different objects. This distinction is preserved by the source API.

The movement implementation subtracts the actor's sample offset and clamps Y
to the grid. Containment and movement reuse their previously recovered source
implementations. There are no callbacks in this recovered contact chain.

## Verification

The x64 unit test covers endpoint interpolation, a second-point-only hit,
distinct receiver/contact bounds, sample-offset movement, a miss with no actor
change, and a zero-width slope yielding NaN or infinity.

The native oracle executes the original contact, interpolation, array accessor,
containment, sample, movement and clamp routines unchanged. It patches no code
and adapts no effects. Synthetic original-layout model and gadget objects
provide the same typed inputs as the source views. Each case compares all 80
meaningful bits of the extended slope result, the contact result byte, exact
actor Y bits and absence of other writes to actors, gadgets, model headers or
model points.

20,352 cases pass at both O0 and O2: first/second/missing contact, distinct gadget
bounds, both directions and unusual integer directions, fractional sample
offsets, random slopes and coordinates, reversed and zero extents, infinities
and NaNs. This is direct native evidence for the complete listed call chain;
it does not claim execution of the full game or rendered animation.
