# Actor update finalization and world sample table

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The terminal block is `0x42E58C..0x42E673` inside `0x42E2BF` (the complete
983-byte enclosing function is pinned). The block and recovered helpers have
original bytes and disassembly in [actor_finalization.asm](actor_finalization.asm).
All complete helper bodies are pinned in `tools/verify_reference.py`.

## Static evidence

| Routine | Role | REA evidence |
| --- | --- | --- |
| `42E2BF` | Scheduler and finalization | `ev_9a69c7a29ed62c50e56318fd2df8b7221c72f3607ec91c86a0beeeaa7f4c6ae3` |
| `403F40` | Select world triple by byte index | `ev_8df7f4ed79a9cacb68de0c400bb947801a47cae3c226bd7250c2878d58d66a89` |
| `403FC0` | Signed row index calculation | `ev_aae08bce87a844f89a13c450b15b0287421641901debb2430c4b57fa3dc515ce` |
| `403820` | Copy selected triple to four model fields | `ev_941b60f88c7a546e48fec50fdbb5ae117c9821c996b5f9f3666adb419b796baa` |
| `41A500` | Current animation resource pointer | `ev_0a75e730cd63c5bb6336d015c8e0a608b4f814cc5a2c277ab3f169b56a6c0b4c` |
| `4658D6` | Map model position | `ev_5ed083b54cc8553d4ec300a7908a444bb726dc29633124821d7f74e51a033912` |
| `465900` | Store mapped triple | `ev_0a5ece7d01f0bbbfaebf9fb50d7d0d84834e87dec01f76ca92e3d55c613b0319` |

The tail independently checks direction `+44 == 1` and `== -1`, querying
behavior `0x2B` each time the corresponding comparison succeeds. If absent,
it stores orientation `+2D8` as zero or one. The two checks remain independent:
a query callback that changes direction can make both execute.

Next it invokes inline model `+78` slot `+8`, then reads the current animation
resource and clip. The exact C string `drown` suppresses terrain sampling;
case differs and embedded-NUL termination follow `strcmp`.
Otherwise Y and then X are converted by x87 signed-64 truncation, retaining
the low 32 bits. The world sample index is
`signed32(unsigned32(y) << 8) / 2 + ((x & 255) / 2)`, with wrapping integer
addition. There is no Y clamp. The byte at world `+664`'s array selects one
of 256 raw 24-byte triples at world `+678`. Source preserves every bit,
including signaling NaN payloads; no floating-point conversion occurs.

`403820` copies the triple to playback `+178`, then to `+110`, `+A8`, and
`+40`. Finally the original calls `458186` through actor `+344`, and the
enclosing update returns low byte true. The effect body remains an explicit
callback in this API. The scheduler's early false returns bypass this tail.

The model vtable at `4B14A8` selects `4658D6` for slot `+8`. That function
calls the recovered world map `43D56C`, then `465900` copies the mapped triple
to model `+90`, i.e. actor `+108`. `position_actor_model` supplies this concrete
implementation for callers; the finalization callback still permits other
model implementations and native callback-order testing.

`world_sample_table` is a new view over the serialized byte array and fixed
6144-byte table already read by `439FD3`. Its bounds check throws for invalid
backing storage; the original would access unchecked memory. It does not
assign an unproven color or lighting meaning to the triples.

## Unit and native evidence

Units check X masking, two-X-per-byte indexing, signed row wrap, raw triple
copies, exact `drown` suppression, case sensitivity, callback ordering,
query-time direction mutation, and invalid backing-store rejection.

The native oracle executes the complete unchanged `42E2BF` with zero elapsed
time and an empty extra list, which reaches the unchanged tail. Both scheduler
pass counts are exercised. Original list, clip, string, x87 conversion, sample,
and triple-copy helpers execute unchanged. Type-query and shadow-effect calls
are explicit adapters. Half the cases adapt the model slot to mutate position
and clip; the other half execute unchanged `4658D6`, `465900`, and the complete
original world-coordinate helper chain.

At both O0 and O2, **20,000** tail cases match the ordered effect trace, query
count, Boolean result, and every actor byte. This includes **10,000** original
model-positioning calls, arbitrary initial orientation, direction values,
query-time mutation, model-time mutation, C-string termination, NaN/infinity
integer conversion, and raw random table bits. Another **20,000** world sample
lookups compare the selected raw triple, including wrapping high Y bits.

This closes the previously unexecuted terminal block as a separate recovered
slice. It does not establish a running-game comparison, arbitrary callback
memory aliasing, or the internals of the shadow effect. Existing scheduler
tests independently cover nonzero time, behavior transitions and early exits.
